//
// FTPListener.cpp
// Copyright (c) 2011, cxxjoe
// Please refer to the LICENSE file for details.
//

#include "BuzzFTPInterface.h"
#include "FTPListener.h"


CFTPListener::CFTPListener(PAsioServicePool a_ioServicePool)
	: m_ioServicePool(a_ioServicePool),
	m_acceptor(m_ioServicePool->GetIOService()),
	m_newConnection(new CFTPConnection(m_ioServicePool->GetIOService()))
{
	
}


bool CFTPListener::Listen(unsigned short a_port, const std::string& a_bindHost)
{
	boost::asio::ip::tcp::resolver l_resolver(m_acceptor.io_service());
	boost::asio::ip::tcp::resolver::query l_query(a_bindHost, lexical_cast<std::string>(a_port));
	boost::asio::ip::tcp::endpoint l_endpoint = *l_resolver.resolve(l_query);
	m_acceptor.open(l_endpoint.protocol());
	m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	m_acceptor.bind(l_endpoint);
	m_acceptor.listen();

	m_acceptor.async_accept(m_newConnection->GetSocket(),
		boost::bind(&CFTPListener::OnAccept, this,
		boost::asio::placeholders::error));

	return true; // :TODO: handle errors/exceptions...
}


void CFTPListener::OnAccept(const boost::system::error_code& e)
{
	if(!e)
	{
		m_newConnection->Start();

		// prepare next connection:
		m_newConnection.reset(new CFTPConnection(m_ioServicePool->GetIOService()));

		m_acceptor.async_accept(m_newConnection->GetSocket(),
			boost::bind(&CFTPListener::OnAccept, this,
			boost::asio::placeholders::error));
	}
	else
	{
		// :TODO:
	}
}


CFTPListener::~CFTPListener()
{
}
