//
// Logger.cpp - part of buzz-ftpd
// Copyright (c) 2011, cxxjoe
// Please refer to the LICENSE file for details.
//

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include "Logger.h"
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <fstream>

namespace fs = boost::filesystem;


CLogger::CLogger(const std::string& a_component, const std::string& a_path, size_t a_inMemoryHistorySize)
	: m_stopped(true),
	m_component(a_component),
	m_logEntryId(0),
	m_historySize(a_inMemoryHistorySize),
	m_timer(m_ioService),
	m_commitInterval(2),
	m_minLogLevel(LL_DEBUG)
{
	fs::path l_path = fs::absolute(a_component + ".log", fs::path(a_path));

	// avoid need for filesystem.h in Logger.h ...
	m_filePath = l_path.generic_string();
}


bool CLogger::Start()
{
	if(!m_stopped)
	{
		return false;
	}

	m_startTime = boost::posix_time::microsec_clock::local_time();

	m_thread = boost::shared_ptr<boost::thread>(
		new boost::thread(boost::bind(&boost::asio::io_service::run, &m_ioService)));

	m_timer.expires_from_now(boost::posix_time::seconds(m_commitInterval));
	m_timer.async_wait(boost::bind(&CLogger::OnTimer, this, boost::asio::placeholders::error));

	m_stopped = false;

	Log(LL_INFO, "Logging started!");

	return true;
}


void CLogger::Log(ELogLevel a_level, const std::string& a_message)
{
	if((m_minLogLevel & (int)a_level) == 0)
	{
		return;
	}

	boost::posix_time::time_duration l_timestamp =
		(boost::posix_time::microsec_clock::local_time() - m_startTime);

	std::string l_entry = boost::str(
		boost::format("%08d [%08x] ")
		% l_timestamp.total_milliseconds()
		% boost::this_thread::get_id())
		+ a_message;

	m_lock.lock();

	uint64_t l_logId = (m_logEntryId++);

	m_queue.push(boost::str(boost::format("%08d ") % l_logId) + l_entry);

	if(a_level == LL_CRITICAL)
	{
		Commit();
	}

	m_lock.unlock();
}


void CLogger::OnTimer(const boost::system::error_code& e)
{
	if(e == boost::asio::error::operation_aborted)
	{
		return;
	}

	if(m_lock.try_lock())
	{
		Commit();

		m_lock.unlock();
	}

	m_timer.expires_from_now(boost::posix_time::seconds(m_commitInterval));
	m_timer.async_wait(boost::bind(&CLogger::OnTimer, this, boost::asio::placeholders::error));
}


// this method assumes that m_lock is currently locked by the calling code!
bool CLogger::Commit()
{
	if(m_queue.empty())
	{
		return true;
	}

	std::ofstream l_file;
	l_file.open(m_filePath.c_str(), std::ios_base::app);

	if(l_file.is_open())
	{
		while(!m_queue.empty())
		{
			std::string l_entry = m_queue.front();

			boost::algorithm::replace_all(l_entry, "\r", "\\r");
			boost::algorithm::replace_all(l_entry, "\n", "\\n");

			l_file << l_entry << std::endl;

			// update in-memory history:
			m_history.push(l_entry);
			while(m_history.size() > m_historySize) m_history.pop();

			m_queue.pop();
		}

		l_file.close();
		return true;
	}

	return false;
}


bool CLogger::Stop()
{
	if(m_stopped)
	{
		return false;
	}

	Log(LL_INFO, "Logging stopping...");

	// synchronize into service thread:
	m_ioService.post(boost::bind(&CLogger::OnStop, this));

	// :TODO: find out if this works as expected if Start()
	// is called immediately after returning from Stop():
	m_stopped = true;

	return true;
}


void CLogger::OnStop()
{
	m_timer.cancel();
	m_timer.wait(); // :TODO: this should work, right?

	Log(LL_INFO, "Logging stopped!");

	Commit();

	m_ioService.stop();
}


CLogger::~CLogger()
{
	// make sure the thread etc. is terminated cleanly before this instance goes away:
	Stop();
	m_ioService.run(); // wait for OnStop to finish, will return instantly when there's nothing left to do.
}
