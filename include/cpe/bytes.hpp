#pragma once

#include <cstddef>     // std::byte
#include <string_view> // std::string_view
#include <vector>      // std::vector

namespace cpe {
	using Bytes = std::vector<std::byte>;

	// Reads the string's characters as raw bytes.
	Bytes to_bytes(std::string_view s);
} // namespace cpe
