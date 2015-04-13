
#ifndef NETMESSAGE_H
#define NETMESSAGE_H

#include <WinSock2.h>

class NetServer;
class NetConnection;


class NetManager
{
public:
	NetManager();
	~NetManager();

	bool Initialize();
	void Shutdown();

	bool CreateServer(NetServer* result, const char* port);
	bool CreateClient(NetConnection* result, const char* target, const char* port);

	NetManager(const NetManager&) = delete;
	NetManager& operator=(const NetManager&) = delete;

private:
	bool mIsInitialized;
};

class NetServer
{
public:
	NetServer();
	~NetServer();

	bool WaitForConnection(NetConnection* pConnection);
	void Shutdown();

	NetServer(const NetServer&) = delete;
	NetServer& operator=(const NetServer&) = delete;

private:
	friend class NetManager;
	void SetupConnection(SOCKET server);

private:
	bool mIsInitialized;
	SOCKET mSocket;
};

class NetConnection
{
public:
	NetConnection();
	~NetConnection();

	void Shutdown();

	bool IsConnected();
	bool Send(const char* buf, size_t size);
	bool Receive(char* buf, size_t bufSize, size_t* receivedSize);

	NetConnection(const NetConnection&) = delete;
	NetConnection& operator=(const NetConnection&) = delete;

private:
	friend class NetManager;
	friend class NetServer;
	void SetupConnection(SOCKET connection);

private:
	bool mIsInitialized;
	SOCKET mSocket;
};

#endif // NETMESSAGE_H