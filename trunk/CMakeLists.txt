
### Require CMake 2.6 or later ###

cmake_minimum_required ( VERSION 2.6 FATAL_ERROR )

project ( BUZZFTPD )

### Look for boost headers and libraries ###

find_package ( Boost 1.46 REQUIRED COMPONENTS system regex thread )

link_directories ( ${Boost_LIBRARY_DIRS} )
include_directories ( ${Boost_INCLUDE_DIRS} )

add_subdirectory ( src )
