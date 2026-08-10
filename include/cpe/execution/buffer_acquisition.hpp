#pragma once

#include <cpe/execution/bytes_acquisition.hpp>

namespace cpe {
	class BufferAcquisition: public BytesAcquisition {
	public:
		explicit BufferAcquisition(std::vector<std::byte> data);
		[[nodiscard]] nonstd::expected<std::vector<std::byte>, PipelineError> read() override;
	private:
		std::vector<std::byte> data_;
	};

} // namespace cpe
