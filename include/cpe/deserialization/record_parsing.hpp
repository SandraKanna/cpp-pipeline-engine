#pragma once

#include <cpe/bytes.hpp>
#include <cpe/data_model.hpp>
#include <cpe/error/record_error.hpp>

#include <nonstd/expected.hpp>	// nonstd::expected

namespace cpe {
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

		[[nodiscard]] virtual nonstd::expected<Record, RecordError> parse(Bytes raw) = 0;
	};
} // namespace cpe
