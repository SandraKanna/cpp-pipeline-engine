#include <algorithm> // std::any_of std::find_if
#include <cpe/data_model.hpp>

namespace cpe {

	bool Fields::set(std::string const& field, Value const& value) {
		// set() forbids duplicate names (ADR-003), so the first match is the only match
		if (contains(field))
			return (false);
		// emplace_back: faster than push_back, builds the pair directly in the vector
		entries_.emplace_back(field, value);
		return (true);
	}

	const Value* Fields::get(std::string const& field) const {
		auto it = std::find_if(entries_.begin(), entries_.end(),
		                       [&field](auto const& pair) { return (pair.first == field); });
		if (it == entries_.end())
			return (nullptr);
		return (&it->second);
	}

	bool Fields::contains(std::string const& field) const noexcept {
		return (std::any_of(entries_.begin(), entries_.end(),
		                    [&field](auto const& pair) { return (pair.first == field); }));
	}

	Fields::iterator Fields::begin() { return entries_.begin(); }

	Fields::iterator Fields::end() { return entries_.end(); }
	Fields::const_iterator Fields::begin() const { return entries_.begin(); }
	Fields::const_iterator Fields::end() const { return entries_.end(); }

	// ----- OVERLOAD ----- //

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
		std::visit(
		    [&o](auto&& arg) {
			    using T = std::decay_t<decltype(arg)>;
			    // if constexpr: only the branch matching T is compiled
			    if constexpr (std::is_same_v<T, std::monostate>)
				    o << "null";
			    else if constexpr (std::is_same_v<T, bool>)
				    o << std::boolalpha << arg;
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
		    },
		    v);
		return (o);
	}

	bool operator==(Fields const& f1, Fields const& f2) {
		// std::equal compares element by element in order; order matters here (ADR-003)
		return (std::equal(f1.begin(), f1.end(), f2.begin(), f2.end()));
	}

	// Value equality, including comparison against a raw scalar (value == 99.99).
	// The inherited std::variant operator== is a template and cannot convert the
	// raw operand to Value; this non-template overload can.
	bool operator==(Value const& v1, Value const& v2) {
		return (static_cast<Value::variant const&>(v1) == static_cast<Value::variant const&>(v2));
	}

} // namespace cpe