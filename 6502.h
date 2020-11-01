#ifndef MOS6502_H
#define MOS6502_H

struct MOS6502 {
    void* userdata;
    unsigned short cycles;
    unsigned short PC; /* program counter */
    unsigned char A; /* accumulator */
    unsigned char X; /* index X */
    unsigned char Y; /* index Y */
    unsigned char S; /* stack pointer */
    unsigned char P;
};

void MOS6502_run(struct MOS6502* mos6502);

/* needs to be defined */
unsigned char MOS6502_read(void* user, unsigned short addr);
void MOS6502_write(void* user, unsigned short addr, unsigned char value);

#ifdef MOS6502_DEBUG
void MOS6502_debug_post_fetch(struct MOS6502*, void* user, unsigned char opcode);
void MOS6502_debug_post_execute(struct MOS6502*, void* user, unsigned char opcode);
#endif

#endif /* MOS6502_H */
