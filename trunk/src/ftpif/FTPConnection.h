//
// FTPConnection.h - part of buzz-ftpd
// Copyright (c) 2011, cxxjoe
// Please refer to the LICENSE file for details.
//

#ifndef _FTP_CONNECTION_H
#define _FTP_CONNECTION_H

#include "FTPInterpreter.h"

class CFTPListener;

/**
 * represents a single FTP client connection.
 **/
class CFTPConnection :
	public boost::enable_shared_from_this<CFTPConnection>,
	private boost::noncopyable,
	public CFTPInterpreter
{
	public:
		CFTPConnection(boost::asio::io_service& a_ioService);
		virtual ~CFTPConnection();

		void Start();
		void Stop();

		boost::asio::ip::tcp::socket& GetSocket() { return m_socket; }

	protected:
		boost::asio::io_service& m_ioService;
		boost::asio::ip::tcp::socket m_socket;
		boost::asio::streambuf m_lineBuf;

		void OnRead(const boost::system::error_code& e);
		void OnWrite(const boost::system::error_code& e);
		void OnShookHands(const boost::system::error_code& e);

		/** CFTPInterpreter implementation **/
		virtual void FTPSend(int, const std::string&);
		virtual void FTPDisconnect();
		virtual bool OnAuth(const std::string&);
		virtual uint32_t OnPBSZ(uint32_t);
		virtual bool OnUser(const std::string&);
		virtual void OnPassword(const std::string&);
		virtual void OnQuit(std::string&);

		/** SSL stuff **/
		bool m_sslHandshakeAfterNextWrite;
		boost::shared_ptr<boost::asio::ssl::context> m_sslCtx;
		boost::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket&> > m_sslSocket;
		bool m_sslActive;

		/** internal helpers **/
		void _ReadLineAsync();
};

typedef boost::shared_ptr<CFTPConnection> PFTPConnection;

#endif /* !_FTP_CONNECTION_H */
