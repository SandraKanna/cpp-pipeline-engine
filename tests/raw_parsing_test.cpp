#include <cpe/data_model.hpp>
#include <cpe/deserialization/raw_parsing.hpp>

#include <cstddef>
#include <string>

#include <gtest/gtest.h>

namespace {
	// Reads the string's characters as raw bytes, for comparing against file contents.
	cpe::Bytes to_bytes(const std::string& s) {
		cpe::Bytes bytes(s.size());
		for (std::size_t i = 0; i < s.size(); ++i) {
			bytes[i] = static_cast<std::byte>(s[i]);
		}
		return bytes;
	}
} // namespace

// Non-empty input becomes a single-field record with the whole input as its value.
TEST(RawParsing, WrapsInputAsSingleRawField) {
	cpe::RawParsing parser;
	auto result = parser.parse(to_bytes("2026-08-19 ERROR disk full"));
	ASSERT_TRUE(result.has_value());
	const cpe::Value* v = result->get("raw");
	ASSERT_NE(v, nullptr);
	EXPECT_EQ(*v, cpe::Value(std::string("2026-08-19 ERROR disk full")));
}

// Empty input still produces a record with the "raw" field, holding an empty string.
TEST(RawParsing, EmptyInputProducesEmptyRawField) {
	cpe::RawParsing parser;
	auto result = parser.parse(cpe::Bytes{});
	ASSERT_TRUE(result.has_value());
	const cpe::Value* v = result->get("raw");
	ASSERT_NE(v, nullptr);
	EXPECT_EQ(*v, cpe::Value(std::string("")));
}

// Bytes are preserved verbatim, including embedded delimiters like \n and \r.
TEST(RawParsing, PreservesEmbeddedBytesVerbatim) {
	cpe::RawParsing parser;
	auto result = parser.parse(to_bytes("line one\nline two\r"));
	ASSERT_TRUE(result.has_value());
	const cpe::Value* v = result->get("raw");
	ASSERT_NE(v, nullptr);
	EXPECT_EQ(*v, cpe::Value(std::string("line one\nline two\r")));
}
