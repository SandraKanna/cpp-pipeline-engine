#pragma once

#include <cpe/deserialization/record_delimitation.hpp>
#include <cpe/error/pipeline_error.hpp>

#include <cstddef>              // std::byte
#include <nonstd/expected.hpp> // nonstd::expected
#include <vector>              // std::vector

namespace cpe {
	/// finds line boundaries (\n and \r\n) in a byte stream.
	class LineDelimitation: public RecordDelimitation {
	public:
		/// delimit() as specified by RecordDelimitation
		[[nodiscard]] nonstd::expected<std::vector<std::vector<std::byte>>, PipelineError>
		delimit(std::vector<std::byte> chunk, bool is_final) override;

	private:
		std::vector<std::byte> pending_; // incomplete tail carried across calls
	};
} // namespace cpe
