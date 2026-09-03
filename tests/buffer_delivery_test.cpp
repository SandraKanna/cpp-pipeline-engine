#include <cpe/bytes.hpp>
#include <cpe/delivery/buffer_delivery.hpp>

#include <gtest/gtest.h>

namespace cpe {
	TEST(BufferDelivery, FreshBufferHasNothingDelivered) {
		BufferDelivery delivery;
		EXPECT_TRUE(delivery.delivered().empty());
	}

	TEST(BufferDelivery, DeliverMakesBytesAvailable) {
		BufferDelivery delivery;
		auto result = delivery.deliver(to_bytes("ALICE"));
		EXPECT_TRUE(result.has_value());
		EXPECT_EQ(delivery.delivered(), to_bytes("ALICE"));
	}

	TEST(BufferDelivery, ConsecutiveDeliversAccumulateInOrder) {
		BufferDelivery delivery;
		ASSERT_TRUE(delivery.deliver(to_bytes("ALICE")).has_value());
		ASSERT_TRUE(delivery.deliver(to_bytes("BOB")).has_value());
		EXPECT_EQ(delivery.delivered(), to_bytes("ALICEBOB"));
	}
} // namespace cpe
