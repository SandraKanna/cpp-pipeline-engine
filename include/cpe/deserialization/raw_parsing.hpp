#pragma once

#include <cpe/bytes.hpp>
#include <cpe/data_model.hpp>
#include <cpe/deserialization/record_parsing.hpp>
#include <cpe/error/record_error.hpp>

#include <nonstd/expected.hpp>	// nonstd::expected

namespace cpe {
	/// Minimal RecordParsing that treats the input as opaque bytes: wraps the
	/// whole record as a single string field named "raw", without inspecting
	/// its content. Useful as a first producer of Records for components that
	/// do not need format interpretation (plain text streams, unparsed logs).
	/// Never fails: any byte sequence is a valid single-field record.
	class RawParsing : public RecordParsing {
	public:
		/// parse() as specified by RecordParsing
		[[nodiscard]] nonstd::expected<Record, RecordError> parse(Bytes raw) override;
	};
} // namespace cpe
