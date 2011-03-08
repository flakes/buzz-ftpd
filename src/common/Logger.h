//
// Logger.h - part of buzz-ftpd
// Copyright (c) 2011, cxxjoe
// Please refer to the LICENSE file for details.
//

#ifndef _LOGGER_H
#define _LOGGER_H

#include "buzzconfig.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <queue>

/**
 * Provides a thread-safe non-blocking logging facility.
 **/
class CLogger :
	private boost::noncopyable
{
	public:
		// constructor: component will be used for the file name,
		// path must the be the path to the logs folder.
		CLogger(const std::string& a_component, const std::string& a_path,
			size_t a_inMemoryHistorySize = 100);
		~CLogger();

		// starts a thread and readies instance for logging
		bool Start();
		// stops the thread and cleans up
		bool Stop();

		// different log levels
		typedef enum
		{
			LL_DEBUG = 255,
			LL_INFO = 128,
			LL_WARNING = 64,
			LL_ERROR = 32,
			LL_CRITICAL = 16
		} ELogLevel;

		// sets a log level. LL_DEBUG includes all lower levels.
		void SetMinLevel(int a_mask) { m_minLogLevel = a_mask; }

		// adds a new entry to the log, non-blocking unless
		// a_level is LL_CRITICAL which will force an immediate
		// blocking write.
		void Log(ELogLevel a_level, const std::string& a_message);

	protected:
		// running or not?
		bool m_stopped;
		// timestamp:
		boost::posix_time::ptime m_startTime;
		// current min logging level:
		int m_minLogLevel;

		// component name:
		std::string m_component;
		// complete path to the log file:
		std::string m_filePath;

		// used for synchronization of the two following members:
		boost::mutex m_lock;
		// every log entry is assigned a unique id:
		uint64_t m_logEntryId;
		// queue of entries waiting to be Commit()ed to the disk:
		std::queue<std::string> m_queue;

		// number of last log entries to keep in memory:
		size_t m_historySize;
		// aforementioned in-memory log. Please note that
		// this only covers disk-committed entries, not pending ones.
		std::queue<std::string> m_history;

		// commits all pending entries in m_queue to the disk/file.
		bool Commit();

		// io_service for our internal commit timer operations:
		boost::asio::io_service m_ioService;
		// io_service/file access thread:
		boost::shared_ptr<boost::thread> m_thread;
		// the commit timer:
		boost::asio::deadline_timer m_timer;
		// commit timer interval in seconds:
		int m_commitInterval;
		// timer handler:
		void OnTimer(const boost::system::error_code& e);
		// used to safely stop the timer and therefore the thread and io_service:
		void OnStop();
};

#endif /* !_LOGGER_H */
