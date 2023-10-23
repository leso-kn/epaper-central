epaperd
=======

.. note::
    The epaperd-code is being compiled using `CMake <https://cmake.org>`_. Check out the :ref:`reference CMakeLists.txt <CMakeLists.txt>` at the bottom of this page.

The *epaperd* reference-program will will pick up `PPM images (P6) <https://en.wikipedia.org/wiki/Netpbm#Description>`_ from a given folder and use libepaper-central to send them to nearby OpenEPaperLink displays.

Image filenames are expected to be tag mac addresses (e.g. 00112233aabbccdd.ppm for a tag with mac address ``00:11:22:33:aa:bb:cc:dd``) with a fallback to ``default.ppm`` in case no specific image file exists for a tag – in this case the mac address will be visible on the display in addition until an applicable .ppm-file becomes available in the cache folder.

``epaperd``\s code provides a basic overview over the way-of-working with libepaper-central, the main snippet can be seen here:

epaperd.c
*********

.. literalinclude:: ../epaperd/epaperd.c
    :language: c
    :lines: 8-

.. _CMakeLists.txt:

To see all additional sources and headers included by ``epaperd.c``, check out the `epaperd/ <https://github.com/leso-kn/epaper-central/tree/main/epaperd>`_ subdirectory of the main repository.

CMakeLists.txt
**************

.. literalinclude:: ../epaperd/CMakeLists.txt
    :language: cmake
    :lines: 8-14
