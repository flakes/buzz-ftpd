//
// FTPInterpreter.cpp - part of buzz-ftpd
// Copyright (c) 2011, cxxjoe
// Please refer to the LICENSE file for details.
//

#include "BuzzFTPInterface.h"
#include "FTPInterpreter.h"

using std::string;
using std::stringstream;


CFTPInterpreter::CFTPInterpreter()
	: m_state(FTIST_INITIAL),
	m_pbsz(0), // defaulting to 0 = RFC violation :TODO:
	m_secureCC(false),
	m_authenticated(false),
	m_prot('C')
{
	m_ccHandlers["AUTH"] = &CFTPInterpreter::_CC_AUTH;
	m_ccHandlers["FEAT"] = &CFTPInterpreter::_CC_FEAT;
	m_ccHandlers["SYST"] = &CFTPInterpreter::_CC_SYST;
	m_ccHandlers["NOOP"] = &CFTPInterpreter::_CC_NOOP;
	m_ccHandlers["PBSZ"] = &CFTPInterpreter::_CC_PBSZ;
	m_ccHandlers["USER"] = &CFTPInterpreter::_CC_USER;
	m_ccHandlers["PASS"] = &CFTPInterpreter::_CC_PASS;
	m_ccHandlers["QUIT"] = &CFTPInterpreter::_CC_QUIT;

	/* 502 */
	m_ccHandlers["CCC"] = &CFTPInterpreter::_CC_NotImplemented;
}


void CFTPInterpreter::FTPResponse(int a_status, const string& a_text)
{
	bool l_multiline = (a_text.find('\n') != string::npos);

	if(!l_multiline)
	{
		this->FTPSend(a_status, lexical_cast<string>(a_status) + " " + a_text + "\r\n");
	}
	else
	{
		// multi-line response formatted according to RFC 959, section 4.2.
		string l_response =
			lexical_cast<string>(a_status) + "-";

		bool l_firstLine = true;
		stringstream l_ss(a_text);
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

	// now act upon the received command:
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
bool CFTPInterpreter::_CC_AUTH(const string& a_cmd, const string& a_args)
{
	if(m_state != FTIST_INITIAL)
	{
		this->FTPResponse(503, "Bad sequence of commands.");
		return false;
	}

	string l_upperArgs(a_args);
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
		// now awaiting call to FeedSSLHandshake.
	}
	else
	{
		this->FTPResponse(504, "Method not implemented.");
	}

	return true;
}


/**
 * 
 **/
void CFTPInterpreter::FeedSSLHandshake(bool a_secured, bool a_ok)
{
	BOOST_ASSERT(m_state == FTIST_GOT_AUTH);

	// :TODO: improve according to RFC 4217, section 10.1.
	// @see CFTPConnection::OnShookHands
	if(a_secured && a_ok)
	{
		m_secureCC = true;
		m_state = FTIST_AUTH_SUCCESSFUL;
		// send nothing, we're fine.
	}
	else if(!a_secured)
	{
		if(a_ok)
		{
			this->FTPResponse(421, "Handshake failed, terminating connection.");
		}

		this->FTPDisconnect();
	}
}


/**
 * CLIENT COMMAND: NOOP
 * RFC 959
 **/
bool CFTPInterpreter::_CC_NOOP(const string& a_cmd, const string& a_args)
{
	this->FTPResponse(200, "Idle hands are the Devil's plaything.");
	return true;
}


/**
 * CLIENT COMMAND: SYST
 * RFC 959
 **/
bool CFTPInterpreter::_CC_SYST(const string& a_cmd, const string& a_args)
{
	this->FTPResponse(215, "UNIX");
	return true;
}


/**
 * CLIENT COMMAND: FEAT
 * RFC 959
 **/
bool CFTPInterpreter::_CC_FEAT(const string& a_cmd, const string& a_args)
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
 * CLIENT COMMAND: PBSZ <SIZE>
 * RFC 2228
 **/
bool CFTPInterpreter::_CC_PBSZ(const string& a_cmd, const string& a_args)
{
	if(!m_secureCC)
	{
		this->FTPResponse(503, "Bad sequence of commands.");
		return false;
	}

	uint32_t l_clientSize = 0;

	try
	{
		l_clientSize = numeric_cast<uint32_t>(lexical_cast<int64_t>(a_args));
	}
	catch(std::exception& e)
	{
		this->FTPResponse(501, "Invalid size argument.");
		return false;
	}

	uint32_t l_serverSize = this->OnPBSZ(l_clientSize);

	if(l_serverSize < l_clientSize)
	{
		this->FTPResponse(200, "Actual PBSZ=" + lexical_cast<string>(l_serverSize));
		m_pbsz = l_serverSize;
	}
	else
	{
		this->FTPResponse(200, "Accepted.");
		m_pbsz = l_clientSize;
	}

	return true;
}


/**
 * CLIENT COMMAND: USER
 * RFC 959
 **/
bool CFTPInterpreter::_CC_USER(const string& a_cmd, const string& a_args)
{
	if(m_state != FTIST_AUTH_SUCCESSFUL)
	{
		this->FTPResponse(503, "Bad sequence of commands.");
	}

	if(!a_args.empty() && this->OnUser(a_args))
	{
		this->FTPResponse(331, "Awaiting password.");

		m_state = FTIST_GOT_USER;
		// now awaiting PASS command.
	}
	else
	{
		this->FTPResponse(530, "Invalid user name.");
	}

	return true;
}


/**
 * CLIENT COMMAND: PASS
 * RFC 959
 **/
bool CFTPInterpreter::_CC_PASS(const string& a_cmd, const string& a_args)
{
	if(!m_secureCC)
	{
		this->FTPResponse(533, "Secure connection required.");
		return false;
	}

	if(m_state != FTIST_GOT_USER)
	{
		this->FTPResponse(503, "Bad sequence of commands.");
	}
	else
	{
		// update state before calling OnPassword in case OnPassword is working synchronously:
		m_state = FTIST_GOT_PASS;

		this->OnPassword(a_args);

		// now awaiting call to FeedCredentialResult.
	}

	return true;
}


void CFTPInterpreter::FeedCredentialResult(bool a_positive, const string& a_banner)
{
	BOOST_ASSERT(m_state == FTIST_GOT_PASS);

	if(a_positive)
	{
		string l_response = "User logged in successfully.";

		if(!a_banner.empty())
		{
			l_response += "\n" + a_banner;
		}

		this->FTPResponse(230, l_response);

		m_state = FTIST_READY;
	}
	else
	{
		this->FTPResponse(530, "Credentials rejected.");
		this->FTPDisconnect();
	}
}


/**
 * Misc client commands that return "502 Not Implemented."
 * RFC 959
 **/
bool CFTPInterpreter::_CC_NotImplemented(const string& a_cmd, const string& a_args)
{
	this->FTPResponse(502, "Not Implemented.");
	return true;
}


/**
 * CLIENT COMMAND: QUIT
 * RFC 959
 **/
bool CFTPInterpreter::_CC_QUIT(const string& a_cmd, const string& a_args)
{
	std::string l_goodbyeMsg = "Goodbye.";

	this->OnQuit(l_goodbyeMsg);

	this->FTPResponse(221, l_goodbyeMsg);

	return false; // terminate connection
}
