
### Require CMake 2.6 or later ###

cmake_minimum_required ( VERSION 2.6 FATAL_ERROR )

project ( BUZZFTPD )

### Look for boost headers and libraries ###

find_package ( Boost 1.46 REQUIRED COMPONENTS system regex thread filesystem )

link_directories ( ${Boost_LIBRARY_DIRS} )
include_directories ( ${Boost_INCLUDE_DIRS} )

### Look for OpenSSL ###

include ( FindOpenSSL )

if ( NOT OPENSSL_FOUND )
	message ( FATAL_ERROR "Unable to locate OpenSSL." )
endif ( NOT OPENSSL_FOUND )

link_directories ( ${OPENSSL_LIBRARY_DIRS} )
include_directories ( ${OPENSSL_INCLUDE_DIR} )

### Descend! ###

add_subdirectory ( src )

### Display some info when all is done ###

message ( STATUS "" )
message ( STATUS "  System: ${CMAKE_SYSTEM}" )
message ( STATUS "  CPU: ${CMAKE_HOST_SYSTEM_PROCESSOR}" )
message ( STATUS "" )
