#include "../include/FoxNet.h"

NetPeer::NetPeer(boost::asio::io_service& service, size_t MaxInputSize, size_t MaxOutputSize) :
	m_service(service), m_socket(service), m_input(MaxInputSize), m_inputOffset(0), m_output(MaxOutputSize), m_outputOffset(0), m_port(0), disconnect_timer(service)
{
}
NetPeer::~NetPeer() 
{
}

const std::string& NetPeer::GetIP() const
{
	try
	{
		std::const_pointer_cast<NetPeer>(shared_from_this())->m_ip = m_socket.remote_endpoint().address().to_string();
	}
	catch(...) {}
	return m_ip;
}
uint16_t NetPeer::GetPort() const
{
	try
	{
		std::const_pointer_cast<NetPeer>(shared_from_this())->m_port = m_socket.remote_endpoint().port();
	}
	catch(...) {}
	return m_port;
}
bool NetPeer::IsConnected() const
{
	return m_socket.is_open();
}
boost::asio::ip::tcp::socket& NetPeer::GetSocket()
{
	return m_socket;
}
const boost::asio::ip::tcp::socket& NetPeer::GetSocket() const
{
	return m_socket;
}

void NetPeer::Disconnect()
{
	try
	{
		disconnect_timer.cancel();

		if(m_socket.is_open())
		{
			
			OnDisconnect();

			m_socket.shutdown(boost::asio::socket_base::shutdown_both);
			m_socket.close();
			m_inputOffset = 0;
			m_outputOffset = 0;
		}
	}
	catch(...)
	{
		m_inputOffset = 0;
		m_outputOffset = 0;
	}
}

std::size_t NetPeer::Send(const void* data, std::size_t length)
{
	if(m_outputOffset + length > m_output.size()) length = m_output.size() - m_outputOffset;
	memcpy(&m_output[m_outputOffset], data, length);
	m_outputOffset += length;
	return length;
}
void NetPeer::Flush()
{
	if(m_outputOffset > 0)
	{
		std::size_t l = OnWrite(&m_output[0], m_outputOffset);
		boost::asio::async_write(m_socket, boost::asio::buffer(&m_output[0], l), boost::bind(&NetPeer::HandleWrite, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
}

void NetPeer::HandleWrite(std::weak_ptr<NetPeer> self, const boost::system::error_code& e, std::size_t length)
{
	std::shared_ptr<NetPeer> _this(self.lock());
	if(_this)
	{
		if(!e)
		{
			if(length < _this->m_outputOffset)
			{
				memcpy(&_this->m_output[0], &_this->m_output[length], _this->m_outputOffset - length);
				_this->m_outputOffset = _this->m_outputOffset - length;
			}
			else
			{
				_this->m_outputOffset = 0;
			}
		}
		else
		{
			if(e != boost::asio::error::operation_aborted)
			{
				_this->Disconnect();
			}
		}
	}
}

void NetPeer::BeginRead()
{
	m_socket.async_read_some(boost::asio::buffer(&m_input[m_inputOffset], m_input.size() - m_inputOffset), boost::bind(&NetPeer::HandleRead, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void NetPeer::HandleRead(std::weak_ptr<NetPeer> self, const boost::system::error_code& e, std::size_t length)
{
	std::shared_ptr<NetPeer> _this(self.lock());
	if(_this)
	{
		if(!e)
		{
			std::size_t l = _this->OnRead(&_this->m_input[0], length + _this->m_inputOffset);
			if(l < length + _this->m_inputOffset)
			{
				memcpy(&_this->m_input[0], &_this->m_input[l], length + _this->m_inputOffset - l);
				_this->m_inputOffset = length + _this->m_inputOffset - l;
			}
			else
			{
				_this->m_inputOffset = 0;
			}
		}
		else
		{
			if(e != boost::asio::error::operation_aborted)
			{
				_this->Disconnect();
			}
		}
	}
}

void NetPeer::DelayedDisconnect(int timeOut)
{
	disconnect_timer.expires_from_now(boost::posix_time::milliseconds(timeOut));
	disconnect_timer.async_wait(boost::bind(&NetPeer::HandleDelayedDisconnect, shared_from_this(), boost::asio::placeholders::error));
}

void NetPeer::HandleDelayedDisconnect(std::weak_ptr<NetPeer> self, const boost::system::error_code& e)
{
	std::shared_ptr<NetPeer> _this(self.lock());
	if (_this)
	{
		if (!e)
			_this->Disconnect();
	}
}

