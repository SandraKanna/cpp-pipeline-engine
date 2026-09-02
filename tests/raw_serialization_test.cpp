#include <cpe/bytes.hpp>
#include <cpe/data_model.hpp>
#include <cpe/error/record_error.hpp>
#include <cpe/serialization/raw_serialization.hpp>

#include <gtest/gtest.h>

#include <cstddef> // std::byte, std::size_t
#include <string>  // std::string

namespace {
	// Reads the string's characters as raw bytes, for comparing against serialized output.
	// Duplicated across test files; extraction to bytes.hpp deferred to refactor session.
	cpe::Bytes to_bytes(const std::string& s) {
		cpe::Bytes bytes(s.size());
		for (std::size_t i = 0; i < s.size(); ++i) {
			bytes[i] = static_cast<std::byte>(s[i]);
		}
		return bytes;
	}
} // namespace

// A Record shaped exactly {"raw": <string>} is the only input the strict contract
// accepts. The output is the byte representation of that string.
TEST(RawSerializationTest, EmitsBytesOfRawStringField) {
	cpe::Record record;
	record.set("raw", cpe::Value{std::string{"hello"}});

	cpe::RawSerialization serializer;
	auto result = serializer.serialize(record);

	ASSERT_TRUE(result.has_value());
	EXPECT_EQ(result.value(), to_bytes("hello"));
}

// Zero fields does not match the {"raw": <string>} shape: the contract rejects it.
TEST(RawSerializationTest, EmptyRecordIsRecordError) {
	cpe::Record record;

	cpe::RawSerialization serializer;
	auto result = serializer.serialize(record);

	EXPECT_FALSE(result.has_value());
}

// More than one field does not match the shape even if one of them is "raw":
// the contract requires exactly one field.
TEST(RawSerializationTest, MoreThanOneFieldIsRecordError) {
	cpe::Record record;
	record.set("raw", cpe::Value{std::string{"hello"}});
	record.set("other", cpe::Value{std::string{"x"}});

	cpe::RawSerialization serializer;
	auto result = serializer.serialize(record);

	EXPECT_FALSE(result.has_value());
}

// A single field with the wrong name does not match the shape: the contract
// requires the field to be named "raw".
TEST(RawSerializationTest, SingleFieldWithWrongNameIsRecordError) {
	cpe::Record record;
	record.set("other", cpe::Value{std::string{"hello"}});

	cpe::RawSerialization serializer;
	auto result = serializer.serialize(record);

	EXPECT_FALSE(result.has_value());
}

// A "raw" field whose value is not a string does not match the shape.
// One non-string alternative (double) stands in for the whole family
// (bool, number, monostate, Object, array): the property is "not string fails".
TEST(RawSerializationTest, RawFieldWithNonStringValueIsRecordError) {
	cpe::Record record;
	record.set("raw", cpe::Value{42.0});

	cpe::RawSerialization serializer;
	auto result = serializer.serialize(record);

	EXPECT_FALSE(result.has_value());
}
