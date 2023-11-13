#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>

#define BUFSIZE 1048576 // 1MB

void printBinaryInt(uint8_t value)
{
    for (int i = sizeof(uint8_t) * 8 - 1; i >= 0; i--)
    {
        printf("%u", (value >> i) & 1);
        fflush(stdout);
    }
}

enum ebml_element_type
{
    MASTER,
    UINT,
    INT,
    STRING,
    UTF8,
    BINARY,
    FLOAT,
    DATE
};

typedef struct simple_vint
{
    uint8_t width;
    uint8_t data[8];
} simple_vint;

uint64_t vint_get_uint(simple_vint *vint)
{
    uint64_t value = 0;
    value = vint->data[vint->width - 1];

    for (int i = vint->width - 1; i > 0; --i)
    {
        value += ((uint64_t)vint->data[i - 1] << ((vint->width - i) * 8));
    }

    return value;
}

int main()
{

    int fd = open("test.webm", O_RDONLY);
    if (fd == -1)
    {
        printf("err");
        return -1;
    }

    uint8_t buffer[BUFSIZE];
    int len, pos = 0;
    uint8_t mask;

    while (1)
    {
        if ((len = read(fd, buffer, 1)) < 0)
        {
            printf("couldnt read starting byte\n");
            break;
        }
        else if (len == 0)
        {
            printf("done\n");
            break;
        }

        pos++;

        if (buffer[0] == 0)
        {
            printf("read 0 byte\n");
            continue;
        }

        unsigned char bits = buffer[0];

        simple_vint id;
        id.width = 1;
        mask = 0x80;

        while (!(buffer[0] & mask))
        {
            mask >>= 1;
            id.width++;
        }

        id.data[0] = buffer[0];

        if ((len = read(fd, buffer, id.width - 1)) != id.width - 1)
        {
            printf("Uh oh, read id data error!\n");
            break;
        }
        pos += id.width - 1;

        printf("============================================\n");
        printf("Element ID First Byte: ");
        fflush(stdout);
        printBinaryInt(bits);
        printf("\n");
        fflush(stdout);
        printf("Element ID Bytes: ");
        fflush(stdout);
        printBinaryInt(bits);

        // Get EBML Element ID.
        for (int i = 1; i < id.width; ++i)
        {
            id.data[i] = buffer[i - 1];
            bits = buffer[i - 1];
            printf(" ");
            printBinaryInt(bits);
            fflush(stdout);
        }

        printf("\n");

        // Get EBML Element Size first byte.
        if ((len = read(fd, buffer, 1)) != 1)
        {
            printf("Uh oh, read first size byte error!\n");
            break;
        }

        pos++;
        bits = buffer[0];

        printf("Element Size  First Byte: ");
        printBinaryInt(bits);
        printf("\n");

        simple_vint size;
        size.width = 1;
        mask = 0x80;

        // Get EBML Element Size vint width.
        while (!(buffer[0] & mask))
        {
            mask >>= 1;
            size.width++;
        }

        buffer[0] ^= mask;
        size.data[0] = buffer[0];

        // Get EBML Element Size vint data.
        if ((len = read(fd, buffer, size.width - 1)) != size.width - 1)
        {
            printf("Uh oh, read id data error!\n");
            break;
        }

        pos += size.width - 1;

        bits = size.data[0];

        printf("Element Size Bytes: ");
        printBinaryInt(bits);

        // Get EBML Element Size.
        for (int i = 1; i < size.width; ++i)
        {
            size.data[i] = buffer[i - 1];
            bits = buffer[i - 1];
            printf(" ");
            printBinaryInt(bits);
        }

        printf("\n");

        uint64_t data_len = vint_get_uint(&size);
        printf("Element Size: %lu\n", data_len);

        if ((len = read(fd, buffer, data_len) != data_len))
        {
            printf("Uh oh... Could not read all the data. Read %u instead of %lu", len, data_len);
        }

        printf("\n==========================================================\n==========================================================\n");

        printf("DATA:\n");

        for (size_t i = 0; i < data_len; i++)
        {
            printBinaryInt(buffer[i]);
            printf(" ");
        }

        printf("\n==========================================================\n==========================================================\n");

        pos += data_len;
    }
}