
### Require CMake 2.6 or later ###

cmake_minimum_required( VERSION 2.6 FATAL_ERROR )


### Look for boost headers and libraries ###

find_package( Boost 1.46 REQUIRED COMPONENTS asio )

