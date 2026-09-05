#include <cpe/engine.hpp>

#include <cpe/bytes.hpp>
#include <cpe/data_model.hpp>
#include <cpe/error/error_policy.hpp>
#include <cpe/error/pipeline_error.hpp>

#include <cpe/acquisition/bytes_acquisition.hpp>
#include <cpe/delivery/bytes_delivery.hpp>
#include <cpe/deserialization/record_delimitation.hpp>
#include <cpe/deserialization/record_parsing.hpp>
#include <cpe/processing/processing.hpp>
#include <cpe/serialization/record_serialization.hpp>

#include <nonstd/expected.hpp> // nonstd::expected, nonstd::make_unexpected

#include <memory>       // std::unique_ptr
#include <optional>     // std::optional
#include <system_error> // std::make_error_code, std::errc
#include <type_traits>  // std::decay_t, std::is_same_v
#include <utility>      // std::move
#include <variant>      // std::visit
#include <vector>       // std::vector

namespace cpe {
	Engine::Engine(std::unique_ptr<BytesAcquisition> acquisition,
	               std::unique_ptr<RecordDelimitation> delimitation,
	               std::unique_ptr<RecordParsing> parsing, std::vector<Processing> processing,
	               std::unique_ptr<RecordSerialization> serialization,
	               std::unique_ptr<BytesDelivery> delivery, ErrorPolicy policy)
	    : acquisition_(std::move(acquisition)), delimitation_(std::move(delimitation)),
	      parsing_(std::move(parsing)), processing_steps_(std::move(processing)),
	      serialization_(std::move(serialization)), delivery_(std::move(delivery)),
	      policy_(policy) {}

	nonstd::expected<Status, PipelineError> Engine::next() {
		while (true) {

			auto raw = pull_delimited();
			if (!raw.has_value()) {
				return nonstd::make_unexpected(std::move(raw).error());
			}
			if (!raw.value().has_value()) {
				return Status::EndOfStream;
			}

			auto parsed = parse(std::move(raw).value().value());
			if (!parsed.has_value()) {
				return nonstd::make_unexpected(std::move(parsed).error());
			}
			if (!parsed.value().has_value()) {
				continue; // dropped by policy
			}

			auto processed = apply_processing(std::move(parsed).value().value());
			if (!processed.has_value()) {
				return nonstd::make_unexpected(std::move(processed).error());
			}
			if (!processed.value().has_value()) {
				continue; // dropped by filter or validation under Skip
			}

			auto bytes = serialize(std::move(processed).value().value());
			if (!bytes.has_value()) {
				return nonstd::make_unexpected(std::move(bytes).error());
			}
			if (!bytes.value().has_value()) {
				continue; // dropped by policy
			}

			auto delivered = deliver(std::move(bytes).value().value());
			if (!delivered.has_value()) {
				return nonstd::make_unexpected(std::move(delivered).error());
			}

			return Status::Delivered;
		}
	}

	nonstd::expected<std::optional<Bytes>, PipelineError> Engine::pull_delimited() {
		// Drain the queue before pulling more bytes: delimit() may
		// yield several records per call, next() yields one at a time.
		while (pending_records_.empty()) {
			// Queue empty and acquisition already exhausted: no more
			// records will ever come out. Signal end of stream.
			if (acquisition_exhausted_) {
				return std::optional<Bytes>{};
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

		Bytes raw = std::move(pending_records_.front());
		pending_records_.pop_front();
		return std::optional<Bytes>{std::move(raw)};
	}

	nonstd::expected<std::optional<Record>, PipelineError> Engine::parse(Bytes raw) {
		auto parsed = parsing_->parse(std::move(raw));
		if (parsed.has_value()) {
			return std::optional<Record>{std::move(parsed).value()};
		}
		if (policy_ == ErrorPolicy::Skip) {
			return std::optional<Record>{};
		}
		// FailFast: promote the RecordError to a PipelineError (ADR-013).
		// RecordError is empty today; the message will be refined once
		// it carries structured context.
		return nonstd::make_unexpected(
		    PipelineError{.code = std::make_error_code(std::errc::protocol_error),
		                  .message = "record parsing failed"});
	}

	nonstd::expected<std::optional<Record>, PipelineError> Engine::apply_processing(Record record) {
		for (auto& step : processing_steps_) {
			// Three outcomes from each visited step: continue with the
			// record (possibly transformed), drop the record, or fail.
			bool dropped = false;
			auto outcome = std::visit(
			    [&](auto& s) -> nonstd::expected<void, PipelineError> {
				    using T = std::decay_t<decltype(s)>;
				    if constexpr (std::is_same_v<T, RecordValidation>) {
					    if (!s(record)) {
						    if (policy_ == ErrorPolicy::Skip) {
							    dropped = true;
							    return {};
						    }
						    // FailFast: promote the failed validation to a
						    // PipelineError (ADR-013), same treatment as a
						    // parsing failure.
						    return nonstd::make_unexpected(PipelineError{
						        .code = std::make_error_code(std::errc::protocol_error),
						        .message = "record validation failed"});
					    }
				    } else if constexpr (std::is_same_v<T, RecordFiltering>) {
					    if (!s(record)) {
						    dropped = true;
					    }
				    } else if constexpr (std::is_same_v<T, RecordTransformation>) {
					    record = s(std::move(record));
				    }
				    return {};
			    },
			    step);

			if (!outcome.has_value()) {
				return nonstd::make_unexpected(std::move(outcome).error());
			}
			if (dropped) {
				return std::optional<Record>{};
			}
		}
		return std::optional<Record>{std::move(record)};
	}

	nonstd::expected<std::optional<Bytes>, PipelineError> Engine::serialize(Record record) {
		auto serialized = serialization_->serialize(std::move(record));
		if (serialized.has_value()) {
			return std::optional<Bytes>{std::move(serialized).value()};
		}
		if (policy_ == ErrorPolicy::Skip) {
			return std::optional<Bytes>{};
		}
		// FailFast: promote the RecordError to a PipelineError (ADR-013).
		// RecordError is empty today; the message will be refined once
		// it carries structured context.
		return nonstd::make_unexpected(
		    PipelineError{.code = std::make_error_code(std::errc::protocol_error),
		                  .message = "record serialization failed"});
	}

	nonstd::expected<void, PipelineError> Engine::deliver(Bytes bytes) {
		return delivery_->deliver(std::move(bytes));
	}
} // namespace cpe
