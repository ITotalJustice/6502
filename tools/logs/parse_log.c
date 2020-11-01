// very unsafe nestest parser.
// don't care about code quality or flexibility, just needed it to work :)
// public domain, should anyone care.

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define CHUNK_SIZE 10000

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef struct {
    u8 *opcode, *A, *X, *Y, *P, *SP;
    u16 *PC;
    u32 *cyc;

    char *data;
    u64 data_size;
    u64 data_pos;

    u64 entry_count;
    u64 alloc_count;
} context_t;

static void ctx_realloc(context_t *ctx) {
    u64 new_size = CHUNK_SIZE + (ctx->alloc_count * CHUNK_SIZE);

    ctx->opcode = realloc(ctx->opcode, new_size); assert(ctx->opcode);
    ctx->A = realloc(ctx->A, new_size); assert(ctx->A);
    ctx->X = realloc(ctx->X, new_size); assert(ctx->X);
    ctx->Y = realloc(ctx->Y, new_size); assert(ctx->Y);
    ctx->P = realloc(ctx->P, new_size); assert(ctx->P);
    ctx->SP = realloc(ctx->SP, new_size); assert(ctx->SP);
    ctx->PC = realloc(ctx->PC, new_size * sizeof(u16)); assert(ctx->PC);
    ctx->cyc = realloc(ctx->cyc, new_size * sizeof(u32)); assert(ctx->cyc);
    ctx->alloc_count++;
}

static u8 ctx_fetch_byte(context_t *ctx, int pre) {
    ctx->data_pos += pre;
    char buf[5] = "0x";
    strncat(buf, ctx->data + ctx->data_pos, 2);
    ctx->data_pos += 3;
    return strtoul(buf, NULL, 0x10);
}

static u16 ctx_fetch_word(context_t *ctx, int pre) {
    ctx->data_pos += pre;
    char buf[7] = "0x";
    strncat(buf, ctx->data + ctx->data_pos, 4);
    ctx->data_pos += 5;
    return strtoul(buf, NULL, 0x10);
}

static u16 ctx_fetch_cyc(context_t *ctx, int pre) {
    ctx->data_pos += pre;
    char buf[0x10] = {0};
    int ed = 0;
    for (; ctx->data[ctx->data_pos + ed] != '\n'; ed++);
    strncat(buf, ctx->data + ctx->data_pos, ed);
    ctx->data_pos += ed;
    return strtoul(buf, NULL, 10);
}

static void ctx_dump_byte(FILE *fout, context_t *ctx, u8 *mbyte, const char *name) {
    fprintf(fout, "\nstatic const unsigned char %s[] = {\n\t", name);
    for (u64 i = 0; i < ctx->entry_count; i++) {
        if ((i % 0x10) == 0 && i) {
            fputs("\n\t", fout);
        }
        fprintf(fout, "0x%02X,", mbyte[i]);
    }
    fprintf(fout, "\n};\n");
}

static void ctx_dump_word(FILE *fout, const context_t *ctx, const u16 *word, const char *name) {
    fprintf(fout, "\nstatic const unsigned short %s[] = {\n\t", name);
    for (u64 i = 0; i < ctx->entry_count; i++) {
        if ((i % 0x10) == 0 && i) {
            fputs("\n\t", fout);
        }
        fprintf(fout, "0x%04X,", word[i]);
    }
    fprintf(fout, "\n};\n");
}

static void ctx_dump_dub_word(FILE *fout, const context_t *ctx, const u32 *word, const char *name) {
    fprintf(fout, "\nstatic const unsigned int %s[] = {\n\t", name);
    for (u64 i = 0; i < ctx->entry_count; i++) {
        if ((i % 0x10) == 0 && i) {
            fputs("\n\t", fout);
        }
        fprintf(fout, "%u,", word[i]);
    }
    fprintf(fout, "\n};\n");
}

int main(int argc, char **argv) {
    context_t ctx = {0};
    ctx.SP = NULL;

    if (argc < 3) {
        fprintf(stderr, "usage: %s path_to_log.txt output.h\n", argv[0]);
        exit(-1);
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        fprintf(stderr, "failed to open file\n");
        exit(-1);
    }

    FILE *fout = fopen(argv[2], "wb");
    if (!fout) {
        fprintf(stderr, "failed to open out file\n");
        fclose(fp);
        exit(-1);
    }

    fseek(fp, 0, SEEK_END);
    ctx.data_size = ftell(fp);
    rewind(fp);
    ctx.data = malloc(ctx.data_size + 1);
    if (!fread(ctx.data, ctx.data_size, 1, fp)) {
        fprintf(stderr, "failed to read file\n");
        exit(-1);
    }
    fclose(fp);

    ctx_realloc(&ctx);

    while (ctx.data_pos < ctx.data_size) {
        if ((ctx.entry_count % (CHUNK_SIZE - 1)) == 0) {
            ctx_realloc(&ctx);
        }

        ctx.PC[ctx.entry_count] = ctx_fetch_word(&ctx, 0);
        ctx.opcode[ctx.entry_count] = ctx_fetch_byte(&ctx, 1);

        ctx.A[ctx.entry_count] = ctx_fetch_byte(&ctx, 41);
        ctx.X[ctx.entry_count] = ctx_fetch_byte(&ctx, 2);
        ctx.Y[ctx.entry_count] = ctx_fetch_byte(&ctx, 2);
        ctx.P[ctx.entry_count] = ctx_fetch_byte(&ctx, 2);
        ctx.SP[ctx.entry_count] = ctx_fetch_byte(&ctx, 3);
        ctx.cyc[ctx.entry_count] = ctx_fetch_cyc(&ctx, 16);

        // skip to new line
        while (ctx.data_pos < ctx.data_size) {
            if (ctx.data[ctx.data_pos++] != '\n') {
                continue;
            }
            break;
        }

        ctx.entry_count++;
    }

    fprintf(fout, "/*parse_log - 0.0.1: auto generate C/C++ array from nestest.log data*/\n\n");
    fprintf(fout, "#ifndef NESTEST_LOG_H\n#define NESTEST_LOG_H\n");

    ctx_dump_byte(fout, &ctx, ctx.opcode, "ARR_OPCODE");
    ctx_dump_byte(fout, &ctx, ctx.A, "ARR_A");
    ctx_dump_byte(fout, &ctx, ctx.X, "ARR_X");
    ctx_dump_byte(fout, &ctx, ctx.Y, "ARR_Y");
    ctx_dump_byte(fout, &ctx, ctx.P, "ARR_P");
    ctx_dump_byte(fout, &ctx, ctx.SP, "ARR_SP");
    ctx_dump_word(fout, &ctx, ctx.PC, "ARR_PC");
    ctx_dump_dub_word(fout, &ctx, ctx.cyc, "ARR_CYC");

    fprintf(fout, "\n#endif /*NESTEST_LOG_H*/\n");

    printf("done!\n");

    fclose(fout);

    free(ctx.opcode);
    free(ctx.A);
    free(ctx.X);
    free(ctx.Y);
    free(ctx.P);
    free(ctx.SP);
    free(ctx.PC);
    free(ctx.cyc);
    free(ctx.data);

    return 0;
}