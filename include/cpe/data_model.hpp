#pragma once

#include <variant>
#include <vector>
#include <string>
#include <iterator>
#include <utility> // std::pair

namespace cpe {
	struct Value;   // forward declaration: Fields and Value itself name Value before it is defined

	class Fields {
	public:
		// Adds a field. Returns false without modifying the record if the name
		// already exists (ADR-003: names are unique). Will be modified when error model appears.
		bool set(std::string const& field, Value const& value);

		// Returns nullptr if the field does not exist. Distinguishes "no field"
		// from "field exists with a null Value" (ADR-001's null category),
		const Value* get(std::string const& field) const;

		// [[nodiscard]] tells the compiler to warn the user if the result (bool) is not used
		// noexcept promises no exception will be thrown
		[[nodiscard]] bool contains(std::string const& field) const noexcept;
	 	
		// `using` to create aliases for iterator types
		using iterator = std::vector<std::pair<std::string, Value>>::iterator;
		using const_iterator = std::vector<std::pair<std::string, Value>>::const_iterator;
		iterator begin();
		iterator end();
		const_iterator begin() const;
		const_iterator end() const ;

	private:
		std::vector<std::pair<std::string, Value>> entries_;
	};

	class Record : public Fields {};
	class Object : public Fields {};

	// A field value: the categories from ADR-001 (scalars + composites).
	// Composites sit behind standard containers, whose heap storage gives
	// Value a fixed size despite being recursive (ADR-002).
	struct Value : std::variant<
		std::monostate,			// the "null" slot (a variant is never empty).
		bool,					
		double,					// number (IEEE 754 binary64, ADR-004)
		std::string,
		Object, 				// object (same contract as Record, ADR-003)
		std::vector<Value>> {	// array
		using variant::variant;	// using-declaration so Value inherits the std::variant's constructor
	};
} // namespace cpe
