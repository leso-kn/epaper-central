/*
 *  SPDX-License-Identifier: MIT
 *  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
 *
 *  epaper-central - "epaperd" reference program.
 */

#include "epaper-central.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

//

struct EctrPixmap *img = NULL;
struct PpmData *default_image = NULL;

//

const char *serial_dev = "/dev/ttyACM0";
const char *cache_dir;

struct option opts[] = {
    { "help", no_argument, 0, 'h' },
    { 0, 0, 0, 0 }
};

void print_usage()
{
    printf("Usage: epaperd [-h] [D /dev/ttyACM0] </cache/directory>\n\n"

           "      -h, --help   Print usage info\n"
           "      -D <dev>     The ZigBee radio device (default: %s)\n\n", serial_dev);
}

void parse_options(int argc, char* const argv[])
{
    int c;
    while (1)
    {
        c = getopt_long(argc, argv, "D:h", opts, 0);
        if (c == -1) break;

        switch (c)
        {
        case 'h':
            print_usage();
            exit(0);
        case 'D':
            serial_dev = optarg;
            break;
        }
    }

    if (argc < optind + 1)
    {
        print_usage();
        exit(1);
    }
    cache_dir = argv[optind];
}

struct EctrPixmap *pixmap_for_tag(uint64_t mac)
{
    char smac[25];
    mac_to_string(smac, mac, 0);

    char filename[strlen(cache_dir)+22];
    sprintf(filename, "%s/%16s.ppm", cache_dir, smac);

    if (access(filename, F_OK) == 0)
    {
        struct PpmData *dat = load_ppm(filename);

        img = ectr_pixmap_from_rgb(dat->pixels, dat->len, dat->w);
        free(dat->pixels-dat->_pxlOffset);

        return img;
    }

    struct PpmData canvas;
    canvas.len = default_image->len;
    canvas.pixels = malloc(canvas.len);
    memcpy(canvas.pixels, default_image->pixels, canvas.len);

    canvas.w = default_image->w;
    canvas.h = default_image->h;

    mac_to_string(smac, mac, ':');
    smac[sizeof(smac)-1] = 0;
    ppm_draw_hex_text(&canvas, smac, 130, 110, 1); // red

    img = ectr_pixmap_from_rgb(canvas.pixels, canvas.len, canvas.w);
    free(canvas.pixels);

    return img;
}

int main(int argc, char* const argv[])
{
    parse_options(argc, argv);
    printf("Running epaperd v%u.%u.%u\n", ECTR_VERSION_MAJOR, ECTR_VERSION_MINOR, ECTR_VERSION_PATCH);

    cache_dir = argv[1];
    default_image = load_ppm(EPAPERD_DATA_DIR"/default.ppm");
    ectr_pixmap_for_tag_callback = &pixmap_for_tag;

    if (ectr_init(serial_dev))
    {
        printf("error: %s\n", ectr_error);
        exit(1);
    }

    while (1)
    {
        ectr_poll();
        if (img)
        {
            ectr_pixmap_free(img);
            img = NULL;
        }
    }

    return 0;
}
