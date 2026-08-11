#include <cpe/execution/file_acquisition.hpp>

#include <cerrno> // errno
#include <cstdio>	// std::FILE* std::fopen std::ferror
#include <utility> // std::move
#include <cstddef>	// std::byte
#include <filesystem>	// std::filesystem::path
#include <nonstd/expected.hpp>	// nonstd::*
#include <system_error>	// std::error_code std::generic_category
#include <vector>	// std::vector

namespace cpe {

	FileAcquisition::FileAcquisition(std::filesystem::path path) : path_(std::move(path)) {}

	nonstd::expected<std::vector<std::byte>, PipelineError> FileAcquisition::read() {
		if (!file_) {
			// "rb": raw bytes, no newline translation. fopen guarantees errno on failure (POSIX).
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

		// 64 KB: conventional read-buffer size, tunable, not measured for this project.
		// 64UL so the whole multiplication happens in a wide unsigned type, never in int.
		static constexpr std::size_t chunk_size = 64UL * 1024;
		std::vector<std::byte> chunk(chunk_size);
		std::size_t n = std::fread(chunk.data(), 1, chunk_size, file_.get());

		// Capture errno right after fread, before any other call overwrites it,
		// same discipline as the fopen path above (guarantee errno on failure)
		int const err = errno;
		if (n == 0) {
			// fread returns 0 on both EOF and read error; the stream indicators tell them apart:
			// ferror for a mid-stream pipeline error and feof for normal EOF.
			if (std::ferror(file_.get()) != 0) {
				return nonstd::make_unexpected(PipelineError{
					.code = std::error_code(err, std::generic_category()),
					.message = "cannot read file: " + path_.string()});
			}
		}

		chunk.resize(n);
		return chunk;
	}

} // namespace cpe
