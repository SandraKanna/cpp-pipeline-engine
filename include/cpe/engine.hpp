#pragma once

#include <cpe/acquisition/bytes_acquisition.hpp>
#include <cpe/bytes.hpp>
#include <cpe/data_model.hpp>
#include <cpe/deserialization/record_delimitation.hpp>
#include <cpe/deserialization/record_parsing.hpp>
#include <cpe/error/error_policy.hpp>
#include <cpe/error/pipeline_error.hpp>
#include <cpe/error/record_error.hpp>
#include <cpe/processing/processing.hpp>

#include <nonstd/expected.hpp> // nonstd::expected

#include <deque>    // std::deque
#include <memory>   // std::unique_ptr
#include <optional> // std::optional
#include <variant>  // std::variant
#include <vector>   // std::vector

namespace cpe {
	/// A single step of the processing chain: either a validation, a filter,
	/// or a transformation. The engine holds an ordered sequence of these
	/// and applies them in order to each record (ADR-014, ADR-016).
	using Processing = std::variant<RecordValidation, RecordFiltering, RecordTransformation>;

	/// Orchestrates the pipeline chain (acquisition, delimitation, parsing,
	/// processing) under the pull-based, one-record-at-a-time, blocking
	/// execution model (ADR-005, ADR-006, ADR-007). The engine mediates
	/// between components: each collaborator receives its input as a parameter,
	/// not by reference to the upstream component.
	class Engine {
	public:
		Engine(std::unique_ptr<BytesAcquisition> acquisition,
		       std::unique_ptr<RecordDelimitation> delimitation,
		       std::unique_ptr<RecordParsing> parsing, std::vector<Processing> processing = {},
		       ErrorPolicy policy = ErrorPolicy::FailFast);

		/// Returns the next record produced by the chain.
		/// A Record wrapped in optional: the next record is available.
		/// nullopt: end of stream reached, not an error.
		/// PipelineError: acquisition or delimitation failed, or a record error
		/// from parsing or validation was promoted under FailFast (ADR-012, ADR-013).
		nonstd::expected<std::optional<Record>, PipelineError> next();

	private:
		std::unique_ptr<BytesAcquisition> acquisition_;
		std::unique_ptr<RecordDelimitation> delimitation_;
		std::unique_ptr<RecordParsing> parsing_;
		std::vector<Processing> processing_steps_;

		/// Delimited records produced by delimit() but not yet parsed.
		/// A single delimit() call may return several records; next()
		/// returns them one at a time, in order.
		std::deque<Bytes> pending_records_;

		/// True once acquisition has returned an empty chunk (EOS signal
		/// per ADR-015). Prevents further read() calls and drives the
		/// is_final=true call to delimit().
		bool acquisition_exhausted_ = false;
		ErrorPolicy policy_;

		/// Produces the next delimited record as raw bytes, pulling from
		/// acquisition and delimitation as needed. Returns nullopt on end
		/// of stream.
		nonstd::expected<std::optional<Bytes>, PipelineError> pull_delimited();

		/// Parses raw bytes into a Record, applying the error policy on
		/// failure. Returns nullopt if the record was dropped under Skip.
		nonstd::expected<std::optional<Record>, PipelineError> parse(Bytes raw);

		/// Applies the processing chain to a record, applying the error
		/// policy on validation failure. Returns nullopt if the record was
		/// dropped (filter drop, or validation under Skip).
		nonstd::expected<std::optional<Record>, PipelineError> apply_processing(Record record);
	};
} // namespace cpe
