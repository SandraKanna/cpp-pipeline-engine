#pragma once

#include <cpe/bytes.hpp>
#include <cpe/delivery/bytes_delivery.hpp>
#include <cpe/error/pipeline_error.hpp>

#include <nonstd/expected.hpp>	// nonstd::expected

namespace cpe {
	class BufferDelivery : public BytesDelivery {
	public:
		/// deliver() as specified by BytesDelivery
		[[nodiscard]] nonstd::expected<void, PipelineError> deliver(Bytes out) override;
		[[nodiscard]] const Bytes& delivered() const noexcept;
	private:
		Bytes out_;
	};
} // namespace cpe
