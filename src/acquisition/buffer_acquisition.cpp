#include <cpe/acquisition/buffer_acquisition.hpp>
#include <cpe/bytes.hpp>

#include <utility>	// std::move


namespace cpe {
	BufferAcquisition::BufferAcquisition(Bytes data) : data_(std::move(data)) {}

	nonstd::expected<Bytes, PipelineError> BufferAcquisition::read() {
		return std::move(data_);
	}

} // namespace cpe
