Configcpp
==========

Welcome to configcpp version **0.6.0**.

Configcpp is a C++ port of the excellent `configuration library by typesafehub <https://github.com/typesafehub/config>`_, a full-featured configuration file library.


Rationale
----------------

The motivation for writing this port was to learn more about the new C++11 standard, as well as trying out some things that could be useful in my other Java port project, `Lucene++ <https://github.com/luceneplusplus/LucenePlusPlus>`_.

**Therefore configcpp will only compile with a C++11 compliant compiler - as of writing that is gcc 4.7.**


Notes
----------------

Full instructions and config file syntax documentation can be found on the `typesafehub/config <https://github.com/typesafehub/config>`_.

Please note that for obvious reasons, Java properties files are not supported.


Components
----------------

- libconfigcpp library
- test_configcpp (unit tests)
- example_configcpp (example application)


Build Instructions using CMake
------------------------------

You'll need Boost 1.46+ installed.

On Debian systems, the following packages are required:

- libboost-filesystem-dev
- libboost-system-dev

First create a build directory::

	$ mkdir build
	$ cd build

Then generate make files, specifying the gcc version::

	$ CXX=g++-4.7 cmake ..

To build and install::

	$ make
	$ make install


To run unit tests
----------------------

Simply run the unit test application::

	$ test/test_configcpp

To list all unit test options, run the test_configcpp with "-h".


Acknowledgments
----------------

- Havoc Pennington (havocp) for inspiring this project.
