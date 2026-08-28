#pragma once

#include <cpe/data_model.hpp>

#include <functional>	// std::function

namespace cpe {
	/// A rule that checks a record against a domain condition.
	/// Returns true if the record passes, false if it fails.
	/// A failing record is a record error (see ADR-013 for handling).
	struct RecordValidation {
		std::function<bool(const Record&)> fn;
   		[[nodiscard]] bool operator()(const Record& r) const;
	};

	/// A rule that decides whether a record continues down the pipeline.
	/// Returns true to keep, false to drop silently. Dropping is not an error.
	///
	/// Distinct from RecordValidation despite sharing the underlying signature:
	/// the two carry different meaning and the type system keeps them apart.
	struct RecordFiltering {
		std::function<bool(const Record&)> fn;
   		[[nodiscard]] bool operator()(const Record& r) const;
	};

	/// A rule that produces a record from a record.
	/// Receives ownership of the input and returns the transformed record.
	struct RecordTransformation {
		std::function<Record(Record)> fn;
		[[nodiscard]] Record operator()(Record r) const;
	};
} // namespace cpe
