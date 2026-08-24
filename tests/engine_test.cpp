#include <cpe/engine.hpp>

#include <cpe/bytes.hpp>
#include <cpe/data_model.hpp>
#include <cpe/error/pipeline_error.hpp>
#include <cpe/error/record_error.hpp>

#include "mocks/mock_bytes_acquisition.hpp"
#include "mocks/mock_record_delimitation.hpp"
#include "mocks/mock_record_parsing.hpp"

#include <nonstd/expected.hpp> // nonstd::expected, nonstd::make_unexpected

#include <memory> // std::unique_ptr
// #include <optional>     // std::optional
#include <system_error> // std::make_error_code, std::errc
#include <utility>      // std::move

#include <gtest/gtest.h>

using ::testing::Return;

// NOLINTNEXTLINE(bugprone-reserved-identifier)
using ::testing::_;

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

namespace cpe::test {
	// Happy path: one chunk from acquisition, one delimited record,
	// one successful parse. next() returns the record.
	TEST(EngineTest, ReturnsSingleRecordFromSingleChunk) {
		// STUDY: create empty mocks that will be owned by the engine
		auto acquisition_owned = std::make_unique<MockBytesAcquisition>();
		auto delimitation_owned = std::make_unique<MockRecordDelimitation>();
		auto parsing_owned = std::make_unique<MockRecordParsing>();

		// STUDY: create another pointer to the mocked object, used for config
		auto* acquisition = acquisition_owned.get();
		auto* delimitation = delimitation_owned.get();
		auto* parsing = parsing_owned.get();

		Bytes bytes = to_bytes("hi");

		Record expected_record;
		expected_record.set("raw", std::string{"hi"});

		// STUDY: program the engine, for it to know what each component's behavior should be
		EXPECT_CALL(*acquisition, read())
		    .WillOnce(Return(nonstd::expected<Bytes, PipelineError>(bytes)));

		EXPECT_CALL(*delimitation, delimit(_, false))
		    .WillOnce(Return(
		        nonstd::expected<std::vector<Bytes>, PipelineError>(std::vector<Bytes>{bytes})));

		EXPECT_CALL(*parsing, parse(_))
		    .WillOnce(Return(nonstd::expected<Record, RecordError>(expected_record)));

		// STUDY: build the engine, giving it ownership of the components
		Engine engine(std::move(acquisition_owned), std::move(delimitation_owned),
		              std::move(parsing_owned), ErrorPolicy::FailFast);

		// STUDY: the engine should call the components and return what was programmed
		auto result = engine.next();

		ASSERT_TRUE(result.has_value());
		ASSERT_TRUE(result.value().has_value());
		// NOLINTNEXTLINE(bugprone-unchecked-optional-access): guarded by ASSERT_TRUE above
		const auto& record = *result.value();
		EXPECT_EQ(record, expected_record);
	}

	// Happy path: multiple records per chunk. Acquisition returns one chunk that delimit()
	// will split. next() returns the one by one, in consecutive order.
	TEST(EngineTest, ReturnsMultipleRecordsFromSingleChunk) {
		auto acquisition_owned = std::make_unique<MockBytesAcquisition>();
		auto delimitation_owned = std::make_unique<MockRecordDelimitation>();
		auto parsing_owned = std::make_unique<MockRecordParsing>();

		auto* acquisition = acquisition_owned.get();
		auto* delimitation = delimitation_owned.get();
		auto* parsing = parsing_owned.get();

		Bytes bytes = to_bytes("record one\nrecord two\nrecord three");

		Record expected_record_1;
		expected_record_1.set("raw", std::string{"record one"});
		Record expected_record_2;
		expected_record_2.set("raw", std::string{"record two"});
		Record expected_record_3;
		expected_record_3.set("raw", std::string{"record three"});

		EXPECT_CALL(*acquisition, read())
		    .WillOnce(Return(nonstd::expected<Bytes, PipelineError>(bytes)));

		EXPECT_CALL(*delimitation, delimit(_, false))
		    .WillOnce(Return(nonstd::expected<std::vector<Bytes>, PipelineError>(std::vector<Bytes>{
		        to_bytes("record one"), to_bytes("record two"), to_bytes("record three")})));

		EXPECT_CALL(*parsing, parse(_))
		    .WillOnce(Return(nonstd::expected<Record, RecordError>(expected_record_1)))
		    .WillOnce(Return(nonstd::expected<Record, RecordError>(expected_record_2)))
		    .WillOnce(Return(nonstd::expected<Record, RecordError>(expected_record_3)));

		Engine engine(std::move(acquisition_owned), std::move(delimitation_owned),
		              std::move(parsing_owned), ErrorPolicy::FailFast);

		auto result_1 = engine.next();
		auto result_2 = engine.next();
		auto result_3 = engine.next();

		ASSERT_TRUE(result_1.has_value());
		ASSERT_TRUE(result_1.value().has_value());
		// NOLINTNEXTLINE(bugprone-unchecked-optional-access): guarded by ASSERT_TRUE above
		const auto& record1 = *result_1.value();
		EXPECT_EQ(record1, expected_record_1);

		ASSERT_TRUE(result_2.has_value());
		ASSERT_TRUE(result_2.value().has_value());
		// NOLINTNEXTLINE(bugprone-unchecked-optional-access): guarded by ASSERT_TRUE above
		const auto& record2 = *result_2.value();
		EXPECT_EQ(record2, expected_record_2);

		ASSERT_TRUE(result_3.has_value());
		ASSERT_TRUE(result_3.value().has_value());
		// NOLINTNEXTLINE(bugprone-unchecked-optional-access): guarded by ASSERT_TRUE above
		const auto& record3 = *result_3.value();
		EXPECT_EQ(record3, expected_record_3);
	}

