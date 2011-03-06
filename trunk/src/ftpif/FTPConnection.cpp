//
// FTPConnection.cpp - part of buzz-ftpd
// Copyright (c) 2011, cxxjoe
// Please refer to the LICENSE file for details.
//

#include "BuzzFTPInterface.h"
#include "FTPConnection.h"
#include "AsioSmartBuffer.h"


CFTPConnection::CFTPConnection(boost::asio::io_service& a_ioService)
	: m_ioService(a_ioService),
	m_socket(m_ioService),
	m_lineBuf(FTP_MAX_LINE + 1),
	m_sslActive(false)
{

}


/**
 * Called from CFTPListener after the connection has been accept()ed.
 **/
void CFTPConnection::Start()
{
	CFTPInterpreter::FeedConnect();

	_ReadLineAsync();
}


void CFTPConnection::Stop()
{
}


/**
 * Called by asio internals when a line has been read.
 **/
void CFTPConnection::OnRead(const boost::system::error_code& e)
{
	if(!e)
	{
		// use an istream...
		std::istream l_stream(&m_lineBuf);
		// to extract the first (should be the only too) line from the buffer:
		std::string l_line;
		std::getline(l_stream, l_line);

		// remove whitespace:
		boost::algorithm::trim(l_line);
		// and feed it into the state machine:
		if(CFTPInterpreter::FeedLine(l_line))
		{
			// :TODO: define what FeedLine's return value actually means
			_ReadLineAsync();
		}
	}
	else
	{
		// :TODO:
	}

	// if this method doesn't fire up a new async read or write,
	// or generally speaking, if no async reads/writes are pending,
	// the class will be destroyed automatically!
}


/**
 * Called by asio internals upon completition of an async_write call.
 **/
void CFTPConnection::OnWrite(const boost::system::error_code& e)
{
	if(e)
	{
		// :TODO:
		return;
	}

	if(m_sslHandshakeAfterNextWrite)
	{
		m_sslHandshakeAfterNextWrite = false;

		m_sslSocket = boost::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket&> >(
			new boost::asio::ssl::stream<boost::asio::ip::tcp::socket&>(m_socket, *m_sslCtx));

		m_sslSocket->async_handshake(boost::asio::ssl::stream_base::server,
			boost::bind(&CFTPConnection::OnShookHands, shared_from_this(),
			boost::asio::placeholders::error));
	}
}


void CFTPConnection::OnShookHands(const boost::system::error_code& e)
{
	// :TODO: try to differentiate FeedSSLHandshake arguments
	// according to RFC 4217, section 10.1.

	if(!e)
	{
		m_sslActive = true;

		CFTPInterpreter::FeedSSLHandshake(true, true);

		_ReadLineAsync();
	}
	else
	{
		CFTPInterpreter::FeedSSLHandshake(false, false);

		// control connection will effectively be terminated here.
	}
}


void CFTPConnection::_ReadLineAsync()
{
	if(!m_sslActive)
	{
		boost::asio::async_read_until(m_socket, m_lineBuf, "\n",
			boost::bind(&CFTPConnection::OnRead, shared_from_this(),
			boost::asio::placeholders::error));
	}
	else
	{
		boost::asio::async_read_until(*m_sslSocket, m_lineBuf, "\n",
			boost::bind(&CFTPConnection::OnRead, shared_from_this(),
			boost::asio::placeholders::error));
	}
}


/**********************************************************
   FTP Interpreter Implementation
**********************************************************/

void CFTPConnection::FTPSend(int a_status, const std::string& a_response)
{
	CAsioSmartBuffer l_buf(a_response);

	if(!m_sslActive)
	{
		boost::asio::async_write(m_socket, l_buf,
			boost::bind(&CFTPConnection::OnWrite, shared_from_this(),
			boost::asio::placeholders::error));
	}
	else
	{
		boost::asio::async_write(*m_sslSocket, l_buf,
			boost::bind(&CFTPConnection::OnWrite, shared_from_this(),
			boost::asio::placeholders::error));
	}
}


void CFTPConnection::FTPDisconnect()
{
	m_socket.close();
}


bool CFTPConnection::OnAuth(const std::string& a_method)
{
	if(a_method == "SSL" || a_method == "TLS" || a_method == "TLS-C")
	{
		m_sslHandshakeAfterNextWrite = true;

		m_sslCtx = boost::shared_ptr<boost::asio::ssl::context>(
			new boost::asio::ssl::context(m_ioService,
			(a_method == "SSL" ? boost::asio::ssl::context::sslv23 : boost::asio::ssl::context::tlsv1))
		);

		m_sslCtx->set_options(boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::no_sslv2);

		// :TODO: paths must be configurable
		m_sslCtx->use_certificate_chain_file("server.pem");
		m_sslCtx->use_private_key_file("server.pem", boost::asio::ssl::context::pem);

		return true;
	}

	return false;
}


uint32_t CFTPConnection::OnPBSZ(uint32_t a_size)
{
	// SSL/TLS = streaming protocol, no buffer/block size required
	// RFC 4217, section 9
	return 0;
}


bool CFTPConnection::OnUser(const std::string& a_name)
{
	return true;
}


void CFTPConnection::OnPassword(const std::string& a_password)
{
	FeedCredentialResult(true, "H\n E\n  L\n   L\n    O");
}


void CFTPConnection::OnQuit(std::string& ar_message)
{
	return;
}
