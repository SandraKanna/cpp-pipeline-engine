#include <cpe/bytes.hpp>
#include <cpe/data_model.hpp>
#include <cpe/error/record_error.hpp>
#include <cpe/serialization/raw_serialization.hpp>

#include <nonstd/expected.hpp>	// nonstd::expected

#include <cstddef> // std::size_t, std::byte
#include <iterator> // std::distance
#include <string> // std::string
#include <variant> // std::holds_alternative, std::get

namespace cpe {
	namespace {
		Bytes to_bytes(const std::string& s) {
			Bytes bytes(s.size());
			for (std::size_t i = 0; i < s.size(); ++i) {
				bytes[i] = static_cast<std::byte>(s[i]);
			}
			return bytes;
		}
	} // namespace

	nonstd::expected<Bytes, RecordError> RawSerialization::serialize(Record r) {
		if (std::distance(r.begin(), r.end()) != 1) {
			return nonstd::make_unexpected(RecordError{});
		}
		const Value* raw_value = r.get("raw");
		if (raw_value == nullptr) {
			return nonstd::make_unexpected(RecordError{});
		}
		if (!std::holds_alternative<std::string>(*raw_value)) {
			return nonstd::make_unexpected(RecordError{});
		}
		return to_bytes(std::get<std::string>(*raw_value));
	}
} // namespace cpe