	// Normal EOS. Acquisition returns empty chunk. The engine calls delimit()
	// one last time with is_final=true. When the queue is empty, next() returns nullopt.
	TEST(EngineTest, ReturnsNulloptAtEndOfStream) {
		auto acquisition_owned = std::make_unique<MockBytesAcquisition>();
		auto delimitation_owned = std::make_unique<MockRecordDelimitation>();
		auto parsing_owned = std::make_unique<MockRecordParsing>();

		auto* acquisition = acquisition_owned.get();
		auto* delimitation = delimitation_owned.get();
		auto* parsing = parsing_owned.get();

		Bytes bytes = to_bytes("record one\n");

		Record expected_record_1;
		expected_record_1.set("raw", std::string{"record one"});

		EXPECT_CALL(*acquisition, read())
		    .WillOnce(Return(nonstd::expected<Bytes, PipelineError>(bytes)))
		    .WillOnce(Return(nonstd::expected<Bytes, PipelineError>(Bytes{})));

		EXPECT_CALL(*delimitation, delimit(_, false))
		    .WillOnce(Return(nonstd::expected<std::vector<Bytes>, PipelineError>(
		        std::vector<Bytes>{to_bytes("record one")})));
		EXPECT_CALL(*delimitation, delimit(_, true))
		    .WillOnce(
		        Return(nonstd::expected<std::vector<Bytes>, PipelineError>(std::vector<Bytes>{})));

		EXPECT_CALL(*parsing, parse(_))
		    .WillOnce(Return(nonstd::expected<Record, RecordError>(expected_record_1)));

		Engine engine(std::move(acquisition_owned), std::move(delimitation_owned),
		              std::move(parsing_owned), ErrorPolicy::FailFast);

		auto result_1 = engine.next();
		auto result_2 = engine.next();

		ASSERT_TRUE(result_1.has_value());
		ASSERT_TRUE(result_1.value().has_value());
		// NOLINTNEXTLINE(bugprone-unchecked-optional-access): guarded by ASSERT_TRUE above
		const auto& record1 = *result_1.value();
		EXPECT_EQ(record1, expected_record_1);

		ASSERT_TRUE(result_2.has_value());
		EXPECT_FALSE(result_2.value().has_value());
	}

	// Acquisition returns PipelineError. next() propagates it as-is.
	TEST(EngineTest, PropagatesPipelineErrorFromAcquisition) {
		auto acquisition_owned = std::make_unique<MockBytesAcquisition>();
		auto delimitation_owned = std::make_unique<MockRecordDelimitation>();
		auto parsing_owned = std::make_unique<MockRecordParsing>();

		auto* acquisition = acquisition_owned.get();

		PipelineError error{.code = std::make_error_code(std::errc::io_error),
		                    .message = "acquisition failed"};

		EXPECT_CALL(*acquisition, read())
		    .WillOnce(
		        Return(nonstd::expected<Bytes, PipelineError>(nonstd::make_unexpected(error))));

		Engine engine(std::move(acquisition_owned), std::move(delimitation_owned),
		              std::move(parsing_owned), ErrorPolicy::FailFast);

		auto result = engine.next();

		ASSERT_FALSE(result.has_value());
		EXPECT_EQ(result.error().code, error.code);
		EXPECT_EQ(result.error().message, error.message);
	}

	// Delimitation returns PipelineError. next() propagates it as-is.
	TEST(EngineTest, PropagatesPipelineErrorFromDelimitation) {
		auto acquisition_owned = std::make_unique<MockBytesAcquisition>();
		auto delimitation_owned = std::make_unique<MockRecordDelimitation>();
		auto parsing_owned = std::make_unique<MockRecordParsing>();

		auto* acquisition = acquisition_owned.get();
		auto* delimitation = delimitation_owned.get();

		Bytes bytes = to_bytes("record one\n");

		PipelineError error{.code = std::make_error_code(std::errc::io_error),
		                    .message = "delimitation failed"};

		EXPECT_CALL(*acquisition, read())
		    .WillOnce(Return(nonstd::expected<Bytes, PipelineError>(bytes)));

		EXPECT_CALL(*delimitation, delimit(_, false))
		    .WillOnce(Return(nonstd::expected<std::vector<Bytes>, PipelineError>(
		        nonstd::make_unexpected(error))));

		Engine engine(std::move(acquisition_owned), std::move(delimitation_owned),
		              std::move(parsing_owned), ErrorPolicy::FailFast);

		auto result = engine.next();

		ASSERT_FALSE(result.has_value());
		EXPECT_EQ(result.error().code, error.code);
		EXPECT_EQ(result.error().message, error.message);
	}

