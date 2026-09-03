#include <cpe/bytes.hpp>
#include <cpe/delivery/buffer_delivery.hpp>
#include <cpe/error/pipeline_error.hpp>

// #include <vector> // std::vector::insert
#include <nonstd/expected.hpp>	// nonstd::expected

namespace cpe {
	nonstd::expected<void, PipelineError> BufferDelivery::deliver(Bytes out) {
		out_.insert(out_.end(), out.begin(), out.end());
		return {};
	}

	const Bytes& BufferDelivery::delivered() const noexcept {
		return out_;
	}
} // namespace cpe
