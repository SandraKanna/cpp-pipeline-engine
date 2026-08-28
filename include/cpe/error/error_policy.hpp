#pragma once

namespace cpe{
    /// Runtime behavior of the pipeline when a record fails (ADR-013).
	/// FailFast promotes the record error to a pipeline error and stops.
	/// Skip discards the failing record and continues with the next one.
	enum class ErrorPolicy : std::uint8_t {
		FailFast,
		Skip,
	};
}