	// Parse fails under FailFast. next() promotes the RecordError to a
	// PipelineError (ADR-013).
	TEST(EngineTest, PromotesRecordErrorUnderFailFast) {
		auto acquisition_owned = std::make_unique<MockBytesAcquisition>();
		auto delimitation_owned = std::make_unique<MockRecordDelimitation>();
		auto parsing_owned = std::make_unique<MockRecordParsing>();

		auto* acquisition = acquisition_owned.get();
		auto* delimitation = delimitation_owned.get();
		auto* parsing = parsing_owned.get();

		Bytes bytes = to_bytes("record one\n");

		EXPECT_CALL(*acquisition, read())
		    .WillOnce(Return(nonstd::expected<Bytes, PipelineError>(bytes)));

		EXPECT_CALL(*delimitation, delimit(_, false))
		    .WillOnce(Return(nonstd::expected<std::vector<Bytes>, PipelineError>(
		        std::vector<Bytes>{to_bytes("record one")})));

		EXPECT_CALL(*parsing, parse(_))
		    .WillOnce(Return(
		        nonstd::expected<Record, RecordError>(nonstd::make_unexpected(RecordError{}))));

		Engine engine(std::move(acquisition_owned), std::move(delimitation_owned),
		              std::move(parsing_owned), ErrorPolicy::FailFast);

		auto result = engine.next();

		ASSERT_FALSE(result.has_value());
		EXPECT_EQ(result.error().code, std::make_error_code(std::errc::protocol_error));
		EXPECT_EQ(result.error().message, "record parsing failed");
	}

	// Parse fails on the first record under Skip. next() discards it and
	// returns the second (ADR-013).
	TEST(EngineTest, SkipsFailingRecord) {
		auto acquisition_owned = std::make_unique<MockBytesAcquisition>();
		auto delimitation_owned = std::make_unique<MockRecordDelimitation>();
		auto parsing_owned = std::make_unique<MockRecordParsing>();

		auto* acquisition = acquisition_owned.get();
		auto* delimitation = delimitation_owned.get();
		auto* parsing = parsing_owned.get();

		Bytes bytes = to_bytes("bad\ngood\n");

		Record expected_record_2;
		expected_record_2.set("raw", std::string{"good"});

		EXPECT_CALL(*acquisition, read())
		    .WillOnce(Return(nonstd::expected<Bytes, PipelineError>(bytes)));

		EXPECT_CALL(*delimitation, delimit(_, false))
		    .WillOnce(Return(nonstd::expected<std::vector<Bytes>, PipelineError>(
		        std::vector<Bytes>{to_bytes("bad"), to_bytes("good")})));

		EXPECT_CALL(*parsing, parse(_))
		    .WillOnce(Return(
		        nonstd::expected<Record, RecordError>(nonstd::make_unexpected(RecordError{}))))
		    .WillOnce(Return(nonstd::expected<Record, RecordError>(expected_record_2)));

		Engine engine(std::move(acquisition_owned), std::move(delimitation_owned),
		              std::move(parsing_owned), ErrorPolicy::Skip);

		auto result = engine.next();

		ASSERT_TRUE(result.has_value());
		ASSERT_TRUE(result.value().has_value());
		// NOLINTNEXTLINE(bugprone-unchecked-optional-access): guarded by ASSERT_TRUE above
		const auto& record = *result.value();
		EXPECT_EQ(record, expected_record_2);
	}

	// After the first nullopt, further next() calls stay nullopt without
	// calling read() again (acquisition_exhausted_ guards the loop).
	TEST(EngineTest, NulloptIsStableAfterEndOfStream) {
		auto acquisition_owned = std::make_unique<MockBytesAcquisition>();
		auto delimitation_owned = std::make_unique<MockRecordDelimitation>();
		auto parsing_owned = std::make_unique<MockRecordParsing>();

		auto* acquisition = acquisition_owned.get();
		auto* delimitation = delimitation_owned.get();

		EXPECT_CALL(*acquisition, read())
		    .WillOnce(Return(nonstd::expected<Bytes, PipelineError>(Bytes{})));

		EXPECT_CALL(*delimitation, delimit(_, true))
		    .WillOnce(
		        Return(nonstd::expected<std::vector<Bytes>, PipelineError>(std::vector<Bytes>{})));

		Engine engine(std::move(acquisition_owned), std::move(delimitation_owned),
		              std::move(parsing_owned), ErrorPolicy::FailFast);

		auto result_1 = engine.next();
		auto result_2 = engine.next();

		ASSERT_TRUE(result_1.has_value());
		EXPECT_FALSE(result_1.value().has_value());

		ASSERT_TRUE(result_2.has_value());
		EXPECT_FALSE(result_2.value().has_value());
	}

} // namespace cpe::test
