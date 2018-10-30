#include "ccmix/word.hpp"

namespace ccmix {

void test_value()
{
	static_assert(word{-1'073'741'823}.value() == -1'073'741'823, "");
	static_assert(word{-1000}.value() == -1000, "");
	static_assert(word{-1}.value() == -1, "");
	static_assert(word{0}.value() == 0, "");
	static_assert(word{1}.value() == 1, "");
	static_assert(word{1000}.value() == 1000, "");
	static_assert(word{1'073'741'823}.value() == 1'073'741'823, "");

	static_assert(word{42, 42, 42, 42}.value() == 0xAAAAAA, "");
	static_assert(word{42, -42, 42, 42}.value() == -0xAAAAAA, "");

	static_assert(word{10, -100, 3, 25}.opcode() == 10, "");
	static_assert(word{10, -100, 3, 25}.opcode_mod() == 25, "");
	static_assert(word{10, -100, 3, 25}.index_spec() == 3, "");
	static_assert(word{10, -100, 3, 25}.address() == -100, "");
}

void test_field()
{
	constexpr word test_word{-0b000001'000011'000111'001111'011111};

	static_assert(test_word.field(field_spec{0, 0}).value() == 0, "");
	static_assert(test_word.field(field_spec{0, 2}).value() == -0b000001'000011, "");
	static_assert(test_word.field(field_spec::all()).value() == test_word.value(), "");
	static_assert(test_word.field(field_spec{1, 5}).value() == -test_word.value(), "");
	static_assert(test_word.field(field_spec{4, 4}).value() == 0b001111, "");
	static_assert(test_word.field(field_spec{4, 5}).value() == 0b001111'011111, "");

	static_assert(!word{1}.field(field_spec{0, 0}).negative(), "");
	static_assert(word{-1}.field(field_spec{0, 0}).negative(), "");
}

constexpr auto set_field(word lhs, word rhs, field_spec f)
{
	lhs.set_field(f, rhs);
	return lhs.value();
}

void test_set_field()
{
	constexpr word a{-0b000001'000010'000011'000100'000101};
	constexpr word b{ 0b000110'000111'001000'001001'000000};

	static_assert(set_field(a, b, field_spec::all()) == b.value(), "");
	static_assert(set_field(a, b, field_spec{1, 5}) == -b.value(), "");
	static_assert(set_field(a, b, field_spec{5, 5}) == -0b000001'000010'000011'000100'000000, "");
	static_assert(set_field(a, b, field_spec{2, 2}) == -0b000001'000000'000011'000100'000101, "");
	static_assert(set_field(a, b, field_spec{2, 3}) == -0b000001'001001'000000'000100'000101, "");
	static_assert(set_field(a, b, field_spec{0, 1}) ==  0b000000'000010'000011'000100'000101, "");
}

}
