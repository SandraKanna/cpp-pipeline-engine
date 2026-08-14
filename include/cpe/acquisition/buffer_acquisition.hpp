#pragma once

#include <cpe/acquisition/bytes_acquisition.hpp>
#include <cpe/error/pipeline_error.hpp>
#include <cpe/bytes.hpp>

#include <nonstd/expected.hpp>	// nonstd::expected

namespace cpe {
	/// Acquires bytes from an in-memory buffer supplied at construction.
	///
	/// The first read() hands out the whole buffer and empties it; the next read()
	/// returns an empty vector, signalling end of stream.
	/// This component never fails.
	class BufferAcquisition: public BytesAcquisition {
	public:
		/// data: the bytes this component will own and hand to the pipeline.
		explicit BufferAcquisition(Bytes data);

		/// read() as specified by BytesAcquisition
		[[nodiscard]] nonstd::expected<Bytes, PipelineError> read() override;
	private:
		Bytes data_;
	};

} // namespace cpe
