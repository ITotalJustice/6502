/* TotalJustice */

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
    union {
        struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            unsigned char C : 1; /* carry */
            unsigned char Z : 1; /* zero */
            unsigned char I : 1; /* interrupt */
            unsigned char D : 1; /* decimal */
            unsigned char B : 2; /* break */
            unsigned char V : 1; /* overflow */
            unsigned char N : 1; /* negative */
#else
            unsigned char N : 1; /* negative */
            unsigned char V : 1; /* overflow */
            unsigned char B : 2; /* break */
            unsigned char D : 1; /* decimal */
            unsigned char I : 1; /* interrupt */
            unsigned char Z : 1; /* zero */
            unsigned char C : 1; /* carry */
#endif
        }_;
        unsigned char P; /* status */
    } status;
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
