#include <cpe/acquisition/buffer_acquisition.hpp>
#include <cstddef>
#include <gtest/gtest.h>
#include <vector>

TEST(BufferAcquisition, NotEmptyBufferLastCallIsEOF) {
	std::vector<std::byte> data = {std::byte{'h'}, std::byte{'i'}};
	cpe::BufferAcquisition buffer(std::move(data));
	auto result = buffer.read();
	ASSERT_TRUE(result.has_value());
	EXPECT_FALSE(result->empty());
	auto result2 = buffer.read(); // EOF
	ASSERT_TRUE(result2.has_value());
	EXPECT_TRUE(result2->empty());
}

TEST(BufferAcquisition, EmptyBufferReturnsEOF) {
	std::vector<std::byte> data = {};
	cpe::BufferAcquisition buffer(data);
	auto result = buffer.read(); // EOF
	ASSERT_TRUE(result.has_value());
	EXPECT_TRUE(result->empty());
}

TEST(BufferAcquisition, MultipleCallsAfterEOFReturnsEOF) {
	std::vector<std::byte> data = {std::byte{'h'}, std::byte{'i'}};
	cpe::BufferAcquisition buffer(std::move(data));
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
