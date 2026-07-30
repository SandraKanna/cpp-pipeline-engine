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

	std::ostream& operator<<(std::ostream& o, Fields const& f) {
		bool first = true;
		o << "{";
		for (const auto& [name, value] : f) {
			if (!first)
				o << ", ";
			first = false;
			o << name << " : " << value;
		}
		o << "}";
		return (o);
	}

	std::ostream& operator<<(std::ostream& o, Value const& v) {
		std::visit([&o](auto&& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, std::monostate>)
				o << "null";
			else if constexpr (std::is_same_v<T, bool>)
				o << std::boolalpha << arg;
				// o << (arg? "true" : "false");
			else if constexpr (std::is_same_v<T, double>)
				o << arg;
			else if constexpr (std::is_same_v<T, std::string>)
				o << arg;
			else if constexpr (std::is_same_v<T, Object>)
				o << arg;
			else if constexpr (std::is_same_v<T, std::vector<Value>>) {
				bool first = true;
				o << "[";
				for (const auto& item : arg) {
					if (!first)
						o << ", ";
					first = false;
					o << item;
				}
				o << "]";
			}
		}, v);
		return (o);
	}

} // namespace cpe