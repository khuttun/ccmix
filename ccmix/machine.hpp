#include "ccmix/word.hpp"
#include <cstdint>

namespace ccmix {

enum opcode
{
    ADD = 1,
    SUB = 2,
    MUL = 3,
    DIV = 4,

    SPECIAL = 5,

    LDA = 8,
    LD1 = 9,
    LD2 = 10,
    LD3 = 11,
    LD4 = 12,
    LD5 = 13,
    LD6 = 14,
    LDX = 15,

    STA = 24,
    ST1 = 25,
    ST2 = 26,
    ST3 = 27,
    ST4 = 28,
    ST5 = 29,
    ST6 = 30,
    STX = 31,
    STJ = 32,
    STZ = 33,

    JMP = 39,
    JA = 40,
    J1 = 41,
    J2 = 42,
    J3 = 43,
    J4 = 44,
    J5 = 45,
    J6 = 46,
    JX = 47,

    AXA = 48,
    AX1 = 49,
    AX2 = 50,
    AX3 = 51,
    AX4 = 52,
    AX5 = 53,
    AX6 = 54,
    AXX = 55,

    CMPA = 56,
    CMP1 = 57,
    CMP2 = 58,
    CMP3 = 59,
    CMP4 = 60,
    CMP5 = 61,
    CMP6 = 62,
    CMPX = 63
};

enum special_opcode_mod
{
    HLT = 2
};

enum jmp_opcode_mod
{
    UNCOND = 0,
    UNCOND_SAVE_J = 1,
    ON_LESS = 4,
    ON_EQUAL = 5,
    ON_GREATER = 6,
    ON_GREATER_EQUAL = 7,
    ON_NOT_EQUAL = 8,
    ON_LESS_EQUAL = 9
};

enum jmp_reg_opcode_mod
{
    NEGATIVE = 0,
    ZERO = 1,
    POSITIVE = 2,
    NONNEGATIVE = 3,
    NONZERO = 4,
    NONPOSITIVE = 5
};

enum addr_xfer_opcode_mod
{
    INC = 0,
    DEC = 1,
    ENT = 2,
    ENN = 3
};

enum class comparison_result
{
    LESS,
    EQUAL,
    GREATER
};

class machine
{
public:
    constexpr void run()
    {
        auto halted = false;

        while (!halted)
        {
            const auto instruction = memory[pc];
            const auto opcode = instruction.opcode();

            // by default, the next instruction is the one in the following memory address
            auto next_pc = pc + 1;

            switch (opcode)
            {
                case ADD:
                    reg_a = word{reg_a.value() + load(instruction).value()};
                    break;

                case SUB:
                    reg_a = word{reg_a.value() - load(instruction).value()};
                    break;

                case MUL:
                    set_reg_ax_value(static_cast<std::int64_t>(reg_a.value()) * load(instruction).value());
                    break;

                case DIV:
                {
                    const auto rax = reg_ax_value();
                    const auto v = load(instruction).value();
                    reg_a = word{static_cast<int>(rax / v)};
                    reg_x = word{static_cast<int>(rax % v)};
                    break;
                }

                case SPECIAL:
                    switch (instruction.opcode_mod())
                    {
                        case HLT:
                            halted = true;
                            break;
                        default:
                            break;
                    }
                    break;

                case LDA:
                    reg_a = load(instruction);
                    break;

                case LD1:
                case LD2:
                case LD3:
                case LD4:
                case LD5:
                case LD6:
                    reg_i[opcode - LD1] = load(instruction);
                    break;

                case LDX:
                    reg_x = load(instruction);
                    break;

                case STA:
                    store(instruction, reg_a);
                    break;

                case ST1:
                case ST2:
                case ST3:
                case ST4:
                case ST5:
                case ST6:
                    store(instruction, reg_i[opcode - ST1]);
                    break;

                case STX:
                    store(instruction, reg_x);
                    break;

                case STJ:
                    store(instruction, reg_j);
                    break;

                case STZ:
                    store(instruction, word{});
                    break;

                case JMP:
                    next_pc = jump(instruction, next_pc);
                    break;

                case JA:
                    next_pc = jump_reg(instruction, reg_a, next_pc);
                    break;

                case J1:
                case J2:
                case J3:
                case J4:
                case J5:
                case J6:
                    next_pc = jump_reg(instruction, reg_i[opcode - J1], next_pc);
                    break;

                case JX:
                    next_pc = jump_reg(instruction, reg_x, next_pc);
                    break;

                case AXA:
                    reg_a = addr_xfer(instruction, reg_a);
                    break;

                case AX1:
                case AX2:
                case AX3:
                case AX4:
                case AX5:
                case AX6:
                    reg_i[opcode - AX1] = addr_xfer(instruction, reg_i[opcode - AX1]);
                    break;

                case AXX:
                    reg_x = addr_xfer(instruction, reg_x);
                    break;

                case CMPA:
                    comparison_ind = compare(instruction, reg_a);
                    break;

                case CMP1:
                case CMP2:
                case CMP3:
                case CMP4:
                case CMP5:
                case CMP6:
                    comparison_ind = compare(instruction, reg_i[opcode - CMP1]);
                    break;

                case CMPX:
                    comparison_ind = compare(instruction, reg_x);
                    break;

                default:
                    break;
            }

            pc = next_pc;
        }
    }

