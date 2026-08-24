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
// #include <system_error> // std::make_error_code, std::errc
#include <utility> // std::move

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

	// Fin de stream normal. Acquisition devuelve chunk vacío. El motor llama a delimit()
	// una última vez con is_final=true. Cuando la cola queda vacía, next() devuelve nullopt.

	// Pipeline error desde acquisition. Acquisition devuelve PipelineError. next() lo propaga tal
	// cual.

	// Pipeline error desde delimitation. Delimitation devuelve PipelineError. next() lo propaga tal
	// cual.

	// Record error con política FailFast. Parsing devuelve RecordError. next() devuelve
	// PipelineError (promoción).

	// Record error con política Skip. Parsing devuelve RecordError en el primer record
	// del chunk, ok en el segundo. next() devuelve el segundo record, saltando el primero.

	// nullopt es estable después de EOS. Después de que next() devuelva nullopt una vez,
	// llamadas posteriores siguen devolviendo nullopt sin volver a llamar a read()
	// (verifica que acquisition_exhausted_ cumple su función).

} // namespace cpe::test