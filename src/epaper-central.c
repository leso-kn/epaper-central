/*
 *  SPDX-License-Identifier: MIT
 *  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
 *
 *  epaper-central - C API implementation.
 */

#include "epaper-central.h"

#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "proto.h"
#include "md5.h"

//

const char *ectr_error = NULL;
struct EctrPixmap *(*ectr_pixmap_for_tag_callback)(uint64_t tag_macaddr) = NULL;

//

char _ectr_cmd[4];
int _ectr_fd;
char _ectr_logging_progress = 0;

#define block_size 4096

#define TTY_NORMAL "\033[0m"
#define TTY_GRAY "\033[90m"
#define TTY_GREEN "\033[32m"
#define TTY_YELLOW "\033[33m"

#define _ectr_error(format, args...) \
    ectr_error = malloc(1024); \
    sprintf((char*)ectr_error, format, args)

//

unsigned int _ectr_sum(unsigned char *data, unsigned int len)
{
	uint64_t s = 0;
	for (unsigned int i = 0; i < len; i++)
    s += data[i];
	return s;
}

void _ectr_xor(unsigned char *dest, unsigned char *src, unsigned int len)
{
    for (unsigned int i = 0; i < len; i ++)
    dest[i] = src[i] ^ 0xAA;
}

unsigned char _ectr_bits_to_byte(char b[8])
{
    unsigned char c = 0;
    for (int i=0; i < 8; ++i)
        if (b[i])
            c |= 1 << i;
    return c;
}

ssize_t _ectr_readexact(int fd, void *buf, size_t n)
{
    size_t i = 0;
    while (i < n)
    i += read(fd, buf+i, n-i);
    return i;
}

void _ectr_wait_cmd(char cmd[4])
{
    char dat[4];
    while (strncmp(dat, cmd, 4))
    {
        dat[0] = dat[1];
        dat[1] = dat[2];
        dat[2] = dat[3];
        _ectr_readexact(_ectr_fd, dat+3, 1);
    }
}

void _ectr_print_mac(FILE *dest, uint64_t mac)
{
    for (unsigned char i = 0; i < 8; i++)
    fprintf(dest, i < 7 ? "%02x:" : "%02x", *(((unsigned char*)&mac)+7-i));
}

void _ectr_send_img_init(uint64_t target_mac)
{
    if (!ectr_pixmap_for_tag_callback) return;
    struct EctrPixmap *pix = ectr_pixmap_for_tag_callback(target_mac);

    struct AvailDataInfo pkt;
    MD5Context ctx;
    md5Init(&ctx);
    md5Update(&ctx, (uint8_t *)pix->pixels, pix->len);
    md5Finalize(&ctx);

	pkt.dataVer = *(uint64_t*)ctx.digest;
	pkt.dataSize = pix->len;
	pkt.dataType = 0x21;
	pkt.dataTypeArgument = 1;
	pkt.nextCheckIn = 0;
	pkt.attemptsLeft = 60 * 24;
	pkt.targetMac = target_mac;

    pkt.checksum = _ectr_sum(((unsigned char*)&pkt)+1, sizeof(struct AvailDataInfo)-1) % 0x100;

    ctx.digest[8] = 0;

    if (_ectr_logging_progress)
    fprintf(stderr, "\n");
    fprintf(stderr, "\r[ %sok%s ] %sota update tag ", TTY_GREEN, TTY_NORMAL, TTY_GRAY);
    _ectr_print_mac(stderr, target_mac);
    fprintf(stderr, " (v.%08x)%s", *(uint32_t*)ctx.digest, TTY_NORMAL);
    fflush(stderr);

    write(_ectr_fd, "SDA>", 4);
	write(_ectr_fd, &pkt, sizeof(struct AvailDataInfo));
}

void _ectr_send_img_blk(uint64_t target_mac, unsigned int blk_i, uint32_t digest)
{
    if (!ectr_pixmap_for_tag_callback) return;
    struct EctrPixmap *pix = ectr_pixmap_for_tag_callback(target_mac);

	unsigned char pixels_part[block_size];
    unsigned int pixels_part_len = (blk_i + 1) * block_size < pix->len ? block_size : pix->len - blk_i * block_size;
    _ectr_xor(pixels_part, pix->pixels + blk_i * block_size, pixels_part_len);

    struct BlockHeader pkt;
    pkt.length = pixels_part_len;
	pkt.checksum = _ectr_sum(pix->pixels + blk_i * block_size, pixels_part_len) % 0x10000;
    _ectr_xor((unsigned char*)&pkt, (unsigned char*)&pkt, sizeof(struct BlockHeader));

    write(_ectr_fd, ">D>", 3);
    _ectr_wait_cmd("ACK>");

    write(_ectr_fd, &pkt, sizeof(struct BlockHeader));
    write(_ectr_fd, pixels_part, pixels_part_len);

    unsigned char pad[block_size - pixels_part_len];
	for (unsigned int i = 0; i < sizeof(pad); i++)
    pad[i] = 0xAA;
    write(_ectr_fd, &pad, sizeof(pad));

    //

    _ectr_logging_progress = 1;
    unsigned int transfered = block_size*blk_i+pixels_part_len;
    FILE *tty = transfered >= pix->len ? stdout : stderr;

    fprintf(tty, "\r[%s%3i%%%s] ota update tag ", transfered >= pix->len ? TTY_GREEN : TTY_YELLOW, 100*transfered/pix->len, TTY_NORMAL);
    _ectr_print_mac(tty, target_mac);
    fprintf(tty, " (v.%08x)", digest);
    if (transfered >= pix->len)
    {
        printf("\n");
        _ectr_logging_progress = 0;
    }
    else
    fflush(tty);
}

