#include <cpe/engine.hpp>

#include <cpe/acquisition/bytes_acquisition.hpp>
#include <cpe/bytes.hpp>
#include <cpe/data_model.hpp>
#include <cpe/deserialization/record_delimitation.hpp>
#include <cpe/deserialization/record_parsing.hpp>
#include <cpe/error/pipeline_error.hpp>

#include <nonstd/expected.hpp> // nonstd::expected, nonstd::make_unexpected

#include <memory>       // std::unique_ptr
#include <optional>     // std::optional
#include <system_error> // std::make_error_code, std::errc
#include <utility>      // std::move

namespace cpe {
	Engine::Engine(std::unique_ptr<BytesAcquisition> acquisition,
	               std::unique_ptr<RecordDelimitation> delimitation,
	               std::unique_ptr<RecordParsing> parsing, ErrorPolicy policy)
	    : acquisition_(std::move(acquisition)), delimitation_(std::move(delimitation)),
	      parsing_(std::move(parsing)), policy_(policy) {}

	nonstd::expected<std::optional<Record>, PipelineError> Engine::next() {
		while (true) {
			// Drain the queue before pulling more bytes: delimit() may
			// yield several records per call, next() yields one at a time.
			if (!pending_records_.empty()) {
				Bytes raw = std::move(pending_records_.front());
				pending_records_.pop_front();

				auto parsed = parsing_->parse(std::move(raw));
				if (parsed.has_value()) {
					return std::move(parsed).value();
				}
				if (policy_ == ErrorPolicy::Skip) {
					continue;
				}

				// FailFast: promote the RecordError to a PipelineError
				// (ADR-013). RecordError is empty today; the message will
				// be refined once it carries structured context.
				return nonstd::make_unexpected(
				    PipelineError{.code = std::make_error_code(std::errc::protocol_error),
				                  .message = "record parsing failed"});
			}

			// Queue empty and acquisition already exhausted: no more
			// records will ever come out. Signal end of stream.
			if (acquisition_exhausted_) {
				return std::nullopt;
			}

			auto chunk = acquisition_->read();
			if (!chunk.has_value()) {
				return nonstd::make_unexpected(std::move(chunk).error());
			}

			// Empty chunk signals EOS from acquisition (ADR-015).
			// The delimiter still needs a final call with is_final=true
			// to flush any pending segment (e.g. a last line without \n).
			bool is_final = chunk.value().empty();
			if (is_final) {
				acquisition_exhausted_ = true;
			}

			auto delimited = delimitation_->delimit(std::move(chunk).value(), is_final);
			if (!delimited.has_value()) {
				return nonstd::make_unexpected(std::move(delimited).error());
			}

			for (auto& record : delimited.value()) {
				pending_records_.push_back(std::move(record));
			}
		}
	}
} // namespace cpe
