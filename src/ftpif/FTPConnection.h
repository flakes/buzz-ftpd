
#ifndef _FTP_CONNECTION_H
#define _FTP_CONNECTION_H

#include "FTPInterpreter.h"

class CFTPListener;

/**
 * represents a single FTP client connection.
 **/
class CFTPConnection :
	public boost::enable_shared_from_this<CFTPConnection>,
	private boost::noncopyable,
	public CFTPInterpreter
{
	public:
		CFTPConnection(boost::asio::io_service& a_ioService);

		void Start();
		void Stop();

		boost::asio::ip::tcp::socket& GetSocket() { return m_socket; }

	protected:
		boost::asio::io_service& m_ioService;
		boost::asio::ip::tcp::socket m_socket;
		boost::asio::streambuf m_lineBuf;

		void OnRead(const boost::system::error_code& e);
		void OnWrite(const boost::system::error_code& e);

		/** CFTPInterpreter implementation **/
		virtual void FTPSend(int, const std::string&);
		virtual void FTPDisconnect();
		virtual bool OnAuth(const std::string&);
		virtual int32_t OnPBSZ(int32_t);
		virtual bool OnUser(const std::string&);
		virtual void OnPassword(const std::string&);
		virtual void OnQuit(std::string&);
};

typedef boost::shared_ptr<CFTPConnection> PFTPConnection;

#endif /* !_FTP_CONNECTION_H */