/*
 *  API
 */

int ectr_init(const char *dev)
{
    _ectr_fd = open(dev, O_RDWR | O_NOCTTY);
    if (_ectr_fd < 0)
    {
        _ectr_error("error: could not open %s\n", dev);
        return 1;
    }

    // Setup serial parameters
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr(_ectr_fd, &tty)) {
        _ectr_error("error: could not get serial tty parameters (code %i: %s)\n", errno, strerror(errno));
        return 1;
    }

    cfsetospeed (&tty, (speed_t)B115200);
    cfsetispeed (&tty, (speed_t)B115200);

    tty.c_cc[VMIN] = 1;

    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &=  ~(PARENB | PARODD); // 8n1
    tty.c_cflag &=  ~CSIZE;
    tty.c_cflag |=  CSTOPB;
    tty.c_cflag |=  CS8;
    cfmakeraw(&tty);

    tcflush(_ectr_fd, TCIFLUSH);
    if (tcsetattr(_ectr_fd, TCSANOW, &tty)) {
        _ectr_error("error: could not set serial tty parameters (code %i: %s\n", errno, strerror(errno));
        return 1;
    }

    return 0;
}

int ectr_poll()
{
    int ret = ECTR_POLL_NOTHING;

    _ectr_cmd[0] = _ectr_cmd[1];
    _ectr_cmd[1] = _ectr_cmd[2];
    _ectr_cmd[2] = _ectr_cmd[3];
    int r = _ectr_readexact(_ectr_fd, _ectr_cmd+3, 1);

    if (strncmp(_ectr_cmd, "ADR>", 4) == 0)
    {
        ret = ECTR_POLL_AVAILABLE_DATA_REQUEST;
        struct AvailableDataRequest req;
        char nul[13];
        _ectr_readexact(_ectr_fd, &req, sizeof(struct AvailableDataRequest));
        _ectr_readexact(_ectr_fd, nul, sizeof(nul));

        usleep(500000);
        _ectr_send_img_init(req.sourceMac);
    }
    else if (strncmp(_ectr_cmd, "RQB>", 4) == 0)
    {
        ret = ECTR_POLL_BLOCK_REQUEST;
        struct BlockRequest req;
        _ectr_readexact(_ectr_fd, &req, sizeof(struct BlockRequest));

        _ectr_send_img_blk(req.srcMac, req.blockId, req.dataVer);
    }
    else if (strncmp(_ectr_cmd, "XFC>", 4) == 0)
    {
        ret = ECTR_POLL_TRANSFER_COMPLETE;
        char data[9];
        _ectr_readexact(_ectr_fd, data, sizeof(data));
    }
    else if (strncmp(_ectr_cmd, "ACK>", 4) == 0)
    {
        ret = ECTR_POLL_ACK;
        //
    }

    return ret;
}

void ectr_pixmap_free(struct EctrPixmap *pixmap)
{
    free(pixmap->pixels);
    free(pixmap);
}

struct EctrPixmap *ectr_pixmap_from_rgb(unsigned char *rgb, unsigned int length, unsigned int w)
{
    struct EctrPixmap *pixmap = malloc(sizeof(struct EctrPixmap));

    unsigned int h = length / w / 3, pixmap_i = 0;
    unsigned char bitarray[8];
    unsigned char bitptr = 0, r, g, b;

    pixmap->len = length * 2 / 3 / 8; // 3 channel r/g/b to 2 channel blk/r
    pixmap->pixels = malloc(pixmap->len);

    for (unsigned char color = 0; color < 2; color++)
        for (unsigned int x = w-1; x > 0; x--)
            for (unsigned int y = 0; y < h; y++)
            {
                r = rgb[(y * w + x) * 3];
                g = rgb[(y * w + x) * 3+1];
                b = rgb[(y * w + x) * 3+2];

                bitarray[7-bitptr] =
                    color
                    ? (/*red*/   r > 127 && g + b < 127*2)
                    : (/*black*/ r + g + b < 127*3);
                bitptr++;
                if (bitptr>=8)
                {
                    bitptr = 0;
                    pixmap->pixels[pixmap_i] = _ectr_bits_to_byte(bitarray);
                    pixmap_i++;
                }
            }

    return pixmap;
}
