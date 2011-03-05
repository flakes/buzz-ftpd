//
// FTPInterpreter.cpp
// Copyright (c) 2011, cxxjoe
// Please refer to the LICENSE file for details.
//
// Based on:
// reference_counted.cpp
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//

#ifndef _ASIO_SMART_BUFFER_H
#define _ASIO_SMART_BUFFER_H

/**
 * A reference-counted non-modifiable buffer class.
 **/
class CAsioSmartBuffer
{
	public:
		// Construct from a std::string:
		explicit CAsioSmartBuffer(const std::string& data)
			: data_(new std::vector<char>(data.begin(), data.end())),
			buffer_(boost::asio::buffer(*data_))
		{
		}

		// Implement the ConstBufferSequence requirements:
		typedef boost::asio::const_buffer value_type;
		typedef const boost::asio::const_buffer* const_iterator;
		const boost::asio::const_buffer* begin() const { return &buffer_; }
		const boost::asio::const_buffer* end() const { return &buffer_ + 1; }

	private:
		boost::shared_ptr<std::vector<char> > data_;
		boost::asio::const_buffer buffer_;
};

#endif /* !_ASIO_SMART_BUFFER_H */
