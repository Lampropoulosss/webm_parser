// #include <stdio.h>
// #include <unistd.h>
// #include <stdlib.h>
// #include <fcntl.h>
// #include <stdint.h>

// #define BUFSIZE 1048576 // 1MB

// void printBinaryInt(uint8_t value)
// {
//     for (int i = sizeof(uint8_t) * 8 - 1; i >= 0; i--)
//     {
//         printf("%u", (value >> i) & 1);
//         fflush(stdout);
//     }
// }

// enum ebml_element_type
// {
//     MASTER,
//     UINT,
//     INT,
//     STRING,
//     UTF8,
//     BINARY,
//     FLOAT,
//     DATE
// };

// typedef struct simple_vint
// {
//     uint8_t width;
//     uint8_t data[8];
// } simple_vint;

// uint64_t vint_get_uint(simple_vint *vint)
// {
//     uint64_t value = 0;
//     value = vint->data[vint->width - 1];

//     for (int i = vint->width - 1; i > 0; --i)
//     {
//         value += ((uint64_t)vint->data[i - 1] << ((vint->width - i) * 8));
//     }

//     return value;
// }

// int main()
// {

//     int fd = open("stream.webm", O_RDONLY);
//     if (fd == -1)
//     {
//         printf("err");
//         return -1;
//     }

//     uint8_t buffer[BUFSIZE];
//     int len, pos = 0;
//     uint8_t mask;

//     while (1)
//     {
//         if ((len = read(fd, buffer, 1)) < 0)
//         {
//             printf("couldnt read starting byte\n");
//             return EXIT_FAILURE;
//         }
//         else if (len == 0)
//         {
//             printf("done\n");
//             break;
//         }

//         pos++;

//         if (buffer[0] == 0)
//         {
//             printf("read 0 byte\n");
//             continue;
//         }

//         unsigned char bits = buffer[0];

//         simple_vint id;
//         id.width = 1;
//         mask = 0x80;

//         while (!(buffer[0] & mask))
//         {
//             mask >>= 1;
//             id.width++;
//         }

//         id.data[0] = buffer[0];

//         if ((len = read(fd, buffer, id.width - 1)) != id.width - 1)
//         {
//             printf("Uh oh, read id data error!\n");
//             return EXIT_FAILURE;
//         }
//         pos += id.width - 1;

//         printf("============================================\n");
//         printf("Element ID First Byte: ");
//         fflush(stdout);
//         printBinaryInt(bits);
//         printf("\n");
//         fflush(stdout);
//         printf("Element ID Bytes: ");
//         fflush(stdout);
//         printBinaryInt(bits);

//         // Get EBML Element ID.
//         for (int i = 1; i < id.width; ++i)
//         {
//             id.data[i] = buffer[i - 1];
//             bits = buffer[i - 1];
//             printf(" ");
//             printBinaryInt(bits);
//             fflush(stdout);
//         }

//         printf("\n");

//         // Get EBML Element Size first byte.
//         if ((len = read(fd, buffer, 1)) != 1)
//         {
//             printf("Uh oh, read first size byte error!\n");
//             return EXIT_FAILURE;
//         }

//         pos++;
//         bits = buffer[0];

//         printf("Element Size  First Byte: ");
//         printBinaryInt(bits);
//         printf("\n");

//         simple_vint size;
//         size.width = 1;
//         mask = 0x80;

//         // Get EBML Element Size vint width.
//         while (!(buffer[0] & mask))
//         {
//             mask >>= 1;
//             size.width++;
//         }

//         buffer[0] ^= mask;
//         size.data[0] = buffer[0];

//         // Get EBML Element Size vint data.
//         if ((len = read(fd, buffer, size.width - 1)) != size.width - 1)
//         {
//             printf("Uh oh, read id data error!\n");
//             return EXIT_FAILURE;
//         }

//         pos += size.width - 1;

//         bits = size.data[0];

//         printf("Element Size Bytes: ");
//         printBinaryInt(bits);

//         // Get EBML Element Size.
//         for (int i = 1; i < size.width; ++i)
//         {
//             size.data[i] = buffer[i - 1];
//             bits = buffer[i - 1];
//             printf(" ");
//             printBinaryInt(bits);
//         }

//         printf("\n");

//         uint64_t data_len = vint_get_uint(&size);
//         printf("Element Size: %lu\n", data_len);

//         if ((len = read(fd, buffer, data_len) != data_len))
//         {
//             printf("Uh oh... Could not read all the data. Read %u instead of %lu", len, data_len);
//             return EXIT_FAILURE;
//         }

//         printf("\n==========================================================\n==========================================================\n");

//         printf("DATA:\n");

//         for (size_t i = 0; i < data_len; i++)
//         {
//             printBinaryInt(buffer[i]);
//             printf(" ");
//         }

//         printf("\n==========================================================\n==========================================================\n");

//         pos += data_len;
//     }
// }

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

#include "webmreader.h"

int main()
{

    const unsigned char segmentID[] = {0x18, 0x53, 0x80, 0x67};
    off_t segmentPosition;
    const unsigned char trackID[] = {0x16, 0x54, 0xAE, 0x6B, 0xD6, 0xAE};
    off_t trackPosition;
    const unsigned char trackEntryID[] = {0xAE};
    off_t trackEntryPosition;
    const unsigned char trackType[] = {0x83};
    const unsigned char trackNumberID[] = {0xD7};
    off_t trackNumberPosition;

    uint8_t buffer[BUFSIZE];
    int len;
    uint8_t mask;

    int fd = open("stream.webm", O_RDONLY);
    if (fd == -1)
    {
        printf("Could not open the file.");
        return EXIT_FAILURE;
    }

    if ((segmentPosition = findOffset(fd, segmentID, sizeof(segmentID), GLOBAL_SEARCH)) == -1)
    {
        printf("Could not find the segment.");
        return EXIT_FAILURE;
    }

    lseek(fd, segmentPosition, SEEK_SET); // Find and go to the Segment Element

    if ((len = read(fd, buffer, 1)) < 0)
    {
        printf("couldnt read starting byte\n");
        return EXIT_FAILURE;
    }
    else if (len == 0)
    {
        printf("done\n");
        return EXIT_SUCCESS;
    }

    simple_vint id;
    mask = 0b10000000;
    id.width = 1;

    while (!(buffer[0] & mask))
    {
        mask >>= 1;
        id.width++;
    }

    id.data[0] = buffer[0];

    if ((len = read(fd, buffer, id.width - 1)) != id.width - 1)
    {
        printf("Uh oh, read id data error!\n");
        return EXIT_FAILURE;
    }

    // Push Element ID to id.data.
    for (int i = 1; i < id.width; ++i)
    {
        id.data[i] = buffer[i - 1];
    }

    simple_vint size;
    size.width = 1;
    mask = 0b10000000;

    // Get Element size width
    while (!(buffer[0] & mask))
    {
        mask >>= 1;
        size.width++;
    }

    buffer[0] ^= mask;
    size.data[0] = buffer[0];

    // Get EBML Element Size data.
    if ((len = read(fd, buffer, size.width - 1)) != size.width - 1)
    {
        printf("Uh oh, read id data error!\n");
        return EXIT_FAILURE;
    }

    segmentPosition = lseek(fd, 0, SEEK_CUR);

    uint64_t segment_len = vint_get_uint(&size);
    uint64_t data_len;

    if ((trackPosition = findOffset(fd, trackID, sizeof(trackID), segment_len)) == -1) // Find the Track element inside the Segment.
    {
        printf("Couldnt not find the desired Track");
        return EXIT_FAILURE;
    }

    lseek(fd, trackPosition + 4, SEEK_SET); // Move to the Track element size byte position.

    if ((len = read(fd, buffer, 1)) != 1)
    {
        printf("Could not read the track size byte!\n");
        return EXIT_FAILURE;
    }

    size.width = 1;
    mask = 0b10000000;

    // Get Element size width
    while (!(buffer[0] & mask))
    {
        mask >>= 1;
        size.width++;
    }

    buffer[0] ^= mask;
    size.data[0] = buffer[0];

    // Get EBML Element Size data.
    if ((len = read(fd, buffer, size.width - 1)) != size.width - 1)
    {
        printf("Uh oh, read id data error!\n");
        return EXIT_FAILURE;
    }

    data_len = vint_get_uint(&size); // Track data size

    if ((trackEntryPosition = findOffset(fd, trackEntryID, sizeof(trackEntryID), data_len)) == -1) // Find the TrackEntry element inside the Segment.
    {
        printf("Couldnt not find the track entry");
        return EXIT_FAILURE;
    }

    lseek(fd, trackEntryPosition + sizeof(trackEntryID), SEEK_SET); // Move to the TrackEntry element size byte position.

    if ((len = read(fd, buffer, 1)) != 1)
    {
        printf("Could not read the track size byte!\n");
        return EXIT_FAILURE;
    }

    size.width = 1;
    mask = 0b10000000;

    // Get Element size width
    while (!(buffer[0] & mask))
    {
        mask >>= 1;
        size.width++;
    }

    buffer[0] ^= mask;
    size.data[0] = buffer[0];

    // Get EBML Element Size data.
    if ((len = read(fd, buffer, size.width - 1)) != size.width - 1)
    {
        printf("Uh oh, read id data error!\n");
        return EXIT_FAILURE;
    }

    data_len = vint_get_uint(&size);             // TrackEntry data size
    trackEntryPosition = lseek(fd, 0, SEEK_CUR); // trackEntryPosition now points to the byte from which the "data part" starts

    lseek(fd, findOffset(fd, trackType, sizeof(trackType), data_len) + sizeof(trackType) + 1, SEEK_SET); // Move to the start of the trackType data

    if ((len = read(fd, buffer, 1)) != 1)
    {
        printf("Could not read the track size byte!\n");
        return EXIT_FAILURE;
    }

    // buffer[0] now contains the type of the track
    if (!buffer[0] == 2)
    {
        printf("Uh oh, the TrackEntry does not contain the audio...\n");
        return EXIT_FAILURE;
    }

    lseek(fd, trackEntryPosition, SEEK_SET); // Move back to the TrackEntry data size byte

    if ((trackEntryPosition = findOffset(fd, trackNumberID, sizeof(trackNumberID), data_len)) == -1) // data_len still includes the data length of the TrackEntry
    {
        printf("Couldnt not find the trackNumber");
        return EXIT_FAILURE;
    }

    lseek(fd, trackEntryPosition + sizeof(trackEntryID), SEEK_SET); // Move to the TrackEntry element size byte position.

    if ((len = read(fd, buffer, 1)) != 1)
    {
        printf("Could not read the trackNumber size byte!\n");
        return EXIT_FAILURE;
    }

    size.width = 1;
    mask = 0b10000000;

    // Get Element size width
    while (!(buffer[0] & mask))
    {
        mask >>= 1;
        size.width++;
    }

    buffer[0] ^= mask;
    size.data[0] = buffer[0];

    // Get EBML Element Size data.
    if ((len = read(fd, buffer, size.width - 1)) != size.width - 1)
    {
        printf("Uh oh, read id data error!\n");
        return EXIT_FAILURE;
    }

    data_len = vint_get_uint(&size); // TrackNumber data size

    if ((len = read(fd, buffer, data_len) != data_len))
    {
        printf("Uh oh, could not read the trackNumber value\n");
        return EXIT_FAILURE;
    }

    // buffer[0] now contains the track number
    int trackNumber = buffer[0];

    const unsigned char clusterID[] = {0x1F, 0x43, 0xB6, 0x75};

    do
    {
        lseek(fd, segmentPosition, SEEK_SET); // Move to the start of the segment
        off_t clusterPosition = findOffset(fd, clusterID, sizeof(clusterID), segment_len);

    } while (1);

    return 0;
}