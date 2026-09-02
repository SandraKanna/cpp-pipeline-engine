#include <cpe/acquisition/buffer_acquisition.hpp>
#include <cpe/bytes.hpp>

#include <cstddef>

#include <gtest/gtest.h>

namespace cpe {
	TEST(BufferAcquisition, NotEmptyBufferLastCallIsEOF) {
		Bytes data = {std::byte{'h'}, std::byte{'i'}};
		BufferAcquisition buffer(std::move(data));
		auto result = buffer.read();
		ASSERT_TRUE(result.has_value());
		EXPECT_FALSE(result->empty());
		auto result2 = buffer.read(); // EOF
		ASSERT_TRUE(result2.has_value());
		EXPECT_TRUE(result2->empty());
	}

	TEST(BufferAcquisition, EmptyBufferReturnsEOF) {
		Bytes data = {};
		BufferAcquisition buffer(data);
		auto result = buffer.read(); // EOF
		ASSERT_TRUE(result.has_value());
		EXPECT_TRUE(result->empty());
	}

	TEST(BufferAcquisition, MultipleCallsAfterEOFReturnsEOF) {
		Bytes data = {std::byte{'h'}, std::byte{'i'}};
		BufferAcquisition buffer(std::move(data));
		auto result = buffer.read();
		ASSERT_TRUE(result.has_value());
		EXPECT_FALSE(result->empty());
		auto result2 = buffer.read(); // EOF
		ASSERT_TRUE(result2.has_value());
		EXPECT_TRUE(result2->empty());
		auto result3 = buffer.read(); // EOF
		ASSERT_TRUE(result3.has_value());
		EXPECT_TRUE(result3->empty());
	}
} // namespace cpe
