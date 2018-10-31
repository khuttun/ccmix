# ccmix

ccmix is a simulator for [the MIX computer](https://en.wikipedia.org/wiki/MIX) written in fully constexpr C++. Finally there's a way to run MIX programs in your C++ compiler!

MIX programs written in ccmix look like this

```
machine m;

// Set initial values for registers
m.reg_a = word{0};
m.reg_i[0] = word{x};
m.reg_i[1] = word{y};

// The program
m.memory[0] = word{AXA, 0, 1, INC};
m.memory[1] = word{AXA, 0, 2, INC};
m.memory[2] = word{SPECIAL, 0, 0, HLT};

// Run, starting from memory location 0
m.run();
```

[See more complete live example in Compiler Explorer](https://godbolt.org/z/CbIWdA).

Note that ccmix doesn't simulate MIX completely. Missing features include at least overflow toggle, I/O and some operators (shifts, move...).
