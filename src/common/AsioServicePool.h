//
// AsioServicePool.h - part of buzz-ftpd
// Copyright (c) 2011, cxxjoe
// Please refer to the LICENSE file for details.
//
// Based on:
// io_service_pool.hpp
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//

#ifndef _ASIO_SERVICE_POOL_H
#define _ASIO_SERVICE_POOL_H

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <vector>

/**
 * Provides a pool of io_service objects.
 **/
class CAsioServicePool :
	private boost::noncopyable
{
	public:
		CAsioServicePool(size_t a_poolSize);

		// Starts all io_services in the pool and blocks until stopped:
		void Run();

		// Stops all io_services in the pool:
		void Stop();

		// Returns a ready-to-use io_service:
		boost::asio::io_service& GetIOService();

	protected:
		typedef boost::shared_ptr<boost::asio::io_service> PIOService;
		typedef boost::shared_ptr<boost::asio::io_service::work> PIOWork;

		// The pool of io_services:
		std::vector<PIOService> m_ioServices;

		// The work that keeps the io_services alive:
		std::vector<PIOWork> m_ioWork;

		// Index of the next io_service that GetIOService returns:
		size_t m_nextIndex;
};

typedef boost::shared_ptr<CAsioServicePool> PAsioServicePool;

#endif /* !_ASIO_SERVICE_POOL_H */
