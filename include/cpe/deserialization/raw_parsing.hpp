#pragma once

#include <cpe/bytes.hpp>
#include <cpe/data_model.hpp>
#include <cpe/deserialization/record_parsing.hpp>
#include <cpe/error/record_error.hpp>

#include <nonstd/expected.hpp>	// nonstd::expected

namespace cpe {
	class RawParsing : public RecordParsing {
	public:
		[[nodiscard]] nonstd::expected<Record, RecordError> parse(Bytes raw) override;
	};
} // namespace cpe
