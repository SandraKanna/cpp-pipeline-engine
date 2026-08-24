#pragma once

#include <cpe/bytes.hpp>
#include <cpe/deserialization/record_delimitation.hpp>
#include <cpe/error/pipeline_error.hpp>

#include <nonstd/expected.hpp>  // nonstd::expected

#include <vector>

#include <gmock/gmock.h>

namespace cpe::test {
    class MockRecordDelimitation : public RecordDelimitation {
    public:
        MOCK_METHOD((nonstd::expected<std::vector<Bytes>, PipelineError>), delimit, (Bytes, bool), (override));
    };
} // namespace cpe::test
