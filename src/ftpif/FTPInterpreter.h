//
// FTPInterpreter.h - part of buzz-ftpd
// Copyright (c) 2011, cxxjoe
// Please refer to the LICENSE file for details.
//

#ifndef _FTP_INTERPRETER_H
#define _FTP_INTERPRETER_H


typedef enum _FTP_INTERPRETER_STATES
{
	FTIST_INITIAL = 1,
	FTIST_GOT_AUTH,
	FTIST_GOT_USER,
	FTIST_GOT_PASS,
	//...
	FTIST_READY,
	//...
} ftp_interpreter_state_t;


/**
 * A validating server-side FTP parser and state machine. Provides events to derived classes.
 * It assumes/establishes a few things:
 * - Only valid UTF-8 strings will be accepted. There are no exceptions.
 * - The only command that will be accepted without an established SSL/TLS connection is AUTH.
 **/
class CFTPInterpreter
{
	protected:
		CFTPInterpreter();


		/**
		 * Called by the implementing class when a new TCP connection has been accepted
		 * by the listener and connected.
		 **/
		void FeedConnect();

		/**
		 * Called by the implementing class when a new (complete) line comes
		 * in via the control connection.
		 **/
		bool FeedLine(const std::string& a_line);

		/**
		 * Called by this class when a response needs to be sent back to the client.
		 * Be aware that a_response may contain CRLFs! a_status is just for informational
		 * purposes and is not to be incorporated into the response in any way.
		 **/
		virtual void FTPSend(int a_status, const std::string& a_response) = 0;

		/**
		 * Called by this class when a critical errors occurs (such as a malformed request).
		 * It is recommended to terminate the connection when this happens.
		 **/
		virtual void FTPDisconnect() = 0; 


		// actual FTP events below this line.

		/**
		 * The implementing class must return whether a_method defines an acceptable
		 * authentication mechanism. Will be SSL, TLS or TLS-C most of the time.
		 **/
		virtual bool OnAuth(const std::string& a_method) = 0;

		/**
		 * The implementing class must return a_size, or, if a_size is too large,
		 * an alternative (lower) protection buffer size.
		 **/
		virtual int32_t OnPBSZ(int32_t a_size) = 0;

		/**
		 * The implementing class must return whether the user name can be accepted.
		 * It is a good idea to always return true unless the name contains invalid
		 * characters. The actual credentials check should take place once the password
		 * has been sent (as suggested by RFC 2577).
		 **/
		virtual bool OnUser(const std::string& a_name) = 0;

		/**
		 * After receiving this event, the implementing class must validate
		 * user name and password and call FeedAuthResult upon completition.
		 **/
		virtual void OnPassword(const std::string& a_password) = 0;

    /**
     * Causes the interpreter to take appropiate action based on whether
     * the client provided valid credentials.
     * @see OnPassword
     **/
    void FeedAuthResult(bool a_positive);


		/**
		 * A QUIT from the client will effectively terminate the connection.
		 * The implementing class is free to store a goodbye message in ar_message.
		 **/
		virtual void OnQuit(std::string& ar_message) = 0;


		// interpreter-internal stuff below this line.
	private:
		// state data:
		ftp_interpreter_state_t m_state;

		int32_t m_pbsz;
		bool m_secureCC;
		char m_prot; // C or P

		// method map:
		typedef std::map<const std::string, bool (CFTPInterpreter::*)(const std::string& a_cmd, const std::string& a_args)> TClientCommandMap;
		TClientCommandMap m_ccHandlers;

		// client command handlers to be used with aforementioned map:
		bool _CC_AUTH(const std::string&, const std::string&);
		bool _CC_FEAT(const std::string&, const std::string&);
		bool _CC_SYST(const std::string&, const std::string&);
		bool _CC_NOOP(const std::string&, const std::string&);

		bool _CC_NotImplemented(const std::string&, const std::string&);

		// utility methods:
		void FTPResponse(int a_status, const std::string& a_text);
};


#endif /* !_FTP_INTERPRETER_H */
