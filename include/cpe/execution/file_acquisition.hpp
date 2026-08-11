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

	class FileAcquisition : public BytesAcquisition {
	public:
		explicit FileAcquisition(std::filesystem::path path);
		[[nodiscard]] nonstd::expected<std::vector<std::byte>, PipelineError> read() override;
		
	private:
		// std::FILE: C-style FILE pointer, preferred here so we can consult errno
		std::unique_ptr<std::FILE, FileDeleter> file_;
		std::filesystem::path path_;
	};
} // namespace cpe
