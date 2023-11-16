#include "webmreader.h"
#include <unistd.h>

off_t findOffset(int fd, const unsigned char ID[], unsigned int IDsize, unsigned long int size)
{
    unsigned char byte;
    unsigned int state = 0;
    off_t foundPosition = -1;
    off_t startingPosition = lseek(fd, 0, SEEK_CUR);
    int found = 0;

    if (size == GLOBAL_SEARCH)
    {
        while (read(fd, &byte, sizeof(byte)) == 1)
        {
            if (byte == ID[state])
            {
                if (state == 0)
                {
                    foundPosition = lseek(fd, 0, SEEK_CUR) - 1;
                }
                state++;

                if (state == IDsize)
                {
                    found = 1;
                    break;
                }
            }
            else
            {
                state = 0;
            }
        }

        lseek(fd, startingPosition, SEEK_SET);

        if (found == 1)
            return foundPosition;

        return -1;
    }
    else
    {

        unsigned long int iterations = 0;

        while ((read(fd, &byte, sizeof(byte)) == 1) && iterations < size)
        {
            ++iterations;

            if (byte == ID[state])
            {
                if (state == 0)
                {
                    foundPosition = lseek(fd, 0, SEEK_CUR) - 1;
                }
                state++;

                if (state == IDsize)
                {
                    found = 1;
                    break;
                }
            }
            else
            {
                state = 0;
            }
        }

        lseek(fd, startingPosition, SEEK_SET);

        if (found == 1)
            return foundPosition;

        return -1;
    }
}