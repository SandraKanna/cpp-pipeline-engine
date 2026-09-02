#include <cpe/data_model.hpp>
#include <cpe/deserialization/raw_parsing.hpp>

#include <cstddef>
#include <string>

#include <gtest/gtest.h>

namespace cpe {
	namespace {
		// Reads the string's characters as raw bytes, for comparing against file contents.
		Bytes to_bytes(const std::string& s) {
			Bytes bytes(s.size());
			for (std::size_t i = 0; i < s.size(); ++i) {
				bytes[i] = static_cast<std::byte>(s[i]);
			}
			return bytes;
		}
	} // namespace

	// Non-empty input becomes a single-field record with the whole input as its value.
	TEST(RawParsing, WrapsInputAsSingleRawField) {
		RawParsing parser;
		auto result = parser.parse(to_bytes("2026-08-19 ERROR disk full"));
		ASSERT_TRUE(result.has_value());
		const Value* v = result->get("raw");
		ASSERT_NE(v, nullptr);
		EXPECT_EQ(*v, Value(std::string("2026-08-19 ERROR disk full")));
	}

	// Empty input still produces a record with the "raw" field, holding an empty string.
	TEST(RawParsing, EmptyInputProducesEmptyRawField) {
		RawParsing parser;
		auto result = parser.parse(Bytes{});
		ASSERT_TRUE(result.has_value());
		const Value* v = result->get("raw");
		ASSERT_NE(v, nullptr);
		EXPECT_EQ(*v, Value(std::string("")));
	}

	// Bytes are preserved verbatim, including embedded delimiters like \n and \r.
	TEST(RawParsing, PreservesEmbeddedBytesVerbatim) {
		RawParsing parser;
		auto result = parser.parse(to_bytes("line one\nline two\r"));
		ASSERT_TRUE(result.has_value());
		const Value* v = result->get("raw");
		ASSERT_NE(v, nullptr);
		EXPECT_EQ(*v, Value(std::string("line one\nline two\r")));
	}
} // namespace cpe
