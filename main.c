#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "cricmp.h"
#include "cricmp_types.h"

char* programName;

char* inName;


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

static void PrintUsage() {
    fprintf(stderr,
        "Usage: %s input\n", programName);
}

static int ParseArguments(int argc, char* argv[]) {
    programName = argv[0];

    // skip to usage if no params are specified
    if (argc == 1)
        goto invalid;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0 ||
            strcmp(argv[i], "-h") == 0) {
            goto invalid;
        }
        else {
            inName = argv[i];
        }
    }

    return 1;
invalid:
    PrintUsage();
    return 0;
}

// Writes all cutscene data to the out file
bool WriteOutputFile(u8* data, int dataSize) {
    char* outName = strcpycat(inName, ".out");
    FILE* outFile = fopen(outName, "wb");
    if (outFile == NULL) {
        fprintf(stderr,
            "error opening output file for saving (%s)\n", outName);
        return false;
    }

    // save the data
    fwrite(data, dataSize, 1, outFile);

    fclose(outFile);

    printf("output file to %s\n", outName);
    free(outName);


    return true;
}

u8* ReadInputFile() {
    FILE* inFile = fopen(inName, "rb");
    if (inFile == NULL) {
        fprintf(stderr,
            "error opening input file for reading (%s)\n", inName);
        return NULL;
    }

    fseek(inFile, 0, SEEK_END);
    long size = ftell(inFile) + 1;
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

    return data;
}

// Main entry point of the application
int main(int argc, char* argv[]) {
    if (!ParseArguments(argc, argv))
        exit(EXIT_FAILURE);

    u8* inData = ReadInputFile();
    if (inData == NULL) return EXIT_FAILURE;

    CMP_ReadFile(inData);

    u8* outData = CMP_GetFileBuffer();
    int outSize = CMP_GetFileSize();

    if (outData == NULL) {
        fprintf(stderr,
            "compressed file could not be processed\n");
        exit(EXIT_FAILURE);
        return EXIT_FAILURE;
    }

    WriteOutputFile(outData, outSize);

    return EXIT_SUCCESS;
}