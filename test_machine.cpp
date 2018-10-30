#include "ccmix/machine.hpp"
#include <initializer_list>
#include <utility>

namespace ccmix {

constexpr auto test_lda(int value_to_load)
{
    machine m;
    m.memory[0] = word{LDA, 1000};
    m.memory[1] = word{SPECIAL, 0, 0, HLT};
    m.memory[1000] = word{value_to_load};
    m.run();
    return m.reg_a.value();
}

static_assert(test_lda(-123) == -123, "");

constexpr auto test_indexed_field_load(int full_value, unsigned int left, unsigned int right)
{
	machine m;
	m.memory[0] = word{LD6, 500};
	m.memory[1] = word{LDX, -1, 6, field_spec{left, right}.as_opcode_mod()};
	m.memory[2] = word{SPECIAL, 0, 0, HLT};
	m.memory[500] = word{600};
	m.memory[599] = word{full_value};
    m.run();
    return m.reg_x.value();
}

static_assert(test_indexed_field_load(-1234, 1, 5) == 1234, "");

constexpr auto test_sta(int full_value, unsigned int left, unsigned int right)
{
    machine m;
    m.reg_a = word{full_value};
    m.memory[0] = word{STA, 1000, 0, field_spec{left, right}.as_opcode_mod()};
    m.memory[1] = word{SPECIAL, 0, 0, HLT};
    m.run();
    return m.memory[1000].value();
}

static_assert(test_sta(0b000001'000011'000111'001111'011111, 2, 3) == 0b000000'001111'011111'000000'000000, "");

constexpr auto test_add(int a, int b)
{
    machine m;
    m.reg_a = word{a};
    m.memory[0] = word{ADD, 1000};
    m.memory[1] = word{SPECIAL, 0, 0, HLT};
    m.memory[1000] = word{b};
    m.run();
    return m.reg_a.value();
}

static_assert(test_add(100, 50) == 150, "");

constexpr auto test_sub(int a, int b)
{
    machine m;
    m.reg_a = word{a};
    m.memory[0] = word{SUB, 1000};
    m.memory[1] = word{SPECIAL, 0, 0, HLT};
    m.memory[1000] = word{b};
    m.run();
    return m.reg_a.value();
}

static_assert(test_sub(100, 102) == -2, "");

constexpr auto test_mul(int a, int b)
{
    machine m;
    m.reg_a = word{a};
    m.memory[0] = word{MUL, 1000};
    m.memory[1] = word{SPECIAL, 0, 0, HLT};
    m.memory[1000] = word{b};
    m.run();
    return m.reg_ax_value();
}

static_assert(test_mul(100, 50) == 5000, "");
static_assert(test_mul(1'000'000'000, -10) == -10'000'000'000, "");

constexpr auto test_div(std::int64_t a, int b)
{
    machine m;
    m.set_reg_ax_value(a);
    m.memory[0] = word{DIV, 1000};
    m.memory[1] = word{SPECIAL, 0, 0, HLT};
    m.memory[1000] = word{b};
    m.run();
    return std::make_pair(m.reg_a.value(), m.reg_x.value());
}

static_assert(test_div(100, 30) == std::make_pair(3, 10), "");
static_assert(test_div(-10'000'000'000, -999'999'999) == std::make_pair(10, -10), "");

constexpr auto test_addr_xfer(int init, int value, addr_xfer_opcode_mod op)
{
    machine m;
    m.reg_a = word{init};
    m.memory[0] = word{AXA, value, 0, op};
    m.memory[1] = word{SPECIAL, 0, 0, HLT};
    m.run();
    return m.reg_a.value();
}

static_assert(test_addr_xfer(5, 2, INC) == 7, "");
static_assert(test_addr_xfer(5, 2, DEC) == 3, "");
static_assert(test_addr_xfer(5, 2, ENT) == 2, "");
static_assert(test_addr_xfer(5, 2, ENN) == -2, "");

constexpr auto test_comparison(word reg, word mem, field_spec f)
{
    machine m;
    m.reg_a = reg;
    m.memory[0] = word{CMPA, 1000, 0, f.as_opcode_mod()};
    m.memory[1] = word{SPECIAL, 0, 0, HLT};
    m.memory[1000] = mem;
    m.run();
    return m.comparison_ind;
}

static_assert(test_comparison(word{-1}, word{1}, field_spec::all()) == comparison_result::LESS, "");
static_assert(test_comparison(word{1}, word{1}, field_spec::all()) == comparison_result::EQUAL, "");
static_assert(test_comparison(word{1}, word{-1}, field_spec::all()) == comparison_result::GREATER, "");
static_assert(test_comparison(word{0, false}, word{0, true}, field_spec::all()) == comparison_result::EQUAL, "");
static_assert(test_comparison(word{-1}, word{1}, field_spec{1, 5}) == comparison_result::EQUAL, "");
static_assert(test_comparison(word{-987654}, word{123456}, field_spec{0, 0}) == comparison_result::EQUAL, "");

constexpr auto test_jmp(jmp_opcode_mod op, int cmp_reg, int cmp_mem, int ret_jmp, int ret_no_jmp)
{
    machine m;
    m.reg_a = word{ret_jmp};
    m.reg_x = word{cmp_reg};
    m.memory[0] = word{CMPX, 10};
    m.memory[1] = word{JMP, 3, 0, op};
    m.memory[2] = word{AXA, ret_no_jmp, 0, ENT};
    m.memory[3] = word{SPECIAL, 0, 0, HLT};
    m.memory[10] = word{cmp_mem};
    m.run();
    return std::make_pair(m.reg_a.value(), m.reg_j.value());
}

static_assert(test_jmp(UNCOND, 0, 0, 101, 102) == std::make_pair(101, 2), "");
static_assert(test_jmp(UNCOND_SAVE_J, 0, 0, 101, 102) == std::make_pair(101, 0), "");

static_assert(test_jmp(ON_LESS, 0, 1, 101, 102).first == 101, "");
static_assert(test_jmp(ON_LESS, 1, 0, 101, 102).first == 102, "");
static_assert(test_jmp(ON_LESS, 1, 1, 101, 102).first == 102, "");

static_assert(test_jmp(ON_EQUAL, 0, 1, 101, 102).first == 102, "");
static_assert(test_jmp(ON_EQUAL, 1, 0, 101, 102).first == 102, "");
static_assert(test_jmp(ON_EQUAL, 1, 1, 101, 102).first == 101, "");

static_assert(test_jmp(ON_GREATER, 0, 1, 101, 102).first == 102, "");
static_assert(test_jmp(ON_GREATER, 1, 0, 101, 102).first == 101, "");
static_assert(test_jmp(ON_GREATER, 1, 1, 101, 102).first == 102, "");

static_assert(test_jmp(ON_GREATER_EQUAL, 0, 1, 101, 102).first == 102, "");
static_assert(test_jmp(ON_GREATER_EQUAL, 1, 0, 101, 102).first == 101, "");
static_assert(test_jmp(ON_GREATER_EQUAL, 1, 1, 101, 102).first == 101, "");

static_assert(test_jmp(ON_NOT_EQUAL, 0, 1, 101, 102).first == 101, "");
static_assert(test_jmp(ON_NOT_EQUAL, 1, 0, 101, 102).first == 101, "");
static_assert(test_jmp(ON_NOT_EQUAL, 1, 1, 101, 102).first == 102, "");

static_assert(test_jmp(ON_LESS_EQUAL, 0, 1, 101, 102).first == 101, "");
static_assert(test_jmp(ON_LESS_EQUAL, 1, 0, 101, 102).first == 102, "");
static_assert(test_jmp(ON_LESS_EQUAL, 1, 1, 101, 102).first == 101, "");

constexpr auto test_jmp_reg(jmp_reg_opcode_mod op, int reg, int ret_jmp, int ret_no_jmp)
{
    machine m;
    m.reg_a = word{ret_jmp};
    m.reg_i[0] = word{reg};
    m.memory[0] = word{J1, 2, 0, op};
    m.memory[1] = word{AXA, ret_no_jmp, 0, ENT};
    m.memory[2] = word{SPECIAL, 0, 0, HLT};
    m.run();
    return m.reg_a.value();
}

static_assert(test_jmp_reg(NEGATIVE, -1, 1, 0) == 1, "");
static_assert(test_jmp_reg(NEGATIVE, 0, 1, 0) == 0, "");
static_assert(test_jmp_reg(NEGATIVE, 1, 1, 0) == 0, "");

static_assert(test_jmp_reg(ZERO, -1, 1, 0) == 0, "");
static_assert(test_jmp_reg(ZERO, 0, 1, 0) == 1, "");
static_assert(test_jmp_reg(ZERO, 1, 1, 0) == 0, "");

static_assert(test_jmp_reg(POSITIVE, -1, 1, 0) == 0, "");
static_assert(test_jmp_reg(POSITIVE, 0, 1, 0) == 0, "");
static_assert(test_jmp_reg(POSITIVE, 1, 1, 0) == 1, "");

static_assert(test_jmp_reg(NONNEGATIVE, -1, 1, 0) == 0, "");
static_assert(test_jmp_reg(NONNEGATIVE, 0, 1, 0) == 1, "");
static_assert(test_jmp_reg(NONNEGATIVE, 1, 1, 0) == 1, "");

static_assert(test_jmp_reg(NONZERO, -1, 1, 0) == 1, "");
static_assert(test_jmp_reg(NONZERO, 0, 1, 0) == 0, "");
static_assert(test_jmp_reg(NONZERO, 1, 1, 0) == 1, "");

static_assert(test_jmp_reg(NONPOSITIVE, -1, 1, 0) == 1, "");
static_assert(test_jmp_reg(NONPOSITIVE, 0, 1, 0) == 1, "");
static_assert(test_jmp_reg(NONPOSITIVE, 1, 1, 0) == 0, "");

constexpr auto find_max(const std::initializer_list<int>& elements)
{
    machine m;

    // Store the elements to memory locations X+1 ... X+n
    constexpr auto X = 1000;
    auto i = 1;
    for (auto e : elements)
    {
        m.memory[X + i++] = word{e};
    }

    // Registers:
    // rA: max element (m)
    // rI1: element count (n)
    // rI2: index of max element (j)
    // rI3: loop index (k)

    m.reg_i[0] = word{static_cast<int>(elements.size())};

    // Init
    m.memory[0] = word{AX3, 0, 1, ENT}; // k <- n
    m.memory[1] = word{JMP, 4, 0, UNCOND}; // jump to "Change m"

    // Compare
    m.memory[2] = word{CMPA, X, 3};
    m.memory[3] = word{JMP, 6, 0, ON_GREATER_EQUAL}; // jump to "Decrease k" if m >= X[k]

    // Change m
    m.memory[4] = word{AX2, 0, 3, ENT}; // j <- k
    m.memory[5] = word{LDA, X, 3}; // m <- X[k]

    // Decrease k
    m.memory[6] = word{AX3, 1, 0, DEC};

    // All tested?
    m.memory[7] = word{J3, 2, 0, POSITIVE}; // jump to "Compare" if k > 0

    m.memory[8] = word{SPECIAL, 0, 0, HLT};

    m.run();

    return std::make_pair(m.reg_i[1].value(), m.reg_a.value());
}

static_assert(find_max({5}) == std::make_pair(1, 5), "");
static_assert(find_max({5, 1}) == std::make_pair(1, 5), "");
static_assert(find_max({1, 5}) == std::make_pair(2, 5), "");
static_assert(find_max({5, 5}) == std::make_pair(2, 5), "");
static_assert(find_max({4, 1234, 62, -3, -100, 141414, 10, 11}) == std::make_pair(6, 141414), "");

}