    constexpr std::int64_t reg_ax_value() const
    {
        const std::uint64_t abs = static_cast<std::uint64_t>(reg_a.abs_value()) << word::n_bits() | reg_x.abs_value();
        return reg_a.negative() ? -abs : abs;
    }

    constexpr void set_reg_ax_value(std::int64_t value)
    {
        const bool neg = value < 0;
        const std::uint64_t abs = neg ? -value : value;
        reg_x = word{static_cast<unsigned int>(abs), neg};
        reg_a = word{static_cast<unsigned int>(abs >> word::n_bits()), neg};
    }

    static constexpr auto memory_size = 4000;

    word reg_a;
    word reg_x;
    word reg_i[6] = {};
    word reg_j;
    word memory[memory_size] = {};
    int pc = 0;
    comparison_result comparison_ind = comparison_result::EQUAL;

private:
    constexpr int indexed_address(word instruction) const
    {
        const auto a = instruction.address();
        const auto i = instruction.index_spec();
        return i == 0 ? a : a + reg_i[i - 1].value();
    }

    constexpr word load(word instruction) const
    {
    	return memory[indexed_address(instruction)].field(field_spec(instruction.opcode_mod()));
    }

    constexpr word addr_xfer(word instruction, word reg) const
    {
        const auto m = indexed_address(instruction);
        switch (instruction.opcode_mod())
        {
            case INC:
                return word{reg.value() + m};
            case DEC:
                return word{reg.value() - m};
            case ENT:
                return m != 0 ? word{m} : word{0, instruction.negative()};
            case ENN:
                return m != 0 ? word{-m} : word{0, !instruction.negative()};
            default:
                return reg;
        }
    }

    constexpr comparison_result compare(word instruction, word reg) const
    {
        const auto reg_val = reg.field(field_spec(instruction.opcode_mod())).value();
        const auto mem_val = load(instruction).value();
        
        if (reg_val < mem_val)
        {
            return comparison_result::LESS;
        }

        if (reg_val > mem_val)
        {
            return comparison_result::GREATER;
        }

        return comparison_result::EQUAL;
    }

    constexpr void store(word instruction, word data)
    {
        memory[indexed_address(instruction)].set_field(field_spec(instruction.opcode_mod()), data);
    }

    constexpr int jump(word instruction, int next_pc)
    {
        const auto m = indexed_address(instruction);
        switch (instruction.opcode_mod())
        {
            case UNCOND:
                return jump_if(true, m, next_pc);
            case UNCOND_SAVE_J:
                return m;
            case ON_LESS:
                return jump_if(comparison_ind == comparison_result::LESS, m, next_pc);
            case ON_EQUAL:
                return jump_if(comparison_ind == comparison_result::EQUAL, m, next_pc);
            case ON_GREATER:
                return jump_if(comparison_ind == comparison_result::GREATER, m, next_pc);
            case ON_GREATER_EQUAL:
                return jump_if(comparison_ind == comparison_result::GREATER || comparison_ind == comparison_result::EQUAL, m, next_pc);
            case ON_NOT_EQUAL:
                return jump_if(comparison_ind == comparison_result::LESS || comparison_ind == comparison_result::GREATER, m, next_pc);
            case ON_LESS_EQUAL:
                return jump_if(comparison_ind == comparison_result::LESS || comparison_ind == comparison_result::EQUAL, m, next_pc);
            default:
                return next_pc;
        }
    }

    constexpr int jump_reg(word instruction, word reg, int next_pc)
    {
        const auto m = indexed_address(instruction);
        switch (instruction.opcode_mod())
        {
            case NEGATIVE:
                return jump_if(reg.value() < 0, m, next_pc);
            case ZERO:
                return jump_if(reg.value() == 0, m, next_pc);
            case POSITIVE:
                return jump_if(reg.value() > 0, m, next_pc);
            case NONNEGATIVE:
                return jump_if(reg.value() >= 0, m, next_pc);
            case NONZERO:
                return jump_if(reg.value() != 0, m, next_pc);
            case NONPOSITIVE:
                return jump_if(reg.value() <= 0, m, next_pc);
            default:
                return next_pc;
        }
    }

    constexpr int jump_if(bool cond, int jump_to, int next_pc)
    {
        if (cond)
        {
            reg_j = word{next_pc};
            return jump_to;
        }
        else
        {
            return next_pc;
        }
    }
};

}
