#ifndef FOXNET_NETSERVICE_H
#define FOXNET_NETSERVICE_H

class NetService : boost::noncopyable
{
public:
	size_t Run();
	size_t Poll();
	void Stop();
	bool HasStopped();

	boost::asio::io_service& operator()();

private:
	boost::asio::io_service m_service;
};

#endif