/*
 *  SPDX-License-Identifier: MIT
 *  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
 *
 *  epaper-central - "epaperd" utility functions.
 */

#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

struct PpmData
{
    uint32_t w;
    uint32_t h;
    uint32_t len;
    uint32_t _pxlOffset;
    unsigned char *pixels;
};

struct PpmData *load_ppm(const char *filename);
void mac_to_string(char dest[25], uint64_t mac, char sep);

void ppm_draw_hex_text(struct PpmData *ppm, const char *text, unsigned int x, unsigned int y, unsigned char color);

#endif // UTIL_H
