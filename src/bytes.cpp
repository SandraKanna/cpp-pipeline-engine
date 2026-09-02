#include <cpe/bytes.hpp>

#include <cstddef>     // std::byte
#include <string_view> // std::string_view

namespace cpe {
	Bytes to_bytes(std::string_view s) {
		Bytes out;
		out.reserve(s.size());
		for (char c : s) {
			out.push_back(static_cast<std::byte>(c));
		}
		return out;
	}
} // namespace cpe
