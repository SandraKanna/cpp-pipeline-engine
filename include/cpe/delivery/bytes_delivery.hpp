#pragma once

#include <cpe/bytes.hpp>
#include <cpe/error/pipeline_error.hpp>

#include <nonstd/expected.hpp>	// nonstd::expected

namespace cpe {
	class BytesDelivery {
	public:
		// Polymorphic base: non-copyable and non-movable to prevent slicing
		// through a reference to the base. Derived components manage their own
		// copy/move semantics according to what they hold.
		BytesDelivery() = default;
		BytesDelivery(const BytesDelivery&) = delete;
		BytesDelivery& operator=(const BytesDelivery&) = delete;
		BytesDelivery(BytesDelivery&&) = delete;
		BytesDelivery& operator=(BytesDelivery&&) = delete;
		virtual ~BytesDelivery() = default;

		[[nodiscard]] virtual nonstd::expected<void, PipelineError> deliver(Bytes out) = 0;
	};
} // namespace cpe
