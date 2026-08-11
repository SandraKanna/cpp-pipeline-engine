#pragma once

#include <cpe/execution/bytes_acquisition.hpp>
#include <vector>   // std::vector
#include <cstddef>	// std::byte
#include <nonstd/expected.hpp>	// nonstd::expected

namespace cpe {
	class BufferAcquisition: public BytesAcquisition {
	public:
		explicit BufferAcquisition(std::vector<std::byte> data);
		[[nodiscard]] nonstd::expected<std::vector<std::byte>, PipelineError> read() override;
	private:
		std::vector<std::byte> data_;
	};

} // namespace cpe
