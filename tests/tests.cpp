#include "catch/catch.hpp"
#include "../src/gba.h"
#include "cpu_state.h"
#include "expected_output.h"

#include <iostream>

// note for test cases: do not assume registers or memory values are set to 0 before starting
// a test. set them manually to 0 if you want them to be 0.

// Just a faster way to check flags
void check_flags_NZCV(bool fN, bool fZ, bool fC, bool fV) {
    REQUIRE(get_flag_N() == fN);
    REQUIRE(get_flag_Z() == fZ);
    REQUIRE(get_flag_C() == fC);
    REQUIRE(get_flag_V() == fV);
}

void wipe_registers() {
    for (int i = 0; i < NUM_REGISTERS; ++i) {
        memory.regs[i] = 0x00000000;
    }
}





TEST_CASE("CPU Thumb Mode - ADD Two Registers") {
    set_bit_T(true);
    wipe_registers();

    SECTION("ADD R1, R2 into R3") {
        memory.regs[1] = 0x00000001;
        memory.regs[2] = 0x00000001;
        execute(0x1853);

        REQUIRE(memory.regs[3] == 0x00000002);
        check_flags_NZCV(false, false, false, false);
    }
    wipe_registers();
}





TEST_CASE("CPU Thumb Mode - ADD Immediate Register") {
    set_bit_T(true);

    SECTION("ADD R2, #0x00") {
        memory.regs[2] = 0x00000000;
        execute(0x3200);

        REQUIRE(memory.regs[2] == 0x00000000);
        check_flags_NZCV(false, true, false, false);
    }

    SECTION("ADD R2, #0x01") {
        memory.regs[2] = 0x7FFFFFFF;
        execute(0x3201);
        REQUIRE(memory.regs[2] == 0x80000000);

        check_flags_NZCV(true, false, false, true);
    }

    SECTION("ADD R2, #0xFF (No Overflow)") {
        memory.regs[2] = 0x00000000;
        execute(0x32FF);

        REQUIRE(memory.regs[2] == 0x000000FF);
        check_flags_NZCV(false, false, false, false);
    }

    SECTION("ADD R2, #0xFF (Overflow)") {
        memory.regs[2] = 0xFFFFFFFF;
        execute(0x3280);
        
        REQUIRE(memory.regs[2] == 0x0000007F);
        check_flags_NZCV(false, false, true, true);
    }
    wipe_registers();
}


TEST_CASE("CPU Thumb Mode - ADD #1 Small Immediate Two Register") {
    set_bit_T(true);

    SECTION("ADD R3, R2, #0x0 (Add zero)") {
        memory.regs[2] = 0x00000000;
        execute(0x1C13);

        REQUIRE(memory.regs[3] == 0x00000000);

        check_flags_NZCV(false, true, false, false);
    }
    wipe_registers();

    SECTION("ADD R3, R2, #0x3 (Add small amount)") {
        memory.regs[2] = 0x00000000;
        execute(0x1CD3);
        REQUIRE(memory.regs[3] == 0x00000003);

        check_flags_NZCV(false, false, false, false);
    }
    wipe_registers();

    SECTION("ADD R3, R2, #0x1 (Add overflow)") {
        memory.regs[2] = 0x7FFFFFFF;
        execute(0x1C53);
        REQUIRE(memory.regs[3] == 0x80000000);

        check_flags_NZCV(true, false, false, true);
    }
    wipe_registers();

    SECTION("ADD R3, R2, #0x1 (Carry + Overflow)") {
        memory.regs[2] = 0xFFFFFFFF;
        execute(0x1C53);
        REQUIRE(memory.regs[3] == 0x00000000);

        check_flags_NZCV(false, true, true, false);
    }   
    wipe_registers();
}


TEST_CASE("CPU Thumb Mode - MOV Immediate") {
    set_bit_T(true);

    SECTION("MOV R2, #0xCD") {
        memory.regs[2] = 0x00000000;
        execute(0x22CD);
        REQUIRE(memory.regs[2] == 0xCD);
    }
    wipe_registers();
}





