#include <cpe/execution/file_acquisition.hpp>

#include <cerrno>
#include <utility>

namespace cpe {

	FileAcquisition::FileAcquisition(std::filesystem::path path) : path_(std::move(path)) {}

	nonstd::expected<std::vector<std::byte>, PipelineError> FileAcquisition::read() {
		if (!file_) {
			// "rb": raw bytes, no newline translation. fopen sets errno on failure (POSIX).
			std::FILE* raw = std::fopen(path_.c_str(), "rb");
			if (raw == nullptr) {
				// Read errno on the line right after the failure: any call in between could
				// overwrite it. generic_category() maps it to std::errc for comparison.
				return nonstd::make_unexpected(PipelineError{
					.code = std::error_code(errno, std::generic_category()),
					.message = "cannot open file: " + path_.string()});
			}
			file_.reset(raw);
		}

		// 64UL so the whole multiplication happens in a wide unsigned type, never in int.
		static constexpr std::size_t chunk_size = 64UL * 1024;
		std::vector<std::byte> chunk(chunk_size);
		std::size_t n = std::fread(chunk.data(), 1, chunk_size, file_.get());
		chunk.resize(n);
		return chunk;
	}

} // namespace cpe
