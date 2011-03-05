
#ifndef _FTP_CONNECTION_H
#define _FTP_CONNECTION_H

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
		CFTPConnection(boost::asio::io_service& a_ioService, CFTPListener& a_listener);

		void Start();
		void Stop();

		boost::asio::ip::tcp::socket& GetSocket() { return m_socket; }

	protected:
		boost::asio::io_service& m_ioService;
		CFTPListener& m_listener;
		boost::asio::ip::tcp::socket m_socket;

		void OnRead(const boost::system::error_code& e, std::size_t a_bytesTransferred);
		void OnWrite(const boost::system::error_code& e);
};

typedef boost::shared_ptr<CFTPConnection> PFTPConnection;

#endif /* !_FTP_CONNECTION_H */
