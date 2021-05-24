#ifndef FOXNET_NETPEER_H
#define FOXNET_NETPEER_H

class NetPeer : boost::noncopyable ,public std::enable_shared_from_this<NetPeer>
{
public:
	NetPeer(boost::asio::io_service& service, size_t MaxInputSize = 1024*32, size_t MaxOutputSize = 1024*32);
	virtual ~NetPeer();

	
	boost::asio::ip::tcp::socket& GetSocket();
	const boost::asio::ip::tcp::socket& GetSocket() const;
	const std::string& GetIP() const;
	uint16_t GetPort() const;
	bool IsConnected() const;
	
	
	std::size_t Send(const void* data, std::size_t length);
	void Flush();
	
	void BeginRead();

	void Disconnect();
	void DelayedDisconnect(int timeOut);
	
	virtual void OnConnect() = 0;
	virtual void OnDisconnect() = 0;
	virtual std::size_t OnRead(const void* data, std::size_t length) = 0;
	virtual std::size_t OnWrite(const void* data, std::size_t length) = 0;

private:
	boost::asio::io_service& m_service;
	boost::asio::ip::tcp::socket m_socket;
	std::vector<uint8_t> m_input;
	std::size_t m_inputOffset;
	std::vector<uint8_t> m_output;
	std::size_t m_outputOffset;
	std::string m_ip;
	uint16_t m_port;

	boost::asio::deadline_timer disconnect_timer;

	
private:
	static void HandleRead(std::weak_ptr<NetPeer> self, const boost::system::error_code& e, std::size_t length);
	static void HandleWrite(std::weak_ptr<NetPeer> self, const boost::system::error_code& e, std::size_t length);
	static void HandleDelayedDisconnect(std::weak_ptr <NetPeer> self, const boost::system::error_code & e);
};

#endif