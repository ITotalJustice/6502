#include "../../6502.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define UNUSED(a) (void)a

#define CLC 0x18
#define SEC 0x38
#define CLD 0xd8 // ok
#define SED 0xf8 // ok
#define LDA_IMM 0xa9 // ok
#define ADC_IMM 0x69 // ok
#define SBC_IMM 0xe9 // ok
#define STA_ABS 0x8d // ok

static struct MOS6502 cpu;
unsigned char finished = 0;
unsigned char ram[0x10000];
unsigned char *entry = NULL;

unsigned char *table = NULL;

unsigned char MOS6502_read(void* user, unsigned short addr) {
    return ram[addr];
}

void MOS6502_write(void* user, unsigned short addr, unsigned char value) {
    ram[addr] = value;
    if (addr = 0x1234)
        finished = 1;
}

char *opstr(unsigned char ch)
{
    switch (ch) {
    case CLC: return "CLC";
    case SEC: return "SEC";
    case CLD: return "CLD";
    case SED: return "SED";
    case ADC_IMM: return "ADC";
    case SBC_IMM: return "SBC";
    }
    return "";
}

char *flags(unsigned char status)
{
    static char buf[9];
    char c = '-';
    char z = '-';
    char i = 'x';
    char d = '-';
    char v = '-';
    char n = '-';

    if (status & 1) c = 'C';
    if (status & 2) z = 'Z';
    if (status & 4) i = 'I';
    if (status & 8) d = 'D';
    if (status & 64) v = 'V';
    if (status & 128) n = 'N';

    sprintf(buf, "%c%cxx%c%c%c%c", n, v, d, i, z, c);
    return buf;
}

void run_test(unsigned char op, unsigned char a, unsigned char b, unsigned char secclc, unsigned char sedcld)
{
    int carry, sum;
    unsigned char *m;
    struct MOS6502 cpu = {0};

    cpu.PC = 0x200;

    m = &ram[0x200];

    *m++ = sedcld; // SED/CLD
    *m++ = LDA_IMM;
    *m++ = a;
    *m++ = secclc; // SEC/CLC
    *m++ = op; // ADC_IMM/SBC_IMM
    *m++ = b;
    *m++ = STA_ABS;
    *m++ = 0x34;
    *m++ = 0x12;

    finished = 0;
    while (!finished) {
        MOS6502_run(&cpu);
    }

    sum = ram[0x1234];

#define FMASK 0xcb

    if (sum != entry[0] || (cpu.status.P & FMASK) != (entry[1] & FMASK)) {
        fprintf(stderr, "failed test %s; LDA #$%02x; %s; %s #$%02x\n", opstr(sedcld), a, opstr(secclc), opstr(op), b);
        fprintf(stderr, "got      sum: $%02x, flags: %s\n", sum, flags(cpu.status.P & FMASK));
        fprintf(stderr, "expected sum: $%02x, flags: %s\n", entry[0], flags(entry[1] & FMASK));
        exit(1);
    }
    entry += 2;
}

#define TABLE_SIZE (256 * 256 * 2 * 2 * 2 * 2)
int main(int argc, char **argv) {
    int a, b;
    FILE *f = NULL;

    UNUSED(argc); UNUSED(argv);

    table = malloc(TABLE_SIZE);
    if (!table) {
        fputs("failed to allocate memory for arithmetic table", stderr);
        goto error;
    }

    f = fopen("arithmetic.bin", "rb");
    if (f == NULL) {
        fputs("failed to open arithmetic table\n", stderr);
        goto error;
    }

    if (fread(table, 1, TABLE_SIZE, f) != TABLE_SIZE) {
        fputs("failed to read arithmetic table\n", stderr);
        goto error;
    }

    fclose(f);

    entry = table;


    // 00000-0FFFF adc clc cld
    for (a = 0; a < 256; a++) for (b = 0; b < 256; b++) run_test(ADC_IMM, a, b, CLC, CLD);

    // 10000-1FFFF adc sec cld
    for (a = 0; a < 256; a++) for (b = 0; b < 256; b++) run_test(ADC_IMM, a, b, SEC, CLD);

    // 20000-2FFFF sbc clc cld
    for (a = 0; a < 256; a++) for (b = 0; b < 256; b++) run_test(SBC_IMM, a, b, CLC, CLD);

    // 30000-3FFFF sbc sec cld
    for (a = 0; a < 256; a++) for (b = 0; b < 256; b++) run_test(SBC_IMM, a, b, SEC, CLD);



    // 00000-0FFFF adc clc sed
    for (a = 0; a < 256; a++) for (b = 0; b < 256; b++) run_test(ADC_IMM, a, b, CLC, SED);

    // 10000-1FFFF adc sec sed
    for (a = 0; a < 256; a++) for (b = 0; b < 256; b++) run_test(ADC_IMM, a, b, SEC, SED);

    // 20000-2FFFF sbc clc sed
    for (a = 0; a < 256; a++) for (b = 0; b < 256; b++) run_test(SBC_IMM, a, b, CLC, SED);

    // 30000-3FFFF sbc sec sed
    for (a = 0; a < 256; a++) for (b = 0; b < 256; b++) run_test(SBC_IMM, a, b, SEC, SED);

    fprintf(stderr, "all arithmetic tests passed!\n");
    return 0;

error:
    free(table);
    if (f) fclose(f);
    return 1;
}
