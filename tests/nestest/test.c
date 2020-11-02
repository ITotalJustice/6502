/* gcc test.c ../../6502.c -o test -std=c89 -DMOS6502_DEBUG -Wall -Wextra -Werror */

#include "../../6502.h"
#include "../../tools/logs/nestest_log.h"
#include "../../tools/nes_rom2h/nestest_rom.h"

#include <stdio.h>
#include <assert.h>

#define UNUSED(a) (void)a

static unsigned char old_opcode = 0;
static unsigned int cycles = 0;
static unsigned int cpu_cycles = 7; /* log starts at 7 cycles */

struct DummyNes {
    struct MOS6502 cpu;
    unsigned char ram[0x800];
    const unsigned char* rom;
};

unsigned char MOS6502_read(void* user, unsigned short addr) {
    switch ((addr >> 12) & 0xF) {
        case 0x0: case 0x1:
            return ((struct DummyNes*)user)->ram[addr & 0x7FF];
        case 0x8: case 0x9: case 0xA: case 0xB:
        case 0xC: case 0xD: case 0xE: case 0xF:
            return ((struct DummyNes*)user)->rom[addr & 0x3FFF];
        default:
            return 0xFF;
    }
}

void MOS6502_write(void* user, unsigned short addr, unsigned char value) {
    switch ((addr >> 12) & 0xF) {
        case 0x0: case 0x1:
            ((struct DummyNes*)user)->ram[addr & 0x7FF] = value;
            break;
    }
}

void MOS6502_debug_post_fetch(struct MOS6502* cpu, void* user, unsigned char opcode) {
    UNUSED(user); UNUSED(opcode);

    #define REG_PC cpu->PC
    #define REG_A cpu->A
    #define REG_X cpu->X
    #define REG_Y cpu->Y
    #define REG_SP cpu->S
#ifdef C89
    #define REG_P cpu->P
#else
    #define REG_P cpu->status.P
#endif

    #define OOF() \
        fprintf(stderr, "OLD_OP: 0x%02X\n", old_opcode); \
        fprintf(stderr, "GOT: A: 0x%02X X: 0x%02X Y: 0x%02X P: 0x%02X SP: 0x%02X PC: 0x%04X CYC: %u\n", \
            REG_A, REG_X, REG_Y, REG_P, REG_SP, REG_PC, cpu_cycles); \
        fprintf(stderr, "WANTED: A: 0x%02X X: 0x%02X Y: 0x%02X P: 0x%02X SP: 0x%02X PC: 0x%04X CYC: %u\n", \
            ARR_A[cycles], ARR_X[cycles], ARR_Y[cycles], ARR_P[cycles], ARR_SP[cycles], ARR_PC[cycles], ARR_CYC[cycles]); \
        assert(REG_A == ARR_A[cycles]); \
        assert(REG_X == ARR_X[cycles]); \
        assert(REG_Y == ARR_Y[cycles]); \
        assert(REG_P == ARR_P[cycles]); \
        assert(REG_SP == ARR_SP[cycles]); \
        assert(REG_PC == ARR_PC[cycles]); \
        assert(cycles == ARR_CYC[cycles]);

    if (REG_A != ARR_A[cycles]) { OOF(); }
    if (REG_X != ARR_X[cycles]) { OOF(); }
    if (REG_Y != ARR_Y[cycles]) { OOF(); }
    if (REG_P != ARR_P[cycles]) { OOF(); }
    if (REG_SP != ARR_SP[cycles]) { OOF(); }
    if (REG_PC != ARR_PC[cycles]) { OOF(); }
    if (cpu_cycles != ARR_CYC[cycles]) { OOF(); }
}

void MOS6502_debug_post_execute(struct MOS6502* cpu, void* user, unsigned char opcode) {
    UNUSED(user);
    cpu_cycles += cpu->cycles;
    old_opcode = opcode;
}

int main(int argc, char **argv) {
    struct DummyNes nes = {0};

    UNUSED(argc); UNUSED(argv);
    nes.cpu.userdata = &nes;
    nes.rom = NESTEST_ROM;

    nes.cpu.S = 0xFD;
    nes.cpu.PC = 0xC000;
#ifdef C89
    nes.cpu.P |= 0x24;
#else
    nes.cpu.status.P |= 0x24;
#endif

    for (cycles = 0; cycles < 8991; cycles++) {
        MOS6502_run(&nes.cpu);
    }

    puts("nestest passed!\n");

    return 0;
}
