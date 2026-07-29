#include <cpe/data_model.hpp>
#include <algorithm> // std::any_of std::find_if

namespace cpe {
	bool Fields::set(std::string const& field, Value const& value) {
		if (contains(field))
			return (false);
		entries_.emplace_back(field, value); // faster than push_back, builds the pair directly in the vector
		return (true);
	}

	const Value* Fields::get(std::string const& field) const {
		auto it = std::find_if(entries_.begin(), entries_.end(),
			[&field](auto const &pair){return (pair.first == field);});
		if (it == entries_.end())
			return (nullptr);	
		return (&it->second);
	}

	bool Fields::contains(std::string const& field) const noexcept {
		return (std::any_of(
			entries_.begin(), entries_.end(), 
			[&field](auto const &pair) 
			{return (pair.first == field);})
		);
	}

	Fields::iterator Fields::begin() { 
		return entries_.begin(); 
	}

	Fields::iterator Fields::end() { 
		return entries_.end(); 
	}
	Fields::const_iterator Fields::begin() const { 
		return entries_.begin(); 
	}
	Fields::const_iterator Fields::end() const { 
		return entries_.end(); 
	}

} // namespace cpe