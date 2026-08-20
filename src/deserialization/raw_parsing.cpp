#include <cpe/bytes.hpp>
#include <cpe/data_model.hpp>
#include <cpe/deserialization/raw_parsing.hpp>
#include <cpe/error/record_error.hpp>

#include <nonstd/expected.hpp>	// nonstd::expected

#include <string>	// std::string

namespace cpe {
	nonstd::expected<Record, RecordError> RawParsing::parse(Bytes raw) {
		Value raw_value = std::string(reinterpret_cast<const char*>(raw.data()), raw.size());
		Record raw_record;
		// Fresh record: "raw" cannot exist yet, so set never fails here.
		raw_record.set("raw", raw_value);
		return raw_record;
	}
} // namespace cpe
