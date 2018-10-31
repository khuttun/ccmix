#ifndef CCMIX_FIELD_SPEC_HPP
#define CCMIX_FIELD_SPEC_HPP

namespace ccmix {

class field_spec
{
public:
    constexpr field_spec(unsigned int left, unsigned int right) : value(8 * left + right) {}
    constexpr field_spec(unsigned int opcode_mod) : value(opcode_mod) {}
    static constexpr field_spec all() { return field_spec{0, 5}; }
    constexpr unsigned int left() const { return value / 8; }
    constexpr unsigned int right() const { return value % 8; }
    constexpr unsigned int as_opcode_mod() const { return value; }

private:
    unsigned int value;
};

}

#endif
