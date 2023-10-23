Building
========

``libepaper-central`` is compiled using `CMake <https://cmake.org>`_.

Despite CMake itself, the only dependency for building the base library and epaperd is a C compiler.

If you would like to build the documentation, make sure `Doxygen <https://www.doxygen.nl/index.html>`_ is installed.

.. code-block:: bash

   > mkdir build
   > cmake -B build .
   > cmake --build build
   > cmake --install build

For Package Maintainers
-----------------------

The library supports the following CMake-variables to customize system paths:

.. code-block:: bash

   > cmake -DCMAKE_INSTALL_PREFIX=/usr \
           -DCMAKE_INSTALL_INCLUDEDIR=/usr/include \
           -DCMAKE_INSTALL_LIBDIR=/usr/lib \
           -DCMAKE_INSTALL_DATADIR=/usr/share .

* ``INCLUDEDIR`` install location for C headers
* ``LIBDIR`` install location for libs and pkg-config files
* ``DATADIR`` install location for runtime resources and man pages

All variables above use ``CMAKE_INSTALL_PREFIX`` as prefix by default.

