#pragma once

#include <cpe/bytes.hpp>
#include <cpe/data_model.hpp>
#include <cpe/error/record_error.hpp>

#include <nonstd/expected.hpp>	// nonstd::expected

namespace cpe {
	/// Abstract contract for turning the bytes of a single delimited record into a Record.
	/// One-in-one-out: no state between calls, no end-of-stream signaling
	/// (delimitation already framed the record). Implementations may reject
	/// malformed input via RecordError (ADR-015, ADR-016).
	class RecordParsing {
	public:
		// Polymorphic base: non-copyable and non-movable to prevent slicing
		// through a reference to the base. Derived components manage their own
		// copy/move semantics according to what they hold.
		RecordParsing() = default;
		RecordParsing(const RecordParsing&) = delete;
		RecordParsing& operator=(const RecordParsing&) = delete;
		RecordParsing(RecordParsing&&) = delete;
		RecordParsing& operator=(RecordParsing&&) = delete;
		virtual ~RecordParsing() = default;

		/// Parses the bytes of one delimited record into a Record.
		///
		/// raw: the bytes of a single record, as produced by a delimitation
		///      component. Content only, without any framing byte.
		///
		/// returns the parsed Record, or a RecordError if the content cannot
		///         be interpreted under the component's format.
		// nonstd::expected becomes std::expected in C++23; only the include and namespace change
		[[nodiscard]] virtual nonstd::expected<Record, RecordError> parse(Bytes raw) = 0;
	};
} // namespace cpe
