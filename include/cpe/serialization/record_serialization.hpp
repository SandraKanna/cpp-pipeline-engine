#pragma once

#include <cpe/bytes.hpp>
#include <cpe/data_model.hpp>
#include <cpe/error/record_error.hpp>

#include <nonstd/expected.hpp>	// nonstd::expected

namespace cpe {
	/// Abstract contract for a component that serializes a Record into bytes.
	///
	/// A concrete component encodes a Record into the byte representation of one
	/// format (CSV row, JSON object, log line). Which format, and how the encoding
	/// is produced, is internal to the component: the pipeline only ever calls
	/// serialize().
	class RecordSerialization {
	public:
		// Polymorphic base: non-copyable and non-movable to prevent slicing
		// through a reference to the base. Derived components manage their own
		// copy/move semantics according to what they hold.
		RecordSerialization() = default;
		RecordSerialization(const RecordSerialization&) = delete;
		RecordSerialization& operator=(const RecordSerialization&) = delete;
		RecordSerialization(RecordSerialization&&) = delete;
		RecordSerialization& operator=(RecordSerialization&&) = delete;
		virtual ~RecordSerialization() = default;

		/// Encodes a Record into its byte representation.
		///
		/// r: the Record to encode. Passed by value: the caller moves it in when
		///    it no longer needs the Record, or passes it plain to keep a copy.
		///
		/// returns the bytes that represent r in the component's format on
		///         success; or a RecordError if r does not fit what the
		///         component knows how to serialize.
		// nonstd::expected becomes std::expected in C++23; only the include and namespace change
		[[nodiscard]] virtual nonstd::expected<Bytes, RecordError> serialize(Record r) = 0;
	};
} // namespace cpe
