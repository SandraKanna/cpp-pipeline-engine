#pragma once

#include <cpe/acquisition/bytes_acquisition.hpp>
#include <cpe/error/pipeline_error.hpp>
#include <vector>   // std::vector
#include <cstddef>	// std::byte
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
		explicit BufferAcquisition(std::vector<std::byte> data);

		/// read() as specified by BytesAcquisition
		[[nodiscard]] nonstd::expected<std::vector<std::byte>, PipelineError> read() override;
	private:
		std::vector<std::byte> data_;
	};

} // namespace cpe
