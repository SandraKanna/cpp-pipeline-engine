#pragma once

#include <cpe/bytes.hpp>
#include <cpe/data_model.hpp>
#include <cpe/deserialization/record_parsing.hpp>
#include <cpe/error/record_error.hpp>

#include <nonstd/expected.hpp>  // nonstd::expected

#include <gmock/gmock.h>

namespace cpe::test {
    class MockRecordParsing : public RecordParsing {
    public:
        MOCK_METHOD((nonstd::expected<Record, RecordError>), parse, (Bytes), (override));
    };
} // namespace cpe::test
