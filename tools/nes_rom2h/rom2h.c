#include <stdio.h>

int main(int argc, char**argv) {
    int i;
    unsigned char buffer[0x4000] = {0};
    FILE* file = fopen("nestest.nes", "rb");
    if (!file) {
        perror("failed to open nestest.nes\n");
        return -1;
    }

    fseek(file, 0x10, 0);
    fread(buffer, sizeof(buffer), 1, file);
    fclose(file);

    file = fopen("nestest_rom.h", "wb");
    if (!file) {
        perror("failed to open nestest_rom.h\n");
        return -1;
    }

    fputs("#ifndef NESTEST_ROM_H\n#define NESTEST_ROM_H\n", file);
    fputs("\nstatic unsigned char NESTEST_ROM[0x4000] = {", file);
    for (i = 0; i < sizeof(buffer); i++) {
        if ((i % 0x10) == 0) {
            fputs("\n\t", file);
        }
        fprintf(file, "0x%02X,", buffer[i]);
    }
    fputs("\n};\n", file);

    fputs("\n#endif /* NESTEST_ROM_H */\n", file);
    fclose(file);

    return 0;
}
