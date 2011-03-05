//
// AsioServicePool.cpp - part of buzz-ftpd
// Copyright (c) 2011, cxxjoe
// Please refer to the LICENSE file for details.
//
// Based on:
// io_service_pool.cpp
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//

#include "AsioServicePool.h"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <stdexcept>


CAsioServicePool::CAsioServicePool(size_t a_poolSize)
	: m_nextIndex(0)
{
	if(a_poolSize < 1)
	{
		throw new std::runtime_error("CAsioServicePool: pool size is zero.");
	}

	// Give all the io_services work to do so that their run() functions will not
	// exit until they are explicitly stopped.
	for(size_t i = 0; i < a_poolSize; i++)
	{
		PIOService l_ioService(new boost::asio::io_service());
		m_ioServices.push_back(l_ioService);

		PIOWork l_ioWork(new boost::asio::io_service::work(*l_ioService));
		m_ioWork.push_back(l_ioWork);
	}
}


void CAsioServicePool::Run()
{
	typedef boost::shared_ptr<boost::thread> PThread;

	// Create a pool of threads to run all of the io_services.
	std::vector<PThread> l_threads;
	for(size_t i = 0; i < m_ioServices.size(); i++)
	{
		PThread l_thread(new boost::thread(
			boost::bind(&boost::asio::io_service::run, m_ioServices[i])));

		l_threads.push_back(l_thread);
	}

	// Wait for all threads in the pool to exit.
	for(size_t i = 0; i < l_threads.size(); i++)
	{
		l_threads[i]->join();
	}
}


void CAsioServicePool::Stop()
{
	// Explicitly stop all io_services.
	for(size_t i = 0; i < m_ioServices.size(); i++)
	{
		m_ioServices[i]->stop();
	}
}


boost::asio::io_service& CAsioServicePool::GetIOService()
{
	// Use a round-robin scheme to choose the next io_service to use.
	boost::asio::io_service& l_ioService = *m_ioServices[m_nextIndex];

	m_nextIndex++;
	if(m_nextIndex == m_ioServices.size())
	{
		m_nextIndex = 0;
	}

	return l_ioService;
}
