#include <cpe/bytes.hpp>
#include <cpe/deserialization/line_delimitation.hpp>

#include <cstddef>
#include <string_view>

#include <gtest/gtest.h>

namespace cpe {
	Bytes string_to_bytes(std::string_view s) {
		Bytes out;
		out.reserve(s.size());
		for (char c : s) {
			out.push_back(static_cast<std::byte>(c));
		}
		return out;
	}

	TEST(LineDelimitation, EmitsCompleteLinesWhenFinal) {
		LineDelimitation delimiter;
		auto result = delimiter.delimit(string_to_bytes("ab\ncde\nef\n"), true);

		ASSERT_TRUE(result.has_value());
		const auto& records = result.value();
		ASSERT_EQ(records.size(), 3U);
		EXPECT_EQ(records[0], string_to_bytes("ab"));
		EXPECT_EQ(records[1], string_to_bytes("cde"));
		EXPECT_EQ(records[2], string_to_bytes("ef"));
	}

	TEST(LineDelimitation, EmitsCompleteLinesWhenFinalButNoDelimiter) {
		LineDelimitation delimiter;
		auto result = delimiter.delimit(string_to_bytes("ab\ncde"), true);

		ASSERT_TRUE(result.has_value());
		const auto& records = result.value();
		ASSERT_EQ(records.size(), 2U);
		EXPECT_EQ(records[0], string_to_bytes("ab"));
		EXPECT_EQ(records[1], string_to_bytes("cde"));
	}

	TEST(LineDelimitation, DoesNotEmitIncompleteTail) {
		LineDelimitation delimiter;

		auto first = delimiter.delimit(string_to_bytes("ab\nc\nde"), false);
		ASSERT_TRUE(first.has_value());
		ASSERT_EQ(first.value().size(), 2U);
		EXPECT_EQ(first.value()[0], string_to_bytes("ab"));
		EXPECT_EQ(first.value()[1], string_to_bytes("c"));
	}

	TEST(LineDelimitation, ReconstructsLineSplitAcrossCalls) {
		LineDelimitation delimiter;

		auto first = delimiter.delimit(string_to_bytes("ab\ncde"), false);
		ASSERT_TRUE(first.has_value());
		ASSERT_EQ(first.value().size(), 1U);
		EXPECT_EQ(first.value()[0], string_to_bytes("ab"));

		auto second = delimiter.delimit(string_to_bytes("fgh\n"), true);
		ASSERT_TRUE(second.has_value());
		ASSERT_EQ(second.value().size(), 1U);
		EXPECT_EQ(second.value()[0], string_to_bytes("cdefgh"));
	}

	TEST(LineDelimitation, KeepsCarriageReturnInRecord) {
		LineDelimitation delimiter;

		auto first = delimiter.delimit(string_to_bytes("ab\r\ncde\r\n"), true);
		ASSERT_TRUE(first.has_value());
		ASSERT_EQ(first.value().size(), 2U);
		EXPECT_EQ(first.value()[0], string_to_bytes("ab\r"));
		EXPECT_EQ(first.value()[1], string_to_bytes("cde\r"));
	}

	TEST(LineDelimitation, EmptyChunkWithoutFinalEmitsNothing) {
		LineDelimitation delimiter;
		auto result = delimiter.delimit(string_to_bytes(""), false);
		ASSERT_TRUE(result.has_value());
		EXPECT_TRUE(result.value().empty());
	}

	TEST(LineDelimitation, EmptyChunkWithFinalEmitsNothing) {
		LineDelimitation delimiter;
		auto result = delimiter.delimit(string_to_bytes(""), true);
		ASSERT_TRUE(result.has_value());
		EXPECT_TRUE(result.value().empty());
	}

	TEST(LineDelimitation, ConsecutiveEmptyLineEmitEmptyRecords) {
		LineDelimitation delimiter;

		auto result = delimiter.delimit(string_to_bytes("a\n\n\nb\n"), true);
		ASSERT_TRUE(result.has_value());

		const auto& records = result.value();
		ASSERT_EQ(records.size(), 4U);
		EXPECT_EQ(records[0], string_to_bytes("a"));
		EXPECT_TRUE(records[1].empty());
		EXPECT_TRUE(records[2].empty());
		EXPECT_EQ(records[3], string_to_bytes("b"));
	}
} // namespace cpe
