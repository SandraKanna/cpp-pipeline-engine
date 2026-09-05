#pragma once

#include <cpe/bytes.hpp>
#include <cpe/delivery/bytes_delivery.hpp>
#include <cpe/error/pipeline_error.hpp>

#include <nonstd/expected.hpp> // nonstd::expected

#include <gmock/gmock.h>

namespace cpe::test {
    class MockBytesDelivery : public BytesDelivery {
    public:
        MOCK_METHOD((nonstd::expected<void, PipelineError>), deliver, (Bytes bytes), (override));
    };
} // namespace cpe::test
