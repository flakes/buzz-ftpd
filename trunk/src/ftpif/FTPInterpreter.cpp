//
// FTPInterpreter.cpp - part of buzz-ftpd
// Copyright (c) 2011, cxxjoe
// Please refer to the LICENSE file for details.
//

#include "BuzzFTPInterface.h"
#include "FTPInterpreter.h"


CFTPInterpreter::CFTPInterpreter()
	: m_state(FTIST_INITIAL),
	m_pbsz(0),
	m_secureCC(false),
	m_prot('C')
{
}
