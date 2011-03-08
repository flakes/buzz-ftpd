
### Require CMake 2.6 or later ###

cmake_minimum_required ( VERSION 2.6 FATAL_ERROR )

project ( BUZZFTPD )

### Look for boost headers and libraries ###

# use static libs because
# - it's cleaner on Windows, binaries stay small too
# - it's possible to get rid of the boost folder after compiling on Linux (huge bins though)
set ( Boost_USE_STATIC_LIBS ON )

if ( NOT MSVC )
	# Static boost libs require linking to pthread on Linux
	set ( CMAKE_USE_PTHREADS_INIT 1 )
	include ( FindThreads )
endif ()

find_package ( Boost 1.46 REQUIRED COMPONENTS system regex thread filesystem )

link_directories ( ${Boost_LIBRARY_DIRS} )
include_directories ( ${Boost_INCLUDE_DIRS} )

set ( Boost_LIBRARIES ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )

### Look for OpenSSL ###

# If OPENSSL_ROOT_DIR was defined in the environment, use it.
if ( NOT $ENV{OPENSSL_ROOT_DIR} STREQUAL "" )
	set ( OPENSSL_ROOT_DIR $ENV{OPENSSL_ROOT_DIR} )
endif ()

include ( FindOpenSSL )

# work around stupid/outdated FindOpenSSL on Windows:
if ( MSVC AND NOT OPENSSL_LIBRARIES )
	FIND_LIBRARY ( LIB_EAY NAMES libeay32 PATHS ${OPENSSL_ROOT_DIR}/lib )
	FIND_LIBRARY ( SSL_EAY NAMES ssleay32 PATHS ${OPENSSL_ROOT_DIR}/lib )
	set ( OPENSSL_LIBRARIES ${SSL_EAY} ${LIB_EAY} )

	if ( OPENSSL_LIBRARIES )
		message ( STATUS "Please ignore the previous OpenSSL not found-type message, it's fine." )
		set ( OPENSSL_FOUND 1 )
	endif ()
endif ()

if ( NOT OPENSSL_FOUND )
	message ( FATAL_ERROR "Unable to locate OpenSSL." )
endif ()

link_directories ( ${OPENSSL_LIBRARY_DIRS} )
include_directories ( ${OPENSSL_INCLUDE_DIR} )

### Create buzzconfig.h ###

if ( CMAKE_BUILD_TYPE STREQUAL "Debug" )
	set ( _DEBUG 1 )
endif ()

configure_file (
	"${BUZZFTPD_SOURCE_DIR}/buzzconfig.h.in"
	"${BUZZFTPD_BINARY_DIR}/buzzconfig.h"
)

include_directories ( ${BUZZFTPD_BINARY_DIR} )

### Descend! ###

add_subdirectory ( src )

### Display some info when all is done ###

message ( STATUS "" )
message ( STATUS "  System: ${CMAKE_SYSTEM}" )
message ( STATUS "  CPU: ${CMAKE_HOST_SYSTEM_PROCESSOR}" )
message ( STATUS "" )

