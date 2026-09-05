#pragma once

#include <cpe/bytes.hpp>
#include <cpe/data_model.hpp>
#include <cpe/error/record_error.hpp>
#include <cpe/serialization/record_serialization.hpp>

#include <nonstd/expected.hpp> // nonstd::expected

#include <gmock/gmock.h>

namespace cpe::test {
    class MockRecordSerialization : public RecordSerialization {
    public:
        MOCK_METHOD((nonstd::expected<Bytes, RecordError>), serialize, (Record record), (override));
    };
} // namespace cpe::test
