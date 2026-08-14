#include <cpe/deserialization/line_delimitation.hpp>
#include <cpe/error/pipeline_error.hpp>
#include <cpe/bytes.hpp>

#include <vector>	// std::vector

#include <nonstd/expected.hpp>	// nonstd::expected

namespace cpe {
	nonstd::expected<std::vector<Bytes>, PipelineError>
	LineDelimitation::delimit(Bytes chunk, bool is_final) {
		// Join the tail carried from the previous call with the new bytes,
		// so the scan below sees one contiguous buffer.
		pending_.insert(pending_.end(), chunk.begin(), chunk.end());

		std::vector<Bytes> records;
		auto start = pending_.begin();
		for (auto it = pending_.begin(); it != pending_.end(); ++it) {
			if (*it == std::byte{'\n'}) {
				records.emplace_back(start, it);
				start = it + 1;
			}
		}

		// On end of stream, any trailing bytes without a closing '\n' are
		// emitted as the last record. Otherwise they become the new tail.
		if (is_final) {
			if (start != pending_.end()) {
				records.emplace_back(start, pending_.end());
			}
			pending_.clear();
		} else {
			pending_.erase(pending_.begin(), start);
		}

		return records;
	}
} // namespace cpe
