#pragma once

#include <cpe/acquisition/bytes_acquisition.hpp>
#include <cpe/bytes.hpp>
#include <cpe/data_model.hpp>
#include <cpe/deserialization/record_delimitation.hpp>
#include <cpe/deserialization/record_parsing.hpp>
#include <cpe/error/pipeline_error.hpp>
#include <cpe/error/record_error.hpp>

#include <nonstd/expected.hpp> // nonstd::expected

#include <deque>    // std::deque
#include <memory>   // std::unique_ptr
#include <optional> // std::optional

namespace cpe {
	/// Runtime behavior of the pipeline when a record fails (ADR-013).
	/// FailFast promotes the record error to a pipeline error and stops.
	/// Skip discards the failing record and continues with the next one.
	enum class ErrorPolicy : std::uint8_t {
		FailFast,
		Skip,
	};

	/// Orchestrates the byte-side chain (acquisition, delimitation, parsing)
	/// under the pull-based, one-record-at-a-time, blocking execution model
	/// (ADR-005, ADR-006, ADR-007). The engine mediates between components:
	/// each collaborator receives its input as a parameter, not by reference
	/// to the upstream component.
	class Engine {
	public:
		Engine(std::unique_ptr<BytesAcquisition> acquisition,
		       std::unique_ptr<RecordDelimitation> delimitation,
		       std::unique_ptr<RecordParsing> parsing, ErrorPolicy policy);

		/// Returns the next record produced by the chain.
		/// A Record wrapped in optional: the next record is available.
		/// nullopt: end of stream reached, not an error.
		/// PipelineError: acquisition or delimitation failed, or a record
		/// error was promoted under FailFast (ADR-012, ADR-013).
		nonstd::expected<std::optional<Record>, PipelineError> next();

	private:
		std::unique_ptr<BytesAcquisition> acquisition_;
		std::unique_ptr<RecordDelimitation> delimitation_;
		std::unique_ptr<RecordParsing> parsing_;

		/// Delimited records produced by delimit() but not yet parsed.
		/// A single delimit() call may return several records; next()
		/// returns them one at a time, in order.
		std::deque<Bytes> pending_records_;

		/// True once acquisition has returned an empty chunk (EOS signal
		/// per ADR-015). Prevents further read() calls and drives the
		/// is_final=true call to delimit().
		bool acquisition_exhausted_ = false;
		ErrorPolicy policy_;
	};
} // namespace cpe
