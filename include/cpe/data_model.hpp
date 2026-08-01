#pragma once

#include <iterator>
#include <ostream>
#include <string>
#include <utility> // std::pair
#include <variant>
#include <vector>

namespace cpe {
	struct Value; // forward declaration: Fields and Value itself name Value before it is defined

	// Fields: name-to-value mapping that Record and Object share (ADR-003)
	class Fields {
	public:
		// Adds a field. Returns false without modifying the record if the name
		// already exists (ADR-003: names are unique). Will be modified when error model appears.
		bool set(std::string const& field, Value const& value);

		// Returns nullptr if the field does not exist. Distinguishes "no field"
		// from "field exists with a null Value" (ADR-001's null category),
		[[nodiscard]] const Value* get(std::string const& field) const;

		// [[nodiscard]] tells the compiler to warn the user if the result (bool) is not used
		// noexcept promises no exception will be thrown
		[[nodiscard]] bool contains(std::string const& field) const noexcept;

		// `using` to create aliases for iterator types (better readability)
		// NOLINTNEXTLINE(readability-identifier-naming): STL container nested-type convention
		using iterator = std::vector<std::pair<std::string, Value>>::iterator;
		// NOLINTNEXTLINE(readability-identifier-naming): STL container nested-type convention
		using const_iterator = std::vector<std::pair<std::string, Value>>::const_iterator;
		iterator begin();
		iterator end();
		[[nodiscard]] const_iterator begin() const;
		[[nodiscard]] const_iterator end() const;

	private:
		std::vector<std::pair<std::string, Value>> entries_;
	};

	// overloading stream insertion for easier values & fields printing
	std::ostream& operator<<(std::ostream& o, Fields const& f);
	std::ostream& operator<<(std::ostream& o, Value const& v);

	// overloading equality comparison operator for Fields
	bool operator==(Fields const& f1, Fields const& f2);

	// Record and Object have the same behavior (ADR-003), but their Role is different
	// Record is what flows through the pipeline. Object is a nested value within a Record.
	class Record : public Fields {};
	class Object : public Fields {};

	// A field value: the categories from ADR-001 (scalars + composites).
	// Composites sit behind standard containers, whose heap storage gives
	// Value a fixed size despite being recursive (ADR-002).
	struct Value : std::variant<std::monostate, // the "null" slot (a variant is never empty).
	                            bool,
	                            double, // number (IEEE 754 binary64, ADR-004)
	                            std::string,
	                            Object,               // object (same contract as Record, ADR-003)
	                            std::vector<Value>> { // array
		using variant::variant; // using-declaration so Value inherits the std::variant's
		                        // constructor
	};

	// overloading equality comparison operator for Value
	bool operator==(Value const& v1, Value const& v2);

} // namespace cpe
