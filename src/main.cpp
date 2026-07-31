#include <cpe/data_model.hpp>
#include <iostream>

int main() {
	std::cout << "--- cpp-pipeline-engine (cpe) ---\n\n";

	// ----- Value ----- //

	cpe::Value number = 3.14;
	cpe::Value text = std::string("hello");
	cpe::Value flag = true;
	cpe::Value empty = std::monostate{};

	std::cout << std::boolalpha; // prints booleans as true/false instead of 1/0

	std::cout << "number is double: " << std::holds_alternative<double>(number) << '\n';

	std::cout << "text is string: " << std::holds_alternative<std::string>(text) << '\n';

	std::cout << "flag is bool: " << std::holds_alternative<bool>(flag) << '\n';

	std::cout << "empty is null: " << std::holds_alternative<std::monostate>(empty) << "\n\n";

	// ----- Record ----- //

	cpe::Record record;

	std::cout << "Insert 'pi': " << record.set("pi", number) << '\n';

	std::cout << "Insert duplicate 'pi': " << record.set("pi", 42.0) << '\n';

	std::cout << "Contains 'pi': " << record.contains("pi") << '\n';

	std::cout << "Contains 'answer': " << record.contains("answer") << '\n';

	if (record.get("pi")) {
		std::cout << "'pi' exists\n";
	}

	if (record.get("missing") == nullptr) {
		std::cout << "'missing' not found\n";
	}

	record.set("num", 42.0);
	record.set("string", "this is a string");

	std::cout << "\nFields:\n";
	for (const auto& [name, value] : record) {
		std::cout << " - " << name << '\n';
	}

	// ----- ostream overload ----- //
	std::cout << "\nThis is Record: \n" << record;
	number = 99.99;
	std::cout << "number: " << number << std::endl;
	text = "new text";
	std::cout << "text: " << text << std::endl;
	std::cout << "flag: " << flag << std::endl;
	std::cout << "empty: " << empty << std::endl;

	std::cout << "\n... an object: \n";
	cpe::Object addr;
	addr.set("city", std::string("Strasbourg"));
	addr.set("street", "rue machin");
	addr.set("postcode", "67000");
	std::cout << addr;

	std::cout << "\n...and another record: \n";
	cpe::Record record2;
	record2.set("address", addr);
	record2.set("scores", std::vector<cpe::Value>{1.0, 2.0, 3.0});
	std::cout << record2 << std::endl;

	// ----- equality comparison overload ----- //
	std::cout << "\nrecord == record2: " << (record == record2) << std::endl;
	std::cout << "record2 == record2: " << (record2 == record2) << std::endl;
	std::cout << "text == text: " << (text == "new text") << std::endl;
	std::cout << "number == Value(90.0): " << (number == cpe::Value(90.0)) << std::endl;
	std::cout << "number == 99.99: " << (number == 99.99) << std::endl;
}