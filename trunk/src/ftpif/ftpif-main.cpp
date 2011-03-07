//
// ftpif-main.cpp - part of buzz-ftpd
// Copyright (c) 2011, cxxjoe
// Please refer to the LICENSE file for details.
//

#include "BuzzFTPInterface.h"
#include "FTPListener.h"
#include "AsioServicePool.h"
#include "BuzzLogger.h"


int main(int argc, char** argv)
{
	// enable logging:
	_s_BuzzLogger = boost::shared_ptr<CLogger>(new CLogger("ftpif", "/tmp/logs"));
	_s_BuzzLogger->Start();

	// set up thread pool:
	PAsioServicePool l_threadPool(new CAsioServicePool(4));

	// initiate listener:
	CFTPListener l_ftpListener(l_threadPool);

	// start listener:
	l_ftpListener.Listen(15000, "0.0.0.0");

	// enter main event loop:
	l_threadPool->Run();

	// before exiting the program, terminate logger:
	_s_BuzzLogger->Stop();

	return 0;
}


/*global*/ boost::shared_ptr<CLogger> _s_BuzzLogger;
