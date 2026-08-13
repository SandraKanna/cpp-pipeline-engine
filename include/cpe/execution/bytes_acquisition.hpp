#pragma once

#include <cpe/error/pipeline_error.hpp>
#include <vector>	// std::vector
#include <cstddef>	// std::byte
#include <nonstd/expected.hpp>	// nonstd::expected

namespace cpe {
	/// Abstract contract for a component that acquires raw bytes from an origin.
	///
	/// A concrete component obtains bytes from one kind of origin (a file, an
	/// in-memory buffer, a network endpoint) and hands them to the pipeline. Which origin,
	/// and how the bytes are obtained, is internal to the component: the pipeline
	/// only ever calls read().
	class BytesAcquisition {
	public:
		/// Produces the next block of bytes from the origin.
		///
		/// Each call advances through the stream: successive calls return
		/// successive blocks, never the same bytes twice. The block size is
		/// chosen by the component and may vary between calls.
		///
		/// returns a non-empty vector with the next block of bytes on success;
		///         an empty vector to signal end of stream, a normal outcome and
		///         not an error; or a PipelineError if the origin cannot be read.
		// nonstd::expected becomes std::expected in C++23; only the include and namespace change
		[[nodiscard]] virtual nonstd::expected<std::vector<std::byte>, PipelineError> read() = 0;
		virtual ~BytesAcquisition() = default;
	};

} // namespace cpe
