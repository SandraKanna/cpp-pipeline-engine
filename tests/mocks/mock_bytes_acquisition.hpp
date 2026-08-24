#pragma once

#include <cpe/acquisition/bytes_acquisition.hpp>
#include <cpe/bytes.hpp>
#include <cpe/error/pipeline_error.hpp>

#include <nonstd/expected.hpp>  // nonstd::expected

#include <gmock/gmock.h>

namespace cpe::test {
	class MockBytesAcquisition : public BytesAcquisition {
	public:
		MOCK_METHOD((nonstd::expected<Bytes, PipelineError>), read, (), (override));
	};
} // namespace cpe::test
