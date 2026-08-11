#include <cpe/execution/file_acquisition.hpp>

#include <cstddef>
#include <string>
#include <system_error>
#include <vector>

#include <gtest/gtest.h>

namespace {
	// Reads the string's characters as raw bytes, for comparing against file contents.
	std::vector<std::byte> to_bytes(const std::string& s) {
		std::vector<std::byte> bytes(s.size());
		for (std::size_t i = 0; i < s.size(); ++i) {
			bytes[i] = static_cast<std::byte>(s[i]);
		}
		return bytes;
	}
} // namespace

// A missing file is reported as a PipelineError carrying no_such_file_or_directory,
// not by throwing: the failure travels on the expected channel (ADR-009, ADR-011).
TEST(FileAcquisitionTest, MissingFileReturnsError) {
	cpe::FileAcquisition acquisition("/nonexistent/path/to/file.txt");

	auto result = acquisition.read();

	ASSERT_FALSE(result.has_value());
	EXPECT_EQ(result.error().code, std::errc::no_such_file_or_directory);
}

// A file smaller than one chunk comes back in a single read; the next read returns
// an empty vector, the end-of-stream signal (ADR-015).
TEST(FileAcquisitionTest, SmallFileReadInOneChunkThenEof) {
	cpe::FileAcquisition acquisition(CPE_FIXTURE_SMALL);

	auto first = acquisition.read();
	ASSERT_TRUE(first.has_value());
	EXPECT_EQ(first.value(), to_bytes("hello, pipeline\n"));

	auto second = acquisition.read();
	ASSERT_TRUE(second.has_value());
	EXPECT_TRUE(second.value().empty());
}

// A file larger than one chunk comes back across several reads. Concatenating every non-empty
// read reconstructs the file exactly, and the read after the last returns emtpy (EOF, ADR-015)
TEST(FileAcquisitionTest, LargeFileReadAcrossMultipleChunks) {
	cpe::FileAcquisition acquisition(CPE_FIXTURE_LARGE);

	std::vector<std::byte> all;
	int reads_with_data = 0;
	while (true) {
		auto result = acquisition.read();
		ASSERT_TRUE(result.has_value());
		if (result.value().empty()) {
			break;
		}
		++reads_with_data;
		all.insert(all.end(), result.value().begin(), result.value().end());
	}

	EXPECT_GT(reads_with_data, 1);
	EXPECT_EQ(all.size(), 120000U);
}
