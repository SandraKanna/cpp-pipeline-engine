#pragma once

#include <cpe/error/pipeline_error.hpp>
#include <vector>	// std::vector
#include <cstddef>	// std::byte
#include <nonstd/expected.hpp>	// nonstd::expected

namespace cpe {
	class BytesAcquisition {
	public:
		//nonstd::expected becomes std::expected in C++23; only the include and namespace change
		[[nodiscard]] virtual nonstd::expected<std::vector<std::byte>, PipelineError> read() = 0;
		virtual ~BytesAcquisition() = default;
	};

} // namespace cpe
