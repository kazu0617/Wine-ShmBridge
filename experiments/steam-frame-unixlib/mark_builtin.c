#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    static const char signature[] = "Wine builtin DLL";
    FILE *file;
    if (argc != 2 || !(file = fopen(argv[1], "r+b"))) return 1;
    if (fseek(file, 64, SEEK_SET) || fwrite(signature, sizeof(signature), 1, file) != 1)
    {
        fclose(file);
        return 2;
    }
    return fclose(file) != 0;
}
