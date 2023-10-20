/*
 *  SPDX-License-Identifier: MIT
 *  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
 *
 *  epaper-central - "epaperd" PPM image utilities.
 */

#ifndef PPM_H
#define PPM_H

#include <string.h>
#include <stdlib.h>

struct __attribute__((__packed__)) PpmHeader
{
    uint8_t pbmType;
    uint32_t pxlOffset;
    uint32_t w;
    uint32_t h;
};

struct PpmHeader parsePpmHeader(unsigned char *hdr, unsigned int len_max)
{
    unsigned char param_i = 0;
    unsigned char j = 0;
    struct PpmHeader r;
    for (unsigned int i = 0; i < len_max; i++)
    {
        if (hdr[i] == 10 && param_i > 2)
        {
            r.pxlOffset = i+1;
            break;
        }

        if (hdr[i] == 10 || (param_i == 1 && hdr[j] >= '0' && hdr[j] <= '9' && hdr[i] == ' '))
        {
            hdr[i] = 0;

            if (param_i < 1 || hdr[j] >= '0' && hdr[j] <= '9')
            {
                switch (param_i)
                {
                    case 0: r.pbmType = atoi(hdr+j); break;
                    case 1: r.w = atoi(hdr+j); break;
                    case 2: r.h = atoi(hdr+j); break;
                }
                param_i++;
            } // or else it's a comment (skip)
            j = i+1;
        }
    }
    return r;
}

#endif // PPM_H
