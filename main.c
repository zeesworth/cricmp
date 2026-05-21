#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "cricmp_enc.h"
#include "cricmp_dec.h"
#include "types.h"
#include "macros.h"

char* programName;
int mode; // 0 = enc, 1 = dec
char* inName;
char* outName = NULL;

#define printf_verbose(...) { if (verbose) fprintf(stderr,__VA_ARGS__); }

// https://stackoverflow.com/a/8465083
// Concatenates two strings together and returns a new string
char* strcpycat(const char* s1, const char* s2)
{
    int length = strlen(s1) + strlen(s2) + 1;
    char* result = malloc(length); // +1 for the null-terminator
    if (result == NULL) {
        fprintf(stderr,
            "out of memory\n");
        exit(EXIT_FAILURE);
    }

    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

static int ParseArguments(int argc, char* argv[]) {
    // skip to usage if no params are specified
    if (argc < 3)
        goto invalid;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0 ||
            strcmp(argv[i], "-h") == 0) {
            goto invalid;
        }
        else {
            if (strcmp(argv[1], "enc") == 0) mode = 0;
            else if (strcmp(argv[1], "dec") == 0) mode = 1;
            else goto invalid;

            inName = argv[2];
            if (argc >= 4) outName = argv[3];
        }
    }

    return 1;
invalid:
    fprintf(stderr, "Usage: CRICMPTool [enc|dec] input [output]\n");
    return 0;
}

// Writes all cutscene data to the out file
bool WriteOutputFile(u8* data, int dataSize, const char* name) {
    FILE* outFile = fopen(name, "wb");
    if (outFile == NULL) return false;
    fwrite(data, dataSize, 1, outFile);
    fclose(outFile);

    return true;
}

u8* ReadInputFile(const char* name, long* out_size) {
    FILE* inFile = fopen(name, "rb");
    if (inFile == NULL) return NULL;

    fseek(inFile, 0, SEEK_END);
    long size = ftell(inFile);
    fseek(inFile, 0, SEEK_SET);  /* same as rewind(f); */

    u8* data = malloc(size);
    if (data == NULL) {
        fprintf(stderr,
            "out of memory\n");
        exit(EXIT_FAILURE);
        return NULL;
    }

    fread(data, size, 1, inFile);
    fclose(inFile);

    *out_size = size;
    return data;
}

// Main entry point of the application
int main(int argc, char* argv[]) {
    if (!ParseArguments(argc, argv))
        exit(EXIT_FAILURE);
    
    char* newOutName = NULL;
    if (outName) newOutName = strdup(outName);

    long inSize;
    u8* inData = ReadInputFile(inName, &inSize);
    if (inData == NULL) {
        fprintf(stderr,
            "error opening input file for reading (%s)\n", inName);
        return EXIT_FAILURE;
    }

    if (mode == 1) {
        // decompress
        CMP_ReadFile(inData);

        u8* outData = CMP_GetFileBuffer();
        int outSize = CMP_GetFileSize();

        if (outData == NULL) {
            fprintf(stderr,
                "compressed file could not be processed\n");
            exit(EXIT_FAILURE);
        }

        if (!newOutName) newOutName = strcpycat(inName, ".out");
        if (!WriteOutputFile(outData, outSize, newOutName)) {
            fprintf(stderr,
                "error opening output file for saving (%s)\n", newOutName);
            return EXIT_FAILURE;
        }
        printf("output file to %s\n", newOutName);
    } else {
        // compress
        CMP_WriteFile(inData, (u32)inSize);

        u8* outData = CMP_GetWriteBuffer();
        int outSize = CMP_GetWriteSize();

        if (!newOutName) newOutName = strcpycat(inName, ".CMP");
        if (outData == NULL) {
            fprintf(stderr,
                "file could not be compressed\n");
            exit(EXIT_FAILURE);
        }
        
        if (!WriteOutputFile(outData, outSize, newOutName)) {
            fprintf(stderr,
                "error opening output file for saving (%s)\n", newOutName);
            return EXIT_FAILURE;
        }
        printf("output file to %s (ratio: %.2f%%)\n", newOutName, ((f32)outSize / (f32)inSize) * 100);
    }

    free(inData);
    free(newOutName);

    return EXIT_SUCCESS;
}