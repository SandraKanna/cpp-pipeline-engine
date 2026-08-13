#pragma once

#include <cpe/execution/bytes_acquisition.hpp>
#include <cstddef>	// std::byte
#include <cstdio>	// std::FILE*
#include <filesystem>	// std::filesystem::path
#include <memory>	// std::unique_ptr
#include <nonstd/expected.hpp>	// nonstd::expected
#include <vector>	// std::vector

namespace cpe {
	struct FileDeleter {
		// This functor closes the FILE* owned by unique_ptr. noexcept because it runs
		// from the unique_ptr destructor, which must not let exceptions escape.
    	void operator()(std::FILE* f) const noexcept { std::fclose(f); }
	};

	/// Acquires bytes from a file, read lazily in fixed-size chunks.
	///
	/// The file is opened on the first read(), not at construction, so a failure
	/// to open is reported as a PipelineError through read() rather than from the
	/// constructor. Each read() returns the next chunk; an empty vector signals
	/// the end of the file. The file is closed when the component is destroyed.
	class FileAcquisition : public BytesAcquisition {
	public:
		/// path: the file to read; not opened until the first read().
		explicit FileAcquisition(std::filesystem::path path);

		/// read() as specified by BytesAcquisition
		[[nodiscard]] nonstd::expected<std::vector<std::byte>, PipelineError> read() override;
		
	private:
		// std::FILE: C-style FILE pointer, preferred here so we can consult errno
		std::unique_ptr<std::FILE, FileDeleter> file_;
		std::filesystem::path path_;
	};
} // namespace cpe
