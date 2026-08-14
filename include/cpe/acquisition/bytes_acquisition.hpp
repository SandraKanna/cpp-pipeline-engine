#pragma once

#include <cpe/error/pipeline_error.hpp>
#include <cpe/bytes.hpp>

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
		// Polymorphic base: non-copyable and non-movable to prevent slicing
		// through a reference to the base. Derived components manage their own
		// copy/move semantics according to what they hold.
		BytesAcquisition() = default;
		BytesAcquisition(const BytesAcquisition&) = delete;
		BytesAcquisition& operator=(const BytesAcquisition&) = delete;
		BytesAcquisition(BytesAcquisition&&) = delete;
		BytesAcquisition& operator=(BytesAcquisition&&) = delete;
		virtual ~BytesAcquisition() = default;

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
		[[nodiscard]] virtual nonstd::expected<Bytes, PipelineError> read() = 0;
	};

} // namespace cpe
