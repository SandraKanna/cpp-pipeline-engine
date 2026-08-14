#pragma once

#include <cpe/error/pipeline_error.hpp>
#include <cstddef>	// std::byte
#include <nonstd/expected.hpp>	// nonstd::expected
#include <vector>	// std::vector

namespace cpe {
	/// Abstract contract for a component that finds record boundaries in a byte stream.
	///
	/// A concrete component recognizes the frontier that defines a record for a
	/// given format (newline, balanced braces, length prefix). Reports only
	/// pipeline errors; malformed content within a record is caught by the parsing
	/// stage (see ADR-008).
	class RecordDelimitation {
	public:
		// Polymorphic base: non-copyable and non-movable to prevent slicing
		// through a reference to the base. Derived components manage their own
		// copy/move semantics according to what they hold.
		RecordDelimitation() = default;
		RecordDelimitation(const RecordDelimitation&) = delete;
		RecordDelimitation& operator=(const RecordDelimitation&) = delete;
		RecordDelimitation(RecordDelimitation&&) = delete;
		RecordDelimitation& operator=(RecordDelimitation&&) = delete;
		virtual ~RecordDelimitation() = default;

		
		/// Delimits records from a chunk of bytes. Bytes not consumed are held
		/// across calls until enough arrive to complete a frontier.
		///
		/// is_final: true on the last call, after acquisition signals end of
		///           stream. The component resolves any bytes it still holds.
		///
		/// returns the delimited records (empty vector if the chunk did not
		///         complete any), or a PipelineError.
		// nonstd::expected becomes std::expected in C++23; only the include and namespace change
		[[nodiscard]] virtual nonstd::expected<std::vector<std::vector<std::byte>>, PipelineError>
		delimit(std::vector<std::byte> chunk, bool is_final) = 0;
	};
}	// namespace cpe
