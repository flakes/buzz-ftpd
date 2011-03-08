//
// BuzzLogger.h - part of buzz-ftpd
// Copyright (c) 2011, cxxjoe
// Please refer to the LICENSE file for details.
//

#ifndef _BUZZ_LOGGER_H
#define _BUZZ_LOGGER_H

#include "buzzconfig.h"
#include "Logger.h"
#include <boost/shared_ptr.hpp>
#include <sstream>

// needs to be instanstiated in main():
extern boost::shared_ptr<CLogger> _s_BuzzLogger;

// use the BUZZ_LOG and BUZZ_LOG_THIS macros from everywhere!

// ugly preprocessor magic below this line.

#ifdef _DEBUG
	#define BUZZ_LOG(LEVEL, ARGS) _BUZZLOG(LEVEL, ARGS)
#else
	// allow compiler to optimize away DEBUG log messages:
	#define BUZZ_LOG(LEVEL, ARGS) if(LEVEL != CLogger::LL_DEBUG) { _BUZZLOG(LEVEL, ARGS); }
#endif

#define _BUZZ_LOG(LEVEL, ARGS) do { \
		std::stringstream _s_log; \
		_s_log << ARGS; \
		::_s_BuzzLogger->Log(LEVEL, _s_log.str()); \
	} while(0)

#define BUZZ_LOG_THIS(LEVEL, ARGS) \
	BUZZ_LOG(LEVEL, "[" << static_cast<void*>(this) << "] " << ARGS)

#endif /* !_BUZZ_LOGGER_H */
