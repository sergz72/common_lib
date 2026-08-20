#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "logger.h"
#include "object_file_loader.h"
#include "common.h"
#include <windows.h>
#include <memoryapi.h>

#define BSS_SIZE 10240

const function_def function_map[] = {
    {"logger", &logger},
    {NULL, NULL}
};

void logger(int level, const char *format, ...)
{
    va_list vArgs;

    va_start(vArgs, format);
    switch (level)
    {
        case LOG_LEVEL_ERROR:
            printf("ERROR ");
            break;
        case LOG_LEVEL_WARNING:
            printf("WARNING ");
            break;
        case LOG_LEVEL_INFO:
            printf("INFO ");
            break;
        case LOG_LEVEL_DEBUG:
            printf("DEBUG ");
            break;
        default:
            printf("LEVEL %d ", level);
            break;
    }
    vprintf(format, vArgs);
    printf("\n");
    va_end(vArgs);
}

int main(int argc, const char **argv)
{
    FILE *pfile;
    long file_size;
    void *buffer;
    int rc;

    if (argc != 2)
    {
        puts("Usage: ObjectLoader object_file");
        return 1;
    }

    pfile = fopen(argv[1], "rb");
    if (!pfile)
    {
        puts("fopen error");
        return 2;
    }

    if (fseek(pfile, 0, SEEK_END))
    {
        fclose(pfile);
        puts("fseek error");
        return 2;
    }

    file_size = ftell(pfile);
    if (file_size == -1)
    {
        fclose(pfile);
        puts("ftell error");
        return 3;
    }
    rewind(pfile);

    buffer = VirtualAlloc(NULL, file_size + BSS_SIZE + sizeof(void*),
                            MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!buffer)
    {
        fclose(pfile);
        puts("out of memory");
        return 4;
    }

    if (fread(buffer, 1, file_size, pfile) != file_size)
    {
        VirtualFree(pfile, 0, MEM_RELEASE);
        puts("fread error");
        return 5;
    }

    fclose(pfile);

    file_size = (file_size / sizeof(void*) + 1) * sizeof(void*);
    char *bss = (char*)buffer + file_size;

    rc = object_file_load(buffer, function_map, bss, BSS_SIZE);
    printf("object_file_load returned %d\n", rc);
    if (rc != 0)
    {
        VirtualFree(pfile, 0, MEM_RELEASE);
        return rc;
    }

    rc = object_file_call("test_main");
    printf("object_file_call returned %d\n", rc);

    VirtualFree(pfile, 0, MEM_RELEASE);

    return rc;
}
