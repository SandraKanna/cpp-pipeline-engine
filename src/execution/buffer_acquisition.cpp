#include <cpe/execution/buffer_acquisition.hpp>

#include <utility>	// std::move
#include <vector>	// std::vector
#include <cstddef>	// std::byte

namespace cpe {
	BufferAcquisition::BufferAcquisition(std::vector<std::byte> data) : data_(std::move(data)) {}

	nonstd::expected<std::vector<std::byte>, PipelineError> BufferAcquisition::read() {
		return std::move(data_);
	}

} // namespace cpe
