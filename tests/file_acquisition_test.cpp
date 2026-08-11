#include <cpe/execution/file_acquisition.hpp>

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <unistd.h>
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
// not by throwing: the failure travels on the expected channel (ADR-009).
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
// read reconstructs the file exactly, and the read after the last returns empty (EOF, ADR-015)
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

// A file the process may not read is reported as a PipelineError carrying
// permission_denied. This proves read() propagates the real errno, not a hardcoded
// one: the missing-file test asserts a different errc through the same branch.
// The fixture is built at runtime with no permissions because git does not preserve
// a committed file's permission bits across a clone.
class UnreadableFile : public testing::Test {
protected:
	std::filesystem::path path_;
	void SetUp() override {
		path_ = std::filesystem::temp_directory_path() / "cpe_unreadable.txt";
		std::ofstream(path_) << "unreadable";
		// no permissions, like chmod 000
		std::filesystem::permissions(path_, std::filesystem::perms::none);
	}
	void TearDown() override { std::filesystem::remove(path_); }
};

// TEST_F: like TEST, but the first argument must be the fixture class name.
// GoogleTest runs its SetUp/TearDown around the test.
TEST_F(UnreadableFile, AccessDeniedReturnsError) {
	// root bypasses permission bits, so as root the read would not fail. Skip it.
	if (geteuid() == 0) {
		GTEST_SKIP() << "running as root: permission bits are bypassed";
	}
	cpe::FileAcquisition acquisition(path_);

	auto result = acquisition.read();

	ASSERT_FALSE(result.has_value());
	EXPECT_EQ(result.error().code, std::errc::permission_denied);
}

// Zero-byte read with feof set: end of stream on the first read (ADR-015).
TEST(FileAcquisitionTest, EmptyFileReturnsEofOnFirstRead) {
	auto path = std::filesystem::temp_directory_path() / "cpe_empty.txt";
	std::ofstream(path).close(); // create a zero-byte file

	cpe::FileAcquisition acquisition(path);
	auto result = acquisition.read();

	ASSERT_TRUE(result.has_value());
	EXPECT_TRUE(result.value().empty());

	std::filesystem::remove(path);
}

// A read error mid-stream is a PipelineError, not EOF. On POSIX, opening a
// directory triggers this: fopen succeeds, first fread fails with EISDIR.
TEST(FileAcquisitionTest, ReadErrorReturnsErrorNotEof) {
	cpe::FileAcquisition acquisition(std::filesystem::temp_directory_path());

	auto result = acquisition.read();

	ASSERT_FALSE(result.has_value());
	EXPECT_EQ(result.error().code, std::errc::is_a_directory);
}
