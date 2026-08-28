#include <cpe/data_model.hpp>
#include <cpe/processing/processing.hpp>

#include <utility>	// std::move

namespace cpe {
	bool RecordValidation::operator()(const Record& r) const {
		return fn(r);
	}

	bool RecordFiltering::operator()(const Record& r) const {
		return fn(r);
	}

	Record RecordTransformation::operator()(Record r) const {
		return fn(std::move(r));
	}
} // namespace cpe
