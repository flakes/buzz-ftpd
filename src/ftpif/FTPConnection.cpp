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
	m_lineBuf(4096)
{

}


void CFTPConnection::Start()
{
	CAsioSmartBuffer l_helloBuf("220 Hello\r\n");

	boost::asio::async_write(m_socket, l_helloBuf,
		boost::bind(&CFTPConnection::OnWrite, shared_from_this(),
		boost::asio::placeholders::error));

	boost::asio::async_read_until(m_socket, m_lineBuf, "\r\n",
		boost::bind(&CFTPConnection::OnRead, shared_from_this(),
		boost::asio::placeholders::error));
}


void CFTPConnection::Stop()
{
}


void CFTPConnection::OnRead(const boost::system::error_code& e)
{
}


void CFTPConnection::OnWrite(const boost::system::error_code& e)
{
}


/**********************************************************
   FTP Interpreter Implementation
**********************************************************/

/*virtual*/ void CFTPConnection::FTPSend(int a_status, const std::string& a_response)
{
	CAsioSmartBuffer l_buf(lexical_cast<std::string>(a_status) + " " + a_response + "\r\n");

	boost::asio::async_write(m_socket, l_buf,
		boost::bind(&CFTPConnection::OnWrite, shared_from_this(),
		boost::asio::placeholders::error));
}

/*virtual*/ void CFTPConnection::FTPDisconnect()
{
	m_socket.close();
}


/*virtual*/ bool CFTPConnection::OnAuth(const std::string& a_method)
{
	return true;
}


/*virtual*/ int32_t CFTPConnection::OnPBSZ(int32_t a_size)
{
	return 100;
}


/*virtual*/ bool CFTPConnection::OnUser(const std::string& a_name)
{
	return true;
}


/*virtual*/ void CFTPConnection::OnPassword(const std::string& a_password)
{
	return;
}


/*virtual*/ void CFTPConnection::OnQuit(std::string& ar_message)
{
	return;
}
