/*
 *  SPDX-License-Identifier: MIT
 *  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
 *
 *  epaper-central - C API definition.
 */

#ifndef EPAPER_CENTRAL_H
#define EPAPER_CENTRAL_H

#include <stdint.h>

#define ECTR_POLL_NOTHING 0
#define ECTR_POLL_ACK 1
#define ECTR_POLL_AVAILABLE_DATA_REQUEST 2
#define ECTR_POLL_BLOCK_REQUEST 3
#define ECTR_POLL_TRANSFER_COMPLETE 4

/**
 * Struct representing a black-and-red pixmap for EPaper tags.
 * 
 * Pixmaps can be created using the `ectr_pixmap_from_*` methods.
 */
struct EctrPixmap
{
    unsigned char *pixels;
    unsigned int len;
};

/**
 * Human-friendly description of the latest error thrown by the library or NULL if no error was thrown.
 */
extern const char *ectr_error;

/**
 * @brief Callback invoked by the library to retrieve the pixmap that should be sent
 * to a tag from the user.
 * 
 * This method is called every time a tag checks in *and* for each individual chunk of
 * the transfer; so heavy generated pixmaps should be cached before being accessed in
 * this method.
 * 
 * However, there's also onboard-caching on the tag-side: This means the library-user
 * does not need to keep track of changes to pixmaps. In other words: Once a pixmap has
 * been fully received, by a tag, this method will only be called a single-time for each
 * check-in until the data changes.
 * 
 * @param tag_macaddr The mac address of the tag that just checked in for new data.
 * @return The pixmap that should be sent to the tag.
 */
extern struct EctrPixmap *(*ectr_pixmap_for_tag_callback)(uint64_t tag_macaddr);

/**
 * @brief Initializes the station library and opens a serial connection to
 * the CC2531 ZigBee stick.
 * 
 * @param dev Name of the ZigBee device (e.g. COM1 on Windows or /dev/ttyACM0 on UNIX)
 * @return 0 on success, !0 otherwise. Populates `ectr_error` in case of failure.
 */
extern int ectr_init(const char *dev);

/**
 * @brief Listens for a single ZigBee packet from an EPaper tag and handles it.
 * 
 * To be used in a loop.
 * 
 * @return The type of the received ZigBee packet as ECTR_POLL_[..].
 */
extern int ectr_poll();

/**
 * @brief Frees the memory allocated by a given pixmap.
 * 
 * @param pixmap The pixmap that should be freed.
 */
extern void ectr_pixmap_free(struct EctrPixmap *pixmap);

/**
 * @brief Creates a new EctrPixmap from an 8-bit (1byte per color) rgb pixel array.
 * 
 * @param rgb Pointer to the rgb pixel array.
 * @param length Size of the rgb pixel array data in bytes.
 * @param w Width of the image, height is calculated auotmatically based on `length`.
 * @return struct EctrPixmap* Pointer to a new EctrPixmap consisting of the input pixels.
 */
extern struct EctrPixmap *ectr_pixmap_from_rgb(unsigned char *rgb, unsigned int length, unsigned int w);

#endif // EPAPER_CENTRAL_H
