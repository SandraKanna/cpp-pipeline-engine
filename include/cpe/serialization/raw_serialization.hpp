#pragma once

#include <cpe/bytes.hpp>
#include <cpe/data_model.hpp>
#include <cpe/error/record_error.hpp>
#include <cpe/serialization/record_serialization.hpp>

#include <nonstd/expected.hpp>	// nonstd::expected

namespace cpe {
	/// Serializes Records with the shape {"raw": <string>} into bytes.
	///
	/// Accepts only Records with exactly one field named "raw" holding a string
	/// value: the inverse of RawParsing's output. Any other shape (zero fields,
	/// several fields, a different name, or a non-string value) is a RecordError.
	class RawSerialization : public RecordSerialization {
	public:
		/// serialize() as specified by RecordSerialization
		[[nodiscard]] nonstd::expected<Bytes, RecordError> serialize(Record r) override;
	};
} // namespace cpe
