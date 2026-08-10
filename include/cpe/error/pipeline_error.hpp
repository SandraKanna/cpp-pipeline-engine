#pragma once
#include <string>
#include <system_error>

namespace cpe {
	struct PipelineError {
		std::error_code code;
		std::string message;
	};
} // namespace cpe