TEST_CASE("CPU Thumb Mode - LSL Immediate") {
    set_bit_T(true);

    set_flag_V(false);
    
    SECTION("LSL R2, R3, #0b00000") {
        memory.regs[2] = 0x00000000;
        set_flag_C(false);
        execute(0b00000'00000'010'011);

        REQUIRE(memory.regs[2] == 0x00000000);
        REQUIRE(memory.regs[3] == 0x00000000);
        check_flags_NZCV(false, true, false, false);
    }

    SECTION("LSL R2, R3, #0b00001") {
        memory.regs[2] = 0x00000001;
        execute(0b00000'00001'010'011);

        REQUIRE(memory.regs[2] == 0x00000001);
        REQUIRE(memory.regs[3] == 0x00000002);
        check_flags_NZCV(false, false, false, false);
    }

    SECTION("LSL R2, R3, #0b00100") {
        memory.regs[2] = 0xFFFFFFFF;
        execute(0b00000'00100'010'011);

        REQUIRE(memory.regs[2] == 0xFFFFFFFF);
        REQUIRE(memory.regs[3] == 0xFFFFFFF0);
        check_flags_NZCV(true, false, true, false);
    }
    wipe_registers();
}





TEST_CASE("CPU Thumb Mode - LSR Immediate") {
    set_bit_T(true);

    set_flag_V(false);

    SECTION("LSR R2, R3, #0b00000") {
        set_flag_C(false);

        memory.regs[2] = 0x00000000;
        execute(0b00001'00000'010'011);

        REQUIRE(memory.regs[2] == 0x00000000);
        REQUIRE(memory.regs[3] == 0x00000000);
        check_flags_NZCV(false, true, false, false);
    }

    SECTION("LSR R2, R3, #0b00001") {
        memory.regs[2] = 0x00000001;
        execute(0b00001'00001'010'011);

        REQUIRE(memory.regs[2] == 0x00000001);
        REQUIRE(memory.regs[3] == 0x00000000);
        check_flags_NZCV(false, true, true, false);
    }

    SECTION("LSR R2, R3, #0b00100") {
        memory.regs[2] = 0xFFFFFFFF;
        execute(0b00001'00100'010'011);

        REQUIRE(memory.regs[2] == 0xFFFFFFFF);
        REQUIRE(memory.regs[3] == 0x0FFFFFFF);
        check_flags_NZCV(false, false, true, false);
    }
    wipe_registers();
}





TEST_CASE("CPU Thumb Mode - Conditional Branches") {
    set_bit_T(true);

    SECTION("BEQ #0x02 (Simple Test)") {
        *memory.pc = 0x10000000;
        set_flag_Z(true);
        execute(0xD002);

        REQUIRE(*memory.pc == 0x10000006);
    }

    SECTION("BEQ #0xFE (Simple Test)") {
        *memory.pc = 0x10000000;
        set_flag_Z(true);
        execute(0xD0FE);

        REQUIRE(*memory.pc == 0x0FFFFFFE);
    }
}





TEST_CASE("CPU Thumb Mode - Logical AND") {
    set_bit_T(true);

    set_flag_C(true);
    set_flag_V(true);

    SECTION("AND R2, R3 (Carry)") {
        memory.regs[2] = 0xFF00FF88;
        memory.regs[3] = 0xF1111111;
        execute(0b010000'0000'010'011);

        REQUIRE(memory.regs[3] == 0xF1001100);
        check_flags_NZCV(true, false, true, true);
    }

    SECTION("AND R2, R3 (Zero)") {
        memory.regs[2] = 0x00000000;
        memory.regs[3] = 0x12345678;
        execute(0b010000'0000'010'011);

        REQUIRE(memory.regs[3] == 0x00000000);
        check_flags_NZCV(false, true, true, true);
    }
}





TEST_CASE("CPU Thumb Mode - Logical EOR") {
    set_bit_T(true);

    set_flag_C(true);
    set_flag_V(true);

    SECTION("EOR R2, R3 (Carry)") {
        memory.regs[2] = 0xFF00FF88;
        memory.regs[3] = 0x11111111;
        execute(0b010000'0001'010'011);

        REQUIRE(memory.regs[3] == 0xEE11EE99);
        check_flags_NZCV(true, false, true, true);
    }

    SECTION("AND R2, R3 (Zero)") {
        memory.regs[2] = 0x12345678;
        memory.regs[3] = 0x12345678;
        execute(0b010000'0001'010'011);

        REQUIRE(memory.regs[3] == 0x00000000);
        check_flags_NZCV(false, true, true, true);
    }
}






TEST_CASE("CPU Thumb Mode - Logical LSL") {
    set_bit_T(true);

    set_flag_V(true);

    SECTION("LSL R2, R3 (Shift == 0)") {
        set_flag_C(true);
        memory.regs[2] = 0xFFFF0000;
        memory.regs[3] = 0x12345678;
        execute(0b010000'0010'010'011);

        REQUIRE(memory.regs[3] == 0x12345678);
        check_flags_NZCV(false, false, true, true);
    }

    SECTION("LSL R2, R3 (Shift < 32)") {
        set_flag_C(true);
        memory.regs[2] = 0xFFFF0004;
        memory.regs[3] = 0x1F345678;
        execute(0b010000'0010'010'011);

        REQUIRE(memory.regs[3] == 0xF3456780);
        check_flags_NZCV(true, false, true, true);
    }

    SECTION("LSL R2, R3 (Shift == 32)") {
        set_flag_C(true);
        memory.regs[2] = 0xFFFF0020;
        memory.regs[3] = 0x1F345679;
        execute(0b010000'0010'010'011);

        REQUIRE(memory.regs[3] == 0x00000000);
        check_flags_NZCV(false, true, true, true);
    }

    SECTION("LSL R2, R3 (Shift > 32)") {
        set_flag_C(true);
        memory.regs[2] = 0xFFFF0021;
        memory.regs[3] = 0x1F345678;
        execute(0b010000'0010'010'011);

        REQUIRE(memory.regs[3] == 0x00000000);
        check_flags_NZCV(false, true, false, true);
    }
}





TEST_CASE("CPU Thumb Mode - Logical LSR") {
    set_bit_T(true);

    set_flag_V(true);

    SECTION("LSR R2, R3 (Shift == 0)") {
        set_flag_C(true);
        memory.regs[2] = 0xFFFF0000;
        memory.regs[3] = 0x12345678;
        execute(0b010000'0011'010'011);

        REQUIRE(memory.regs[3] == 0x12345678);
        check_flags_NZCV(false, false, true, true);
    }

    SECTION("LSR R2, R3 (Shift < 32)") {
        set_flag_C(true);
        memory.regs[2] = 0xFFFF0004;
        memory.regs[3] = 0x1F345678;
        execute(0b010000'0011'010'011);

        REQUIRE(memory.regs[3] == 0x01F34567);
        check_flags_NZCV(false, false, true, true);
    }

    SECTION("LSR R2, R3 (Shift == 32)") {
        set_flag_C(true);
        memory.regs[2] = 0xFFFF0020;
        memory.regs[3] = 0x1F345679;
        execute(0b010000'0011'010'011);

        REQUIRE(memory.regs[3] == 0x00000000);
        check_flags_NZCV(false, true, false, true);
    }

    SECTION("LSR R2, R3 (Shift > 32)") {
        set_flag_C(true);
        memory.regs[2] = 0xFFFF0021;
        memory.regs[3] = 0x1F345678;
        execute(0b010000'0011'010'011);

        REQUIRE(memory.regs[3] == 0x00000000);
        check_flags_NZCV(false, true, false, true);
    }
} 





TEST_CASE("CPU Thumb Mode - Logical ASR") {
    set_bit_T(true);

    set_flag_V(true);

    SECTION("ASR R2, R3 (Shift == 0)") {
        set_flag_C(true);
        memory.regs[2] = 0xFFFF0000;
        memory.regs[3] = 0x12345678;
        execute(0b010000'0100'010'011);

        REQUIRE(memory.regs[3] == 0x12345678);
        check_flags_NZCV(false, false, true, true);
    }

    SECTION("ASR R2, R3 (Shift < 32)") {
        set_flag_C(true);
        memory.regs[2] = 0xFFFF0004;
        memory.regs[3] = 0x9F345678;
        execute(0b010000'0100'010'011);

        REQUIRE(memory.regs[3] == 0xF9F34567);
        check_flags_NZCV(true, false, true, true);
    }

    SECTION("ASR R2, R3 (Shift >= 32, positive)") {
        set_flag_C(true);
        memory.regs[2] = 0xFFFF0020;
        memory.regs[3] = 0x1F345679;
        execute(0b010000'0100'010'011);

        REQUIRE(memory.regs[3] == 0x00000000);
        check_flags_NZCV(false, true, false, true);
    }

    SECTION("ASR R2, R3 (Shift >= 32, negative)") {
        set_flag_C(true);
        memory.regs[2] = 0xFFFF0020;
        memory.regs[3] = 0x9F345678;
        execute(0b010000'0100'010'011);

        REQUIRE(memory.regs[3] == 0xFFFFFFFF);
        check_flags_NZCV(true, false, true, true);
    }
}





TEST_CASE("CPU Thumb Mode - ADC") {
    set_bit_T(true);

    SECTION("ADC R2, R3 (Zero)") {
        set_flag_C(false);
        memory.regs[2] = 0x00000000;
        memory.regs[3] = 0x00000000;
        execute(0b010000'0101'010'011);

        REQUIRE(memory.regs[3] == 0x00000000);
        check_flags_NZCV(false, true, false, false);
    }

    SECTION("ADC R2, R3 (V flag)") {
        set_flag_C(true);
        memory.regs[2] = 0x7FFFFFFF;
        memory.regs[3] = 0x00000001;
        execute(0b010000'0101'010'011);

        REQUIRE(memory.regs[3] == 0x80000001);
        check_flags_NZCV(true, false, false, true);
    }

    SECTION("ADC R2, R3 (No Overflow)") {
        set_flag_C(false);
        memory.regs[2] = 0x00000001;
        memory.regs[3] = 0xFFFFFFFE;
        execute(0b010000'0101'010'011);

        REQUIRE(memory.regs[3] == 0xFFFFFFFF);
        check_flags_NZCV(true, false, false, false);
    }

    SECTION("ADC R2, R3 (Overflow)") {
        set_flag_C(true);
        memory.regs[2] = 0x00000001;
        memory.regs[3] = 0xFFFFFFFE;
        execute(0b010000'0101'010'011);
        
        REQUIRE(memory.regs[3] == 0x00000000);
        check_flags_NZCV(false, true, true, false);
    }
    wipe_registers();
}





TEST_CASE("CPU Thumb Mode - SBC") {
    set_bit_T(true);

    SECTION("SBC R2, R3 (Zero)") {
        set_flag_C(true);
        memory.regs[2] = 0x00000000;
        memory.regs[3] = 0x00000000;
        execute(0b010000'0110'010'011);

        REQUIRE(memory.regs[3] == 0x00000000);
        check_flags_NZCV(false, true, false, false);
    }

    SECTION("SBC R2, R3 (V flag)") {
        set_flag_C(false);
        memory.regs[2] = 0x80000000;
        memory.regs[3] = 0x80000001;
        execute(0b010000'0110'010'011);

        REQUIRE(memory.regs[3] == 0x00000000);
        check_flags_NZCV(false, true, true, true);
    }

    SECTION("SBC R2, R3 (No Overflow)") {
        set_flag_C(true);
        memory.regs[2] = 0x00000001;
        memory.regs[3] = 0xFFFFFFFE;
        execute(0b010000'0110'010'011);

        REQUIRE(memory.regs[3] == 0xFFFFFFFD);
        check_flags_NZCV(true, false, true, false);
    }

    SECTION("SBC R2, R3 (Overflow)") {
        set_flag_C(false);
        memory.regs[2] = 0xFFFFFFFF;
        memory.regs[3] = 0xFFFFFFFE;
        execute(0b010000'0110'010'011);
        
        REQUIRE(memory.regs[3] == 0xFFFFFFFE);
        check_flags_NZCV(true, false, false, false);
    }
    wipe_registers();
}





TEST_CASE("CPU Thumb Mode - ROR") {
    set_bit_T(true);

    set_flag_C(true);
    set_flag_V(true);

    SECTION("ROR R2, R3 (r3 == 0)") {
        memory.regs[2] = 0xFFFFFF00;
        memory.regs[3] = 0x01234567;
        execute(0b010000'0111'010'011);

        REQUIRE(memory.regs[3] == 0x01234567);
        check_flags_NZCV(false, false, true, true);
    }

    SECTION("ROR R2, R3 (r3[4:0] == 0 && r3 r3[7:0] != 0)") {
        memory.regs[2] = 0xFFFFFFF0;
        memory.regs[3] = 0x01234567;
        execute(0b010000'0111'010'011);
        REQUIRE(memory.regs[3] == 0x01234567);

        check_flags_NZCV(false, false, false, true);
    }

    SECTION("ROR R2, R3 (r3[4:0] > 0)") {
        memory.regs[2] = 0xFFFFFFF8;
        memory.regs[3] = 0x01234567;
        execute(0b010000'0111'010'011);

        REQUIRE(memory.regs[3] == 0x67012345);
        check_flags_NZCV(false, false, false, true);
    }
    wipe_registers();
}





TEST_CASE("CPU Thumb Mode - TST") {
    set_bit_T(true);

    set_flag_C(true);
    set_flag_V(true);

    SECTION("TST R2, R3 (alu_out != 0)") {
        memory.regs[2] = 0xFF00FF00;
        memory.regs[3] = 0x91234567;
        execute(0b010000'1000'010'011);

        REQUIRE(memory.regs[3] == 0x91234567);
        check_flags_NZCV(true, false, true, true);
    }

    SECTION("TST R2, R3 (alu_out == 0)") {
        memory.regs[2] = 0xFEDCBA98;
        memory.regs[3] = 0x01234567;
        execute(0b010000'1000'010'011);

        REQUIRE(memory.regs[3] == 0x01234567);
        check_flags_NZCV(false, true, true, true);
    }
}





TEST_CASE("CPU Thumb Mode - NEG") {
    set_bit_T(true);

    set_flag_C(true);
    set_flag_V(true);

    SECTION("TST R2, R3 (Zero)") {
        memory.regs[2] = 0x00000000;
        execute(0b010000'1001'010'011);

        REQUIRE(memory.regs[3] == 0x00000000);
        check_flags_NZCV(false, true, false, false);
    }

    SECTION("TST R2, R3 (Carry set)") {
        memory.regs[2] = 0b00000000'00000000'00000000'00011001;
        execute(0b010000'1001'010'011);

        REQUIRE(memory.regs[3] == 0b11111111'11111111'11111111'11100111);
        check_flags_NZCV(true, false, true, false);
    }

    SECTION("TST R2, R3 (Overflow set)") {
        memory.regs[2] = 0b10000000'00000000'00000000'00000000;
        execute(0b010000'1001'010'011);

        REQUIRE(memory.regs[3] == 0b10000000'00000000'00000000'00000000);
        check_flags_NZCV(true, false, true, true);
    }
}





TEST_CASE("CPU Thumb Mode - CMP Registers (Low)") {
    set_bit_T(true);

    SECTION("CMP R2, R3 (Zero)") {
        memory.regs[2] = 0x00000000;
        memory.regs[3] = 0x00000000;
        execute(0b010000'1010'010'011);

        check_flags_NZCV(false, true, false, false);
    }

    SECTION("CMP R2, R3 (V flag)") {
        memory.regs[2] = 0x80000000;
        memory.regs[3] = 0x80000000;
        execute(0b010000'1010'010'011);

        check_flags_NZCV(false, true, true, true);
    }

    SECTION("CMP R2, R3 (No Overflow)") {
        memory.regs[2] = 0x00000001;
        memory.regs[3] = 0xFFFFFFFE;
        execute(0b010000'1010'010'011);

        check_flags_NZCV(true, false, true, false);
    }

    SECTION("CMP R2, R3 (Overflow)") {
        memory.regs[2] = 0xFFFFFFFF;
        memory.regs[3] = 0xFFFFFFFD;
        execute(0b010000'1010'010'011);
        
        check_flags_NZCV(true, false, false, false);
    }
    wipe_registers();
}





TEST_CASE("CPU Thumb Mode - CMN") {
    set_bit_T(true);

    SECTION("CMN R2, R3 (Zero)") {
        memory.regs[2] = 0x00000000;
        memory.regs[3] = 0x00000000;
        execute(0b010000'1011'010'011);

        check_flags_NZCV(false, true, false, false);
    }

    SECTION("CMN R2, R3 (V flag)") {
        memory.regs[2] = 0x7FFFFFFF;
        memory.regs[3] = 0x00000002;
        execute(0b010000'1011'010'011);

        check_flags_NZCV(true, false, false, true);
    }

    SECTION("CMN R2, R3 (No Overflow)") {
        memory.regs[2] = 0x00000001;
        memory.regs[3] = 0xFFFFFFFE;
        execute(0b010000'1011'010'011);

        check_flags_NZCV(true, false, false, false);
    }

    SECTION("CMN R2, R3 (Overflow)") {
        memory.regs[2] = 0x00000001;
        memory.regs[3] = 0xFFFFFFFF;
        execute(0b010000'1011'010'011);
        
        check_flags_NZCV(false, true, true, false);
    }
    wipe_registers();
}





TEST_CASE("CPU Thumb Mode - ORR") {
    set_bit_T(true);

    SECTION("ORR R2, R3") {
        set_flag_C(false);
        set_flag_V(false);
        memory.regs[2] = 0xFF00FF00;
        memory.regs[3] = 0xFFF000FF;
        execute(0b010000'1100'010'011);

        REQUIRE(memory.regs[3] == 0xFFF0FFFF);
        check_flags_NZCV(true, false, false, false);
    }
}





TEST_CASE("CPU Thumb Mode - MUL") {
    set_bit_T(true);

    SECTION("MUL R2, R3") {
        set_flag_C(false);
        set_flag_V(false);
        memory.regs[2] = 0xFF00FF00;
        memory.regs[3] = 0x00000008;
        execute(0b010000'1101'010'011);

        REQUIRE(memory.regs[3] == 0xF807F800);
        check_flags_NZCV(true, false, false, false);
    }
}





TEST_CASE("CPU Thumb Mode - BIC") {
    set_bit_T(true);

    SECTION("BIC R2, R3") {
        set_flag_C(false);
        set_flag_V(false);
        memory.regs[2] = 0xFF00FF00;
        memory.regs[3] = 0x00FF0F00;
        execute(0b010000'1110'010'011);

        REQUIRE(memory.regs[3] == 0x00FF0000);
        check_flags_NZCV(false, false, false, false);
    }
}





TEST_CASE("CPU Thumb Mode - MVN") {
    set_bit_T(true);

    SECTION("MVN R2, R3") {
        set_flag_C(false);
        set_flag_V(false);
        memory.regs[2] = 0x01234567;
        execute(0b010000'1111'010'011);

        REQUIRE(memory.regs[3] == 0xFEDCBA98);
        check_flags_NZCV(true, false, false, false);
    }
}





TEST_CASE("CPU Thumb Mode - BX") {
    set_bit_T(true);

    SECTION("BX R14 (high register, no exchange)") {
        *memory.pc      = 0x00000000;
        memory.regs[14] = 0x01234567;
        execute(0b010001110'1'110'000);

        REQUIRE(*memory.pc == 0x01234566);
        REQUIRE(get_bit_T() == true);
    }

    SECTION("BX R2 (low register, no exchange)") {
        *memory.pc     = 0x00000000;
        memory.regs[2] = 0x01234567;
        execute(0b010001110'0'010'000);

        REQUIRE(*memory.pc  == 0x01234566);
        REQUIRE(get_bit_T() == true);
    }

    SECTION("BX R14 (exchange)") {
        *memory.pc     = 0x00000000;
        memory.regs[2] = 0x01234566;
        execute(0b010001110'0'010'000);

        REQUIRE(*memory.pc  == 0x01234566);
        REQUIRE(get_bit_T() == false);
    }
}





TEST_CASE("CPU Thumb Mode - MOV (no flag changes, high registers)") {
    set_bit_T(true);

    SECTION("MOV R2, R3 (low source, low destination)") {
        memory.regs[2] = 0x01234567;
        memory.regs[3] = 0x00000000;
        execute(0b01000110'0'0'010'011);

        REQUIRE(memory.regs[3] == 0x01234567);
    }

    SECTION("MOV R10, R3 (low source, high destination)") {
        memory.regs[10] = 0x01234567;
        memory.regs[3]  = 0x00000000;
        execute(0b01000110'0'1'010'011);

        REQUIRE(memory.regs[3] == 0x01234567);
    }

    SECTION("MOV R2, R11 (high source, low destination)") {
        memory.regs[2]  = 0x01234567;
        memory.regs[11] = 0x00000000;
        execute(0b01000110'1'0'010'011);

        REQUIRE(memory.regs[3] == 0x01234567);
    }

    SECTION("MOV R10, R11 (high source, high destination)") {
        memory.regs[10] = 0x01234567;
        memory.regs[11] = 0x00000000;
        execute(0b01000110'1'1'010'011);

        REQUIRE(memory.regs[3] == 0x01234567);
    }
}





TEST_CASE("CPU Thumb Mode - ADD (no flag changes, high registers)") {
    set_bit_T(true);

    SECTION("ADD R2, R3 (low source, low destination)") {
        memory.regs[2] = 0x01234567;
        memory.regs[3] = 0x00000001;
        execute(0b01000100'0'0'010'011);

        REQUIRE(memory.regs[3] == 0x01234568);
    }

    SECTION("ADD R10, R3 (low source, high destination)") {
        memory.regs[10] = 0x01234567;
        memory.regs[3]  = 0x00000001;
        execute(0b01000100'0'1'010'011);

        REQUIRE(memory.regs[3] == 0x01234568);
    }

    SECTION("ADD R2, R11 (high source, low destination)") {
        memory.regs[2]  = 0x01234567;
        memory.regs[11] = 0x00000001;
        execute(0b01000100'1'0'010'011);

        REQUIRE(memory.regs[3] == 0x01234568);
    }

    SECTION("ADD R10, R11 (high source, high destination)") {
        memory.regs[10] = 0x01234567;
        memory.regs[11] = 0x00000001;
        execute(0b01000100'1'1'010'011);

        REQUIRE(memory.regs[3] == 0x01234568);
    }
}





TEST_CASE("CPU Thumb Mode - CMP Registers (High)") {
    set_bit_T(true);

    SECTION("CMP R10, R11 (Zero)") {
        memory.regs[10] = 0x00000000;
        memory.regs[11] = 0x00000000;
        execute(0b01000101'1'1'010'011);

        check_flags_NZCV(false, true, false, false);
    }

    SECTION("CMP R10, R11 (V flag)") {
        memory.regs[10] = 0x80000000;
        memory.regs[11] = 0x80000000;
        execute(0b01000101'1'1'010'011);

        check_flags_NZCV(false, true, true, true);
    }

    SECTION("CMP R10, R11 (No Overflow)") {
        memory.regs[10] = 0x00000001;
        memory.regs[11] = 0xFFFFFFFE;
        execute(0b01000101'1'1'010'011);

        check_flags_NZCV(true, false, true, false);
    }

    SECTION("CMP R10, R11 (Overflow)") {
        memory.regs[10] = 0xFFFFFFFF;
        memory.regs[11] = 0xFFFFFFFD;
        execute(0b01000101'1'1'010'011);
        
        check_flags_NZCV(true, false, false, false);
    }
    wipe_registers();
}





TEST_CASE("CPU Thumb Mode - LDRSH") {
    set_bit_T(true);

    memory.regs[2] = 0x05000000;
    memory.main[0x05000000] = 0x42;
    memory.main[0x05000001] = 0x53;
    memory.main[0x05000002] = 0x99;

    SECTION("LDRSH R4, [R2, R3] (Zero offset)") {
        memory.regs[3] = 0x00000000;
        memory.regs[4] = 0x00000000;
        execute(0b0101111'010'011'100);

        REQUIRE(memory.regs[4] == 0x00005342);
    }

    SECTION("LDRSH R4, [R2, R3] (Non-Zero offset)") {
        memory.regs[3] = 0x00000001;
        memory.regs[4] = 0x00000000;
        execute(0b0101111'010'011'100);

        REQUIRE(memory.regs[4] == 0xFFFF9953);
    }
}





TEST_CASE("CPU Thumb Mode - LDRB (Relative Offset)") {
    set_bit_T(true);

    memory.regs[2] = 0x05000000;
    memory.main[0x05000000] = 0x42;
    memory.main[0x05000001] = 0x53;

    SECTION("LDRB R4, [R2, R3] (Zero offset)") {
        memory.regs[3] = 0x00000000;
        memory.regs[4] = 0x00000000;
        execute(0b0101110'010'011'100);

        REQUIRE(memory.regs[4] == 0x00000042);
    }

    SECTION("LDRB R4, [R2, R3] (Non-Zero offset)") {
        memory.regs[3] = 0x00000001;
        memory.regs[4] = 0x00000000;
        execute(0b0101110'010'011'100);

        REQUIRE(memory.regs[4] == 0x00000053);
    }
}





TEST_CASE("CPU Thumb Mode - LDRH (Relative Offset)") {
    set_bit_T(true);

    memory.regs[2] = 0x05000000;
    memory.main[0x05000000] = 0x42;
    memory.main[0x05000001] = 0x53;
    memory.main[0x05000002] = 0x99;


    SECTION("LDRH R4, [R2, R3] (Zero offset)") {
        memory.regs[3] = 0x00000000;
        memory.regs[4] = 0x00000000;
        execute(0b0101101'010'011'100);

        REQUIRE(memory.regs[4] == 0x00005342);
    }

    SECTION("LDRH R4, [R2, R3] (Non-Zero offset)") {
        memory.regs[3] = 0x00000001;
        memory.regs[4] = 0x00000000;
        execute(0b0101101'010'011'100);

        REQUIRE(memory.regs[4] == 0x00009953);
    }
}





TEST_CASE("CPU Thumb Mode - LDR (Relative Offset)") {
    set_bit_T(true);

    memory.regs[2] = 0x05000000;
    memory.main[0x05000000] = 0x42;
    memory.main[0x05000001] = 0x53;
    memory.main[0x05000002] = 0x99;
    memory.main[0x05000003] = 0x4E;
    memory.main[0x05000004] = 0xC5;
    memory.main[0x05000005] = 0xF5;


    SECTION("LDR R4, [R2, R3] (Zero offset)") {
        memory.regs[3] = 0x00000000;
        memory.regs[4] = 0x00000000;
        execute(0b0101100'010'011'100);

        REQUIRE(memory.regs[4] == 0x4E995342);
    }

    SECTION("LDR R4, [R2, R3] (Non-Zero offset)") {
        memory.regs[3] = 0x00000002;
        memory.regs[4] = 0x00000000;
        execute(0b0101100'010'011'100);

        REQUIRE(memory.regs[4] == 0xF5C54E99);
    }
}





TEST_CASE("CPU Thumb Mode - LDRSB (Relative Offset)") {
    set_bit_T(true);

    memory.regs[2] = 0x05000000;
    memory.main[0x05000000] = 0x42;
    memory.main[0x05000001] = 0x93;

    SECTION("LDRSB R4, [R2, R3] (Zero offset)") {
        memory.regs[3] = 0x00000000;
        memory.regs[4] = 0x00000000;
        execute(0b0101011'010'011'100);

        REQUIRE(memory.regs[4] == 0x00000042);
    }

    SECTION("LDRSB R4, [R2, R3] (Non-Zero offset)") {
        memory.regs[3] = 0x00000001;
        memory.regs[4] = 0x00000000;
        execute(0b0101011'010'011'100);

        REQUIRE(memory.regs[4] == 0xFFFFFF93);
    }
}





TEST_CASE("CPU Thumb Mode - LDRB (Immediate Offset)") {
    set_bit_T(true);

    memory.regs[3] = 0x05000000;
    memory.main[0x05000000] = 0x42;
    memory.main[0x05000004] = 0x93;

    SECTION("LDRB R2, [R3, #0x00000] (Zero offset)") {
        memory.regs[2] = 0x00000000;
        execute(0b01111'00000'011'010);

        REQUIRE(memory.regs[2] == 0x00000042);
    }

    SECTION("LDRB R2, [R3, #0x00001] (Non-Zero offset)") {
        memory.regs[2] = 0x00000000;
        execute(0b01111'00001'011'010);

        REQUIRE(memory.regs[2] == 0x00000093);
    }
}





TEST_CASE("CPU Thumb Mode - LDR (Immediate Offset)") {
    set_bit_T(true);

    memory.regs[3] = 0x05000000;
    memory.main[0x05000000] = 0x42;
    memory.main[0x05000001] = 0x53;
    memory.main[0x05000002] = 0x99;
    memory.main[0x05000003] = 0x4E;
    memory.main[0x05000004] = 0xC5;
    memory.main[0x05000005] = 0xF5;
    memory.main[0x05000006] = 0xAB;
    memory.main[0x05000007] = 0x31;


    SECTION("LDR R2, [R3, #0x00000] (Zero offset)") {
        memory.regs[2] = 0x00000000;
        execute(0b01101'00000'011'010);

        REQUIRE(memory.regs[2] == 0x4E995342);
    }

    SECTION("LDR R2, [R3, #0x00001] (Non-Zero offset)") {
        memory.regs[2] = 0x00000000;
        execute(0b01101'00001'011'010);

        REQUIRE(memory.regs[2] == 0x31ABF5C5);
    }
}





TEST_CASE("CPU Thumb Mode - STRB (Immediate Offset)") {
    set_bit_T(true);

    memory.regs[3] = 0x05000000;
    memory.main[0x05000000] = 0x00;
    memory.main[0x05000004] = 0x00;


    SECTION("STRB R2, [R3, #0x00000] (Zero offset)") {
        memory.regs[2] = 0xABCDEF42;
        execute(0b01110'00000'011'010);

        REQUIRE(memory.main[0x05000000] == 0x42);
    }

    SECTION("STRB R2, [R3, #0x00001] (Non-Zero offset)") {
        memory.regs[2] = 0xABCDEFC5;
        execute(0b01110'00001'011'010);

        REQUIRE(memory.main[0x05000004] == 0xC5);
    }
}





TEST_CASE("CPU Thumb Mode - STR (Immediate Offset)") {
    set_bit_T(true);

    memory.regs[3] = 0x05000000;
    memory.main[0x05000000] = 0x00;
    memory.main[0x05000001] = 0x00;
    memory.main[0x05000002] = 0x00;
    memory.main[0x05000003] = 0x00;
    memory.main[0x05000004] = 0x00;
    memory.main[0x05000005] = 0x00;
    memory.main[0x05000006] = 0x00;
    memory.main[0x05000007] = 0x00;


    SECTION("STR R2, [R3, #0x00000] (Zero offset)") {
        memory.regs[2] = 0x83AD4342;
        execute(0b01100'00000'011'010);

        REQUIRE(memory.main[0x05000000] == 0x42);
        REQUIRE(memory.main[0x05000001] == 0x43);
        REQUIRE(memory.main[0x05000002] == 0xAD);
        REQUIRE(memory.main[0x05000003] == 0x83);
    }

    SECTION("STR R2, [R3, #0x00001] (Non-Zero offset)") {
        memory.regs[2] = 0x09F5C54E;
        execute(0b01100'00001'011'010);

        REQUIRE(memory.main[0x05000004] == 0x4E);
        REQUIRE(memory.main[0x05000005] == 0xC5);
        REQUIRE(memory.main[0x05000006] == 0xF5);
        REQUIRE(memory.main[0x05000007] == 0x09);
    }
}





TEST_CASE("CPU Thumb Mode - LDR (Stack Pointer + Immediate Offset)") {
    set_bit_T(true);

    *memory.sp = 0x05000000;
    memory.main[0x05000000] = 0x42;
    memory.main[0x05000001] = 0x53;
    memory.main[0x05000002] = 0x99;
    memory.main[0x05000003] = 0x4E;
    memory.main[0x05000004] = 0xC5;
    memory.main[0x05000005] = 0xF5;
    memory.main[0x05000006] = 0xAB;
    memory.main[0x05000007] = 0x31;

    SECTION("LDR R2, [SP, #0x00000] (Zero offset)") {
        memory.regs[2] = 0x00000000;
        execute(0b10011'010'00000000);

        REQUIRE(memory.regs[2] == 0x4E995342);
    }

    SECTION("LDR R2, [SP, #0x00001] (Non-Zero offset)") {
        memory.regs[2] = 0x00000000;
        execute(0b10011'010'00000001);

        REQUIRE(memory.regs[2] == 0x31ABF5C5);
    }
}





TEST_CASE("CPU Thumb Mode - STR (Stack Pointer + Immediate Offset)") {
    set_bit_T(true);

    *memory.sp = 0x05000000;

    SECTION("LDR R2, [SP, #0x00000] (Zero offset)") {
        memory.regs[2] = 0x4E995342;
        execute(0b10010'010'00000000);

        REQUIRE(memory.main[0x05000000] == 0x42);
        REQUIRE(memory.main[0x05000001] == 0x53);
        REQUIRE(memory.main[0x05000002] == 0x99);
        REQUIRE(memory.main[0x05000003] == 0x4E);
    }

    SECTION("LDR R2, [SP, #0x00001] (Non-Zero offset)") {
        memory.regs[2] = 0x31ABF5C5;
        execute(0b10010'010'00000001);

        REQUIRE(memory.main[0x05000004] == 0xC5);
        REQUIRE(memory.main[0x05000005] == 0xF5);
        REQUIRE(memory.main[0x05000006] == 0xAB);
        REQUIRE(memory.main[0x05000007] == 0x31);
    }
}





TEST_CASE("CPU Thumb Mode - PUSH") {
    set_bit_T(true);

    memory.regs[0] = 0xA03B4523;
    memory.regs[1] = 0x928847FF;
    memory.regs[2] = 0xC38297DE;
    memory.regs[3] = 0x4EC5F582;
    memory.regs[4] = 0x883729B4;
    memory.regs[5] = 0xA98DC823;
    memory.regs[6] = 0x038D87FF;
    memory.regs[7] = 0x000F383D;
    *memory.lr     = 0x8289CCC3;

    SECTION("PUSH {R0, R1, R2, R4, R5, R7} (Without linkage register)") {
        *memory.sp = 0x05000018;
        execute(0b1011010'0'10110111);

        REQUIRE(memory.main[0x05000000] == 0x23); // register 0
        REQUIRE(memory.main[0x05000001] == 0x45);
        REQUIRE(memory.main[0x05000002] == 0x3B);
        REQUIRE(memory.main[0x05000003] == 0xA0); 
        REQUIRE(memory.main[0x05000004] == 0xFF); // register 1
        REQUIRE(memory.main[0x05000005] == 0x47);
        REQUIRE(memory.main[0x05000006] == 0x88);
        REQUIRE(memory.main[0x05000007] == 0x92);
        REQUIRE(memory.main[0x05000008] == 0xDE); // register 2
        REQUIRE(memory.main[0x05000009] == 0x97);
        REQUIRE(memory.main[0x0500000A] == 0x82);
        REQUIRE(memory.main[0x0500000B] == 0xC3);
        REQUIRE(memory.main[0x0500000C] == 0xB4); // register 4
        REQUIRE(memory.main[0x0500000D] == 0x29);
        REQUIRE(memory.main[0x0500000E] == 0x37);
        REQUIRE(memory.main[0x0500000F] == 0x88);
        REQUIRE(memory.main[0x05000010] == 0x23); // register 5
        REQUIRE(memory.main[0x05000011] == 0xC8);
        REQUIRE(memory.main[0x05000012] == 0x8D);
        REQUIRE(memory.main[0x05000013] == 0xA9);
        REQUIRE(memory.main[0x05000014] == 0x3D); // register 7
        REQUIRE(memory.main[0x05000015] == 0x38);
        REQUIRE(memory.main[0x05000016] == 0x0F);
        REQUIRE(memory.main[0x05000017] == 0x00);
        REQUIRE(*memory.sp              == 0x05000000);
    }

    SECTION("PUSH {R3, LR} (With linkage register)") {
        *memory.sp = 0x05000008;
        execute(0b1011010'1'00001000);

        REQUIRE(memory.main[0x05000000] == 0x82);
        REQUIRE(memory.main[0x05000001] == 0xF5);
        REQUIRE(memory.main[0x05000002] == 0xC5);
        REQUIRE(memory.main[0x05000003] == 0x4E);
        REQUIRE(memory.main[0x05000004] == 0xC3);
        REQUIRE(memory.main[0x05000005] == 0xCC);
        REQUIRE(memory.main[0x05000006] == 0x89);
        REQUIRE(memory.main[0x05000007] == 0x82);
    }
}





TEST_CASE("CPU Thumb Mode - POP") {
    set_bit_T(true);

    wipe_registers();

    SECTION("POP {R0, R1, R2, R4, R5, R7} (Without linkage register)") { 
        memory.main[0x05000000] = 0x23;  // register 0
        memory.main[0x05000001] = 0x45; 
        memory.main[0x05000002] = 0x3B; 
        memory.main[0x05000003] = 0xA0; 
        memory.main[0x05000004] = 0xFF;  // register 1
        memory.main[0x05000005] = 0x47; 
        memory.main[0x05000006] = 0x88; 
        memory.main[0x05000007] = 0x92; 
        memory.main[0x05000008] = 0xDE;  // register 2
        memory.main[0x05000009] = 0x97; 
        memory.main[0x0500000A] = 0x82; 
        memory.main[0x0500000B] = 0xC3;
        memory.main[0x0500000C] = 0xB4;  // register 4
        memory.main[0x0500000D] = 0x29; 
        memory.main[0x0500000E] = 0x37; 
        memory.main[0x0500000F] = 0x88; 
        memory.main[0x05000010] = 0x23;  // register 5
        memory.main[0x05000011] = 0xC8; 
        memory.main[0x05000012] = 0x8D; 
        memory.main[0x05000013] = 0xA9; 
        memory.main[0x05000014] = 0x3D;  // register 7
        memory.main[0x05000015] = 0x38; 
        memory.main[0x05000016] = 0x0F; 
        memory.main[0x05000017] = 0x00;

        *memory.sp = 0x05000000;
        execute(0b1011110'0'10110111);

        REQUIRE(memory.regs[0] == 0xA03B4523);
        REQUIRE(memory.regs[1] == 0x928847FF);
        REQUIRE(memory.regs[2] == 0xC38297DE);
        REQUIRE(memory.regs[4] == 0x883729B4);
        REQUIRE(memory.regs[5] == 0xA98DC823);
        REQUIRE(memory.regs[7] == 0x000F383D);
        REQUIRE(*memory.sp     == 0x05000018);
    }

    SECTION("PUSH {R3, LR} (With linkage register)") {
        memory.main[0x05000000] = 0x82;
        memory.main[0x05000001] = 0xF5;
        memory.main[0x05000002] = 0xC5;
        memory.main[0x05000003] = 0x4E;
        memory.main[0x05000004] = 0xC3;
        memory.main[0x05000005] = 0xCC;
        memory.main[0x05000006] = 0x89;
        memory.main[0x05000007] = 0x82;

        *memory.sp = 0x05000000;
        execute(0b1011110'1'00001000);
        REQUIRE(memory.regs[3] == 0x4EC5F582);
        REQUIRE(*memory.pc     == 0x8289CCC3);
        REQUIRE(*memory.sp     == 0x05000008);
    }
}





TEST_CASE("CPU Thumb Mode - Stack Pointer Arithmetic") {
    set_bit_T(true);

    SECTION("ADD SP, #0 * 4") {
        *memory.sp = 0x05000000;
        execute(0b10110000'0'0000000);

        REQUIRE(*memory.sp == 0x05000000);
    }

    SECTION("ADD SP, #5 * 4") {
        *memory.sp = 0x05000000;
        execute(0b10110000'0'0000101);
        
        REQUIRE(*memory.sp == 0x05000014);
    }

    SECTION("SUB SP, #0 * 4") {
        *memory.sp = 0x05000000;
        execute(0b10110000'1'0000000);
        
        REQUIRE(*memory.sp == 0x05000000);
    }

    SECTION("SUB SP, #5 * 4") {
        *memory.sp = 0x05000014;
        execute(0b10110000'1'0000101);
        
        REQUIRE(*memory.sp == 0x05000000);
    }
}





TEST_CASE("CPU Thumb Mode - Load halfword") {
    set_bit_T(true);

    memory.main[0x08000000] = 0x4E;
    memory.main[0x08000001] = 0xC5;
    memory.main[0x08000002] = 0xF5;
    memory.main[0x08000003] = 0x00;

    SECTION("LDRH R2, [R3, R4] (Zero offset)") {
        memory.regs[2]          = 0x00000000;
        memory.regs[3]          = 0x08000000;
        execute(0b10001'00000'011'010);

        REQUIRE(memory.regs[2] == 0x0000C54E);
    }

    SECTION("LDRH R2, [R3, R4] (Non-Zero offset)") {
        memory.regs[2]          = 0x00000000;
        memory.regs[3]          = 0x08000000;
        execute(0b10001'00001'011'010);

        REQUIRE(memory.regs[2] == 0x000000F5);
    }
}





TEST_CASE("CPU Thumb Mode - Store halfword") {
    set_bit_T(true);

    SECTION("STRH R2, [R3, R4] (Zero offset)") {
        memory.main[0x08000000] = 0x00;
        memory.main[0x08000001] = 0x00;
        memory.main[0x08000002] = 0x00;
        memory.main[0x08000003] = 0x00;
        memory.regs[2]          = 0x0000C54E;
        memory.regs[3]          = 0x08000000;
        execute(0b10000'00000'011'010);

        REQUIRE(memory.main[0x08000000] == 0x4E);
        REQUIRE(memory.main[0x08000001] == 0xC5);
    }

    SECTION("STRH R2, [R3, R4] (Non-Zero offset)") {
        memory.main[0x08000000] = 0x00;
        memory.main[0x08000001] = 0x00;
        memory.main[0x08000002] = 0x00;
        memory.main[0x08000003] = 0x00;
        memory.regs[2]          = 0x000000F5;
        memory.regs[3]          = 0x08000000;
        execute(0b10000'00001'011'010);

        REQUIRE(memory.main[0x08000002] == 0xF5);
        REQUIRE(memory.main[0x08000003] == 0x00);
    }
}





TEST_CASE("CPU Thumb Mode - Load Address (Add #6)") {
    set_bit_T(true);

    SECTION("ADD R2, SP, #0x0 * 4 (Zero offset)") {
        *memory.sp     = 0x05000000;
        memory.regs[2] = 0x00000000;
        execute(0b10101'010'00000000);

        REQUIRE(memory.regs[2] == 0x05000000);
    }

    SECTION("ADD R2, SP, #0x5 * 4 (Non-Zero offset)") {
        *memory.sp     = 0x05000000;
        memory.regs[2] = 0x0;
        execute(0b10101'010'00000101);

        REQUIRE(memory.regs[2] == 0x05000014);
    }
}





TEST_CASE("CPU Thumb Mode - Arithmetic Shift Right (Immediate)") {
    set_bit_T(true);

    set_flag_V(false);

    SECTION("ASR R2, R3, #0x00000000 (Shift == 0 && Rm[31] == 0)") {
        memory.regs[2] = 0xFFFFFFFF;
        memory.regs[3] = 0x00000000;
        execute(0b00010'00000'011'010);

        REQUIRE(memory.regs[2] == 0x00000000);
        check_flags_NZCV(false, true, false, false);
    }

    SECTION("ASR R2, R3, #0x00000000 (Shift == 0 && Rm[31] != 0)") {
        memory.regs[2] = 0x00000000;
        memory.regs[3] = 0x80000000;
        execute(0b00010'00000'011'010);

        REQUIRE(memory.regs[2] == 0xFFFFFFFF);
        check_flags_NZCV(true, false, true, false);
    }

    SECTION("ASR R2, R3, #0x00000000 (Shift > 0 && rm > 0)") {
        memory.regs[2] = 0x00000000;
        memory.regs[3] = 0x01234567;
        execute(0b00010'00100'011'010);

        REQUIRE(memory.regs[2] == 0x00123456);
        check_flags_NZCV(false, false, false, false);
    }

    SECTION("ASR R2, R3, #0x00000000 (Shift > 0 && rm < 0)") {
        memory.regs[2] = 0x00000000;
        memory.regs[3] = 0x81234568;
        execute(0b00010'00100'011'010);

        REQUIRE(memory.regs[2] == 0xF8123456);
        check_flags_NZCV(true, false, true, false);
    }
}





TEST_CASE("CPU Thumb Mode - LDMIA") {
    set_bit_T(true);

    memory.main[0x08000000] = 0x00;
    memory.main[0x08000001] = 0x4E;
    memory.main[0x08000002] = 0xC5;
    memory.main[0x08000003] = 0xF5;
    memory.main[0x08000004] = 0xAB;
    memory.main[0x08000005] = 0xCD;
    memory.main[0x08000006] = 0xEF;
    memory.main[0x08000007] = 0x01;
    memory.main[0x08000008] = 0x83;
    memory.main[0x08000009] = 0x92;
    memory.main[0x0800000A] = 0xC8;
    memory.main[0x0800000B] = 0xD1;

    SECTION("LDMIA R6!, {R0, R2, R7} (Rn not included in register_list)") {
        memory.regs[0] = 0x00000000;
        memory.regs[2] = 0x00000000;
        memory.regs[7] = 0x00000000;
        memory.regs[6] = 0x08000000; // start register
        execute(0b11001'110'10000101);

        REQUIRE(memory.regs[0] == 0xF5C54E00);
        REQUIRE(memory.regs[2] == 0x01EFCDAB);
        REQUIRE(memory.regs[7] == 0xD1C89283);
        REQUIRE(memory.regs[6] == 0x0800000C); // start register must have updated
    }

    SECTION("LDMIA R6!, {R0, R2, R6} (Rn included in register_list)") {
        memory.regs[0] = 0x00000000;
        memory.regs[2] = 0x00000000;
        memory.regs[6] = 0x08000000; // start register
        execute(0b11001'110'01000101);

        REQUIRE(memory.regs[0] == 0xF5C54E00);
        REQUIRE(memory.regs[2] == 0x01EFCDAB);
        REQUIRE(memory.regs[6] == 0xD1C89283);
    }
}





TEST_CASE("CPU Thumb Mode - STMIA") {
    set_bit_T(true);

    memory.regs[0] = 0xF5C54E00;
    memory.regs[2] = 0x01EFCDAB;
    memory.regs[7] = 0xD1C89283;

    SECTION("STMIA R6!, {R0, R2, R7} (Rn not included in register_list)") {
        memory.main[0x08000000] = 0x00;
        memory.main[0x08000001] = 0x00;
        memory.main[0x08000002] = 0x00;
        memory.main[0x08000003] = 0x00;
        memory.main[0x08000004] = 0x00;
        memory.main[0x08000005] = 0x00;
        memory.main[0x08000006] = 0x00;
        memory.main[0x08000007] = 0x00;
        memory.main[0x08000008] = 0x00;
        memory.main[0x08000009] = 0x00;
        memory.main[0x0800000A] = 0x00;
        memory.main[0x0800000B] = 0x00;
        memory.regs[6] = 0x08000000;
        execute(0b11000'110'10000101);

        REQUIRE(memory.main[0x08000000] == 0x00);
        REQUIRE(memory.main[0x08000001] == 0x4E);
        REQUIRE(memory.main[0x08000002] == 0xC5);
        REQUIRE(memory.main[0x08000003] == 0xF5);
        REQUIRE(memory.main[0x08000004] == 0xAB);
        REQUIRE(memory.main[0x08000005] == 0xCD);
        REQUIRE(memory.main[0x08000006] == 0xEF);
        REQUIRE(memory.main[0x08000007] == 0x01);
        REQUIRE(memory.main[0x08000008] == 0x83);
        REQUIRE(memory.main[0x08000009] == 0x92);
        REQUIRE(memory.main[0x0800000A] == 0xC8);
        REQUIRE(memory.main[0x0800000B] == 0xD1);
    }

    SECTION("STMIA R6!, {R0, R2, R6} (Rn included in register_list)") {
        memory.main[0x08000000] = 0x00;
        memory.main[0x08000001] = 0x00;
        memory.main[0x08000002] = 0x00;
        memory.main[0x08000003] = 0x00;
        memory.main[0x08000004] = 0x00;
        memory.main[0x08000005] = 0x00;
        memory.main[0x08000006] = 0x00;
        memory.main[0x08000007] = 0x00;
        memory.main[0x08000008] = 0x00;
        memory.main[0x08000009] = 0x00;
        memory.main[0x0800000A] = 0x00;
        memory.main[0x0800000B] = 0x00;
        memory.regs[6] = 0x08000000;
        execute(0b11000'110'01000101);

        REQUIRE(memory.main[0x08000000] == 0x00);
        REQUIRE(memory.main[0x08000001] == 0x4E);
        REQUIRE(memory.main[0x08000002] == 0xC5);
        REQUIRE(memory.main[0x08000003] == 0xF5);
        REQUIRE(memory.main[0x08000004] == 0xAB);
        REQUIRE(memory.main[0x08000005] == 0xCD);
        REQUIRE(memory.main[0x08000006] == 0xEF);
        REQUIRE(memory.main[0x08000007] == 0x01);
        REQUIRE(memory.main[0x08000008] == 0x00);
        REQUIRE(memory.main[0x08000009] == 0x00);
        REQUIRE(memory.main[0x0800000A] == 0x00);
        REQUIRE(memory.main[0x0800000B] == 0x08);
    }
}





TEST_CASE("CPU Thumb Mode - CMP (Immediate)") {
    set_bit_T(true);

    SECTION("CMP R2, #0x00 (Zero)") {
        memory.regs[2] = 0x00000000;
        execute(0b00101'010'00000000);

        check_flags_NZCV(false, true, false, false);
    }

    SECTION("CMP R2, #0x80 (V flag)") {
        memory.regs[2] = 0xFFFFFFFF;
        execute(0b00101'010'10000000);

        check_flags_NZCV(true, false, true, false);
    }

    SECTION("CMP R2, #0x01 (No Overflow)") {
        memory.regs[2] = 0xFFFFFFFE;
        execute(0b00101'010'00000001);

        check_flags_NZCV(true, false, true, false);
    }

    SECTION("CMP R2, #0x01 (Overflow)") {
        memory.regs[2] = 0xFFFFFFFF;
        execute(0b00101'010'00000001);
        
        check_flags_NZCV(true, false, true, false);
    }
    wipe_registers();
}










// TODO: Move the below functions to a different file.

#define REQUIRE_MESSAGE(cond, msg) do { INFO(msg); REQUIRE(cond); } while((void)0, 0)

void check_cpu_state(CpuState expected, CpuState actual, std::string error_message) {
    REQUIRE_MESSAGE(expected.type   == actual.type,   error_message);
    REQUIRE_MESSAGE(expected.opcode == actual.opcode, error_message);
    
    for (int i = 0; i < 16; i++) {
        REQUIRE_MESSAGE(expected.regs[i] == actual.regs[i], error_message + ", " + std::to_string(i));
    }
}

TEST_CASE("CPU THUMB Mode - VBA Logs (thumb-alu_200000.log)") {
    set_bit_T(true);

    uint32_t num_instructions = 200000;
    CpuState* expected_output = produce_expected_cpu_states("tests/asm/logs/thumb-alu_200000.log", num_instructions);
    
    get_rom_as_bytes("tests/asm/bin/thumb-alu.gba", memory.rom_1, SIZE_ROM_1);

    for (int i = 0; i < 200000 - 1; i++) {
        if (expected_output[i].type == THUMB) {
            set_bit_T(true);
            set_cpu_state(expected_output[i]);
            execute(fetch());
            check_cpu_state(expected_output[i + 1], get_cpu_state(), "Failed at " + std::to_string(i));
        }
    }
}