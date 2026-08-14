#pragma once

#include <cpe/deserialization/record_delimitation.hpp>
#include <cpe/error/pipeline_error.hpp>
#include <cpe/bytes.hpp>

#include <vector>              // std::vector

#include <nonstd/expected.hpp> // nonstd::expected

namespace cpe {
	/// finds line boundaries (\n and \r\n) in a byte stream.
	class LineDelimitation: public RecordDelimitation {
	public:
		/// delimit() as specified by RecordDelimitation
		[[nodiscard]] nonstd::expected<std::vector<Bytes>, PipelineError>
		delimit(Bytes chunk, bool is_final) override;

	private:
		Bytes pending_; // incomplete tail carried across calls
	};
} // namespace cpe
