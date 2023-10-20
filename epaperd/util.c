/*
 *  SPDX-License-Identifier: MIT
 *  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
 *
 *  epaper-central - "epaperd" utility functions implementation.
 */

#include "util.h"

#include "epaper-central.h"
#include "font.h"
#include "ppm.h"

#include <stdio.h>
#include <stdlib.h>

struct PpmData *load_ppm(const char *filename)
{
    FILE *pixels_fd = fopen(filename, "r");

    if (!pixels_fd)
    {
        printf("error: could not open %s\n", filename);
        exit(1);
    }

    struct PpmData *res = malloc(sizeof(struct PpmData));

    fseek(pixels_fd, 0, SEEK_END);
    res->len = ftell(pixels_fd);
    rewind(pixels_fd);

    res->pixels = malloc(res->len);
    fread(res->pixels, 1, res->len, pixels_fd);
    fclose(pixels_fd);

    struct PpmHeader pixels_hdr = parsePpmHeader(res->pixels, res->len);
    res->w = pixels_hdr.w;
    res->h = pixels_hdr.h;
    res->_pxlOffset = pixels_hdr.pxlOffset;
    res->pixels += res->_pxlOffset;

    return res;
}

void mac_to_string(char dest[25], uint64_t mac, char sep)
{
    for (unsigned char i = 0; i < 8; i++)
    sprintf(dest+i*(2+(sep!=0)), (i < 7 && sep!=0) ? "%02x%c" : "%02x", *(((unsigned char*)&mac)+7-i), sep);
}

unsigned char _ectr_bits_to_byte(char b[8])
{
    unsigned char c = 0;
    for (int i=0; i < 8; ++i)
        if (b[i])
            c |= 1 << i;
    return c;
}

void ppm_draw_hex_text(struct PpmData *ppm, const char *text, unsigned int x, unsigned int y, unsigned char color)
{
    unsigned int bit_i;

    for (int i = strlen(text)-1; i >= 0; i--)
    {
        char *c = text[i] == ':' ? font_colon_bits : font_hex[text[i] >= 'a' ? text[i] - 'a' + 10 : text[i] - '0'];

        for (unsigned int _y = 0; _y < font_height; _y++)
            for (unsigned int _x = 0; _x < font_width; _x++)
            {
                bit_i = _y * font_width + _x;
                unsigned char pxl = (c[bit_i / 7] >> bit_i % 7) & 1;

                if (pxl)
                {
                    uint32_t pxl_i = (((y + _y) * ppm->w) + x + font_width*i + _x) * 3; // rgb
                    if (color == 1)
                    {
                        // red
                        ppm->pixels[pxl_i+0] = 255;
                        ppm->pixels[pxl_i+1] = 0;
                        ppm->pixels[pxl_i+2] = 0;
                    }
                    else
                    {
                        // black
                        ppm->pixels[pxl_i+0] = 0;
                        ppm->pixels[pxl_i+1] = 0;
                        ppm->pixels[pxl_i+2] = 0;
                    }
                }
            }
        }
}
