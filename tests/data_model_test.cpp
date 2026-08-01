#include <cpe/data_model.hpp>
#include <gtest/gtest.h>
#include <string>
#include <vector>

// ADR-001: a field set to null exists; it is not the same as an absent field.
TEST(Record, PresentNullDiffersFromAbsent) {
	cpe::Record rec;
	rec.set("a", cpe::Value{}); // present, holds null

	const cpe::Value* v = rec.get("a");
	ASSERT_NE(v, nullptr);       // present, even though null
	EXPECT_EQ(*v, cpe::Value{}); // and it compares equal to null

	EXPECT_EQ(rec.get("absent"), nullptr); // absent -> nullptr
}

// ADR-002: comparing a Value that holds an Object compares the fields inside it.
TEST(Value, NestedObjectEquality) {
	auto make = [](cpe::Value const& b) {
		cpe::Object obj;
		obj.set("a", cpe::Value{3.14});
		obj.set("b", b);
		return cpe::Value{obj};
	};

	const cpe::Value array = std::vector<cpe::Value>{1.0, 2.0};
	const cpe::Value text = std::string("something");

	EXPECT_EQ(make(array), make(array));
	EXPECT_NE(make(array), make(text));
}

// ADR-002: comparison reaches an array nested inside an object.
TEST(Value, NestedArrayEquality) {
	auto make = [](std::vector<cpe::Value> scores) {
		cpe::Object obj;
		obj.set("scores", scores);
		return cpe::Value{obj};
	};

	EXPECT_EQ(make({1.0, 2.0}), make({1.0, 2.0}));
	EXPECT_NE(make({1.0, 2.0}), make({1.0, 3.0}));
}

// ADR-003: a new name is stored and becomes accessible.
TEST(Record, SetNewNameSucceeds) {
	cpe::Record rec;
	EXPECT_TRUE(rec.set("a", cpe::Value{3.14}));
	EXPECT_TRUE(rec.contains("a"));
}

// ADR-003: a duplicate name is rejected and the original value is kept.
TEST(Record, SetDuplicateNameRejected) {
	cpe::Record rec;
	rec.set("a", cpe::Value{3.14});

	EXPECT_FALSE(rec.set("a", cpe::Value{2.0}));

	const cpe::Value* v = rec.get("a");
	ASSERT_NE(v, nullptr);           // stop here if absent: the next line would deref null
	EXPECT_EQ(*v, cpe::Value{3.14}); // first value shouldnt have been modified
}

// ADR-003: iteration follows insertion order, not sorted or arbitrary.
TEST(Record, PreservesInsertionOrder) {
	cpe::Record rec;
	rec.set("a", cpe::Value{1.0});
	rec.set("b", cpe::Value{2.0});
	rec.set("c", cpe::Value{3.0});

	std::vector<std::string> names;
	for (auto const& [name, value] : rec)
		names.push_back(name);

	EXPECT_EQ(names, (std::vector<std::string>{"a", "b", "c"}));
}

// ADR-003: same fields in a different order are not equal (order is meaningful).
TEST(Record, SameFieldsDifferentOrderNotEqual) {
	cpe::Record a;
	a.set("x", cpe::Value{1.0});
	a.set("y", cpe::Value{2.0});

	cpe::Record b;
	b.set("y", cpe::Value{2.0});
	b.set("x", cpe::Value{1.0});

	EXPECT_NE(a, b);
}
