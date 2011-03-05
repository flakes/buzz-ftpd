//
// FTPInterpreter.cpp - part of buzz-ftpd
// Copyright (c) 2011, cxxjoe
// Please refer to the LICENSE file for details.
//

#include "BuzzFTPInterface.h"
#include "FTPInterpreter.h"

using std::string;


CFTPInterpreter::CFTPInterpreter()
	: m_state(FTIST_INITIAL),
	m_pbsz(0),
	m_secureCC(false),
	m_prot('C')
{
	m_ccHandlers["AUTH"] = &CFTPInterpreter::_CC_AUTH;
	m_ccHandlers["FEAT"] = &CFTPInterpreter::_CC_FEAT;
	m_ccHandlers["SYST"] = &CFTPInterpreter::_CC_SYST;
	m_ccHandlers["NOOP"] = &CFTPInterpreter::_CC_NOOP;

	/* 502 */
	m_ccHandlers["CCC"] = &CFTPInterpreter::_CC_NotImplemented;
}


void CFTPInterpreter::FTPResponse(int a_status, const std::string& a_text)
{
	bool l_multiline = (a_text.find('\n') != string::npos);

	if(!l_multiline)
	{
		this->FTPSend(a_status, lexical_cast<string>(a_status) + " " + a_text + "\r\n");
	}
	else
	{
		// multi-line response formatted according to RFC 959, section 4.2.
		std::string l_response =
			lexical_cast<string>(a_status) + "-";

		bool l_firstLine = true;
		std::stringstream l_ss(a_text);
		string l_line;

		// iterate over lines:
		while(std::getline(l_ss, l_line))
		{
			if(l_firstLine)
				l_firstLine = false;
			else
				l_response += "  ";

			l_response += l_line + "\r\n";
		}

		// add "end bracket" line:
		l_response += lexical_cast<string>(a_status) + " End.\r\n";

		this->FTPSend(a_status, l_response);
	}
}


void CFTPInterpreter::FeedConnect()
{
	this->FTPResponse(220, "Ready.");
}


bool CFTPInterpreter::FeedLine(const string& a_line)
{
	// check maximum line length:
	if(a_line.size() >= FTP_MAX_LINE + 1)
	{
		this->FTPResponse(500, "Line too long");
		return false;
	}

	// extract FTP command and a possible space-delimited argument:
	string l_cmdWord, l_cmdArgs;
	string::size_type l_spPos = a_line.find(' ');

	l_cmdWord = a_line.substr(0, l_spPos);
	boost::algorithm::to_upper(l_cmdWord);

	if(l_spPos != string::npos)
	{
		l_cmdArgs = a_line.substr(l_spPos + 1);
		boost::algorithm::trim_left(l_cmdArgs);
	}

	// now act upon the received command!

	if(!m_secureCC &&
		l_cmdWord != "AUTH" && l_cmdWord != "SYST" && l_cmdWord != "FEAT")
	{
		this->FTPResponse(533, "Secure connection required.");
		return false;
	}

	// branch based on the map we set up in the constructor:

	TClientCommandMap::const_iterator l_method = m_ccHandlers.find(l_cmdWord);

	if(l_method != m_ccHandlers.end())
	{
		// wow:
		return (this->*(l_method->second))(l_cmdWord, l_cmdArgs);
	}
	/*else if()
	{
	}*/
	else
	{
		// :TODO: watch out for 500 vs. 202/502
		this->FTPResponse(500, "Command not understood.");
	}

	return true;
}


/**
 * CLIENT COMMAND: AUTH <MECHANISM>
 * RFC 2228 / RFC 4217
 **/
bool CFTPInterpreter::_CC_AUTH(const std::string& a_cmd, const std::string& a_args)
{
	if(m_state != FTIST_INITIAL)
	{
		this->FTPResponse(503, "Bad sequence of commands.");
		return false;
	}

	std::string l_upperArgs(a_args);
	boost::algorithm::to_upper(l_upperArgs);

	if(a_args.empty())
	{
		this->FTPResponse(501, "Missing argument.");
	}
	else if(this->OnAuth(l_upperArgs))
	{
		m_state = FTIST_GOT_AUTH;

		this->FTPResponse(234, "Proceeding with handshake.");

		// prevent implementing class from trying further reads
		// on the clear text socket:
		return false;
	}
	else
	{
		this->FTPResponse(504, "Method not implemented.");
	}

	return true;
}


/**
 * CLIENT COMMAND: NOOP
 * RFC 959
 **/
bool CFTPInterpreter::_CC_NOOP(const std::string&, const std::string&)
{
	this->FTPResponse(200, "Idle hands are the Devil's plaything.");
	return true;
}


/**
 * CLIENT COMMAND: SYST
 * RFC 959
 **/
bool CFTPInterpreter::_CC_SYST(const std::string&, const std::string&)
{
	this->FTPResponse(215, "UNIX");
	return true;
}


/**
 * CLIENT COMMAND: FEAT
 * RFC 959
 **/
bool CFTPInterpreter::_CC_FEAT(const std::string&, const std::string&)
{
	this->FTPResponse(211,
		"Extensions supported:\n"
		/* RFC 2228 / RFC 4217 */
		"AUTH SSL\n"
		"AUTH TLS\n"
		"PBSZ\n"
		"PROT\n"
		/* drftpd */
		"PRET\n"
		/* Miscellaneous */
		"NOOP"
		// avoid trailing newline
	);
	return true;
}


/**
 * Misc client commands that return "502 Not Implemented."
 * RFC 959
 **/
bool CFTPInterpreter::_CC_NotImplemented(const std::string&, const std::string&)
{
	this->FTPResponse(502, "Not Implemented.");
	return true;
}

