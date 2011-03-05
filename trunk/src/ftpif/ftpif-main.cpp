//
// ftpif-main.cpp - part of buzz-ftpd
// Copyright (c) 2011, cxxjoe
// Please refer to the LICENSE file for details.
//

#include "BuzzFTPInterface.h"
#include "FTPListener.h"
#include "AsioServicePool.h"

int main(int argc, char** argv)
{
	PAsioServicePool l_threadPool(new CAsioServicePool(4));

	CFTPListener l_ftpListener(l_threadPool);

	l_ftpListener.Listen(15000, "localhost");

	l_threadPool->Run();

	return 0;
}
