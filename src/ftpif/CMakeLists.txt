
include_directories ( ${BUZZFTPD_SOURCE_DIR}/src/common )
include_directories ( ${BUZZFTPD_SOURCE_DIR}/src/ftpif )

add_executable (
	buzz-ftpif
	ftpif-main.cpp
	FTPListener.cpp
	FTPConnection.cpp
	FTPInterpreter.cpp
	${BUZZFTPD_SOURCE_DIR}/src/common/AsioServicePool.cpp
)

target_link_libraries (
	buzz-ftpif
	${Boost_LIBRARIES}
)
