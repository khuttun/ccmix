#include "ccmix/field_spec.hpp"
#include <cstdint>

namespace ccmix {

class word
{
public:
    static constexpr int n_bits()
    {
        return N_BYTES * byte::N_BITS;
    }

    constexpr word() {}

    constexpr word(unsigned int abs_value, bool negative)
    {
        neg = negative;
        for (int i = 0; i < N_BYTES; ++i)
        {
            data[i] = abs_value >> (i * byte::N_BITS);
        }
    }

    constexpr word(int value) : word(value < 0 ? -value : value, value < 0) {}

    constexpr word(
        unsigned int opcode,
        int addr,
        unsigned int index = 0,
        unsigned int mod = field_spec::all().as_opcode_mod())
    {
        data[OPCODE_BYTE] = opcode;
        data[MOD_BYTE] = mod;
        data[INDEX_BYTE] = index;

        neg = addr < 0;
        const unsigned int abs_addr = neg ? -addr : addr;
        data[ADDR_LOW_BYTE] = abs_addr;
        data[ADDR_HIGH_BYTE] = abs_addr >> byte::N_BITS;
    }

    constexpr bool negative() const
    {
        return neg;
    }

    constexpr unsigned int abs_value() const
    {
        unsigned int val = 0;

        for (int i = 0; i < N_BYTES; ++i)
        {
            val |= data[i] << (i * byte::N_BITS);
        }

        return val;
    }

    constexpr int value() const
    {
        const auto val = abs_value();
        return neg ? -val : val;
    }

    constexpr unsigned int opcode() const
    {
        return data[OPCODE_BYTE];
    }

    constexpr unsigned int opcode_mod() const
    {
        return data[MOD_BYTE];
    }

    constexpr unsigned int index_spec() const
    {
        return data[INDEX_BYTE];
    }

    constexpr int address() const
    {
        const auto a = data[ADDR_LOW_BYTE] | (data[ADDR_HIGH_BYTE] << byte::N_BITS);
        return neg ? -a : a;
    }

    constexpr word field(field_spec f) const
    {
        const auto left = f.left();
        const auto right = f.right();
        const auto include_sign = left == 0;
        const auto n_bytes = right - left + (include_sign ? 0 : 1);

        word result;

        for (unsigned int i = 0; i < n_bytes; ++i)
        {
            result.data[i] = data[N_BYTES - right + i];
        }

        if (include_sign)
        {
            result.neg = neg;
        }

        return result;
    }

    constexpr void set_field(field_spec f, word w)
    {
        const auto left = f.left();
        const auto right = f.right();
        const auto include_sign = left == 0;
        const auto n_bytes = right - left + (include_sign ? 0 : 1);

        for (unsigned int i = 0; i < n_bytes; ++i)
        {
            data[N_BYTES - right + i] = w.data[i];
        }

        if (include_sign)
        {
            neg = w.neg;
        }
    }

private:
    static constexpr int N_BYTES = 5;
    static constexpr int OPCODE_BYTE = 0;
    static constexpr int MOD_BYTE = 1;
    static constexpr int INDEX_BYTE = 2;
    static constexpr int ADDR_LOW_BYTE = 3;
    static constexpr int ADDR_HIGH_BYTE = 4;

    class byte
    {
    public:
        constexpr byte() : data(0) {}
        constexpr byte(unsigned int value) : data(value & MASK) {}
        constexpr operator unsigned int() const { return data; }

        static constexpr int N_BITS = 6;

    private:
        static constexpr std::uint8_t MASK = 0b00111111;
        std::uint8_t data;
    };

    bool neg = false;
    byte data[N_BYTES] = {};
};

}
