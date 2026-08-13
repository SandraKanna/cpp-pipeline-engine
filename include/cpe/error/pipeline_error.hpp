#pragma once
#include <string>
#include <system_error>

namespace cpe {
	/// Failure that blocks pipeline execution, returned to the caller (ADR-008,
	/// ADR-009). A component produces one when it cannot continue: the origin
	/// cannot be read, the destination cannot be written, and so on.
	struct PipelineError {
		std::error_code code;	///< The underlying error, comparable against std::errc.
		std::string message;	///< Human-readable context (which file, which operation).
	};
} // namespace cpe
