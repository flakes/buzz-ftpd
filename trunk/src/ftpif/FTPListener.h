//
// FTPListener.h - part of buzz-ftpd
// Copyright (c) 2011, cxxjoe
// Please refer to the LICENSE file for details.
//

#ifndef _FTP_LISTENER_H
#define _FTP_LISTENER_H

#include "FTPConnection.h"
#include "AsioServicePool.h"

/**
 * Accepts and manages incoming client connections.
 **/
class CFTPListener :
	private boost::noncopyable
{
	public:
		CFTPListener(PAsioServicePool a_ioServicePool);

		bool Listen(unsigned short a_port, const std::string& a_bindHost);

		~CFTPListener();

	protected:
		PAsioServicePool m_ioServicePool;
		boost::asio::ip::tcp::acceptor m_acceptor;

		PFTPConnection m_newConnection;

		void OnAccept(const boost::system::error_code& e);
};

#endif /* !_FTP_LISTENER_H */
