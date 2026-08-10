#pragma once

#include <cpe/error/pipeline_error.hpp>
#include <vector>
#include <cstddef>
#include <nonstd/expected.hpp>

namespace cpe {
	class BytesAcquisition {
	public:
		//nonstd::expected becomes std::expected in C++23; only the include and namespace change
		[[nodiscard]] virtual nonstd::expected<std::vector<std::byte>, PipelineError> read() = 0;
		virtual ~BytesAcquisition() = default;
	};

} // namespace cpe
