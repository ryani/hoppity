
#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#include "NetMessage.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib,"Ws2_32.lib")

NetManager::NetManager()
	: mIsInitialized(false)
{
	
}

NetManager::~NetManager()
{
	Shutdown();
}

void NetManager::Shutdown()
{
	if (!mIsInitialized)
		return;

	WSACleanup();
}

bool NetManager::Initialize() {
	WSADATA winsockData;
	int winsockStatus = WSAStartup(MAKEWORD(2, 2), &winsockData);
	if (winsockStatus != 0)
	{
		printf("Winsock init failed with error %d\n", winsockStatus);
		return false;
	}

	if (winsockData.wVersion != MAKEWORD(2, 2))
	{
		printf("Couldn't init winsock2.2\n");
		WSACleanup();
		return false;
	}

	mIsInitialized = true;
	return true;
}

bool NetManager::CreateServer(NetServer* pServer, const char* port)
{
	// Shut down the server if it's active
	pServer->Shutdown();

	// Set up the server
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo *result;
	int iResult = getaddrinfo(nullptr, port, &hints, &result);

	if (iResult != 0)
	{
		printf("getaddrinfo failed with code %d\n", iResult);
		return false;
	}

	SOCKET listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket() failed with code %d\n", WSAGetLastError());
		freeaddrinfo(result);
		return false;
	}

	iResult = bind(listenSocket, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with code %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(listenSocket);
		return false;
	}

	freeaddrinfo(result);

	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		printf("listen failed with code %d\n", WSAGetLastError());
		closesocket(listenSocket);
		return false;
	}

	pServer->SetupConnection(listenSocket);
	return true;
}

bool NetManager::CreateClient(NetConnection* pConnection, const char* host, const char* port)
{
	return false; // TODO
}

void NetServer::Shutdown()
{
	if (!mIsInitialized)
		return;
	mIsInitialized = false;

	if (mSocket != INVALID_SOCKET)
	{
		closesocket(mSocket);
		mSocket = INVALID_SOCKET;
	}
}

NetServer::NetServer()
	: mIsInitialized(false)
	, mSocket(INVALID_SOCKET)
{
}

NetServer::~NetServer()
{
	Shutdown();
}

bool NetServer::WaitForConnection(NetConnection* pConnection)
{
	if (!mIsInitialized)
		return false;

	SOCKET clientSocket = accept(mSocket, nullptr, nullptr);
	if (clientSocket == INVALID_SOCKET)
		return false;

	pConnection->SetupConnection(clientSocket);
	return true;
}

void NetServer::SetupConnection(SOCKET server)
{
	Shutdown();
	mIsInitialized = true;
	mSocket = server;
}

NetConnection::NetConnection()
	: mIsInitialized(false)
	, mSocket(INVALID_SOCKET)
{

}

NetConnection::~NetConnection()
{
	Shutdown();
}

void NetConnection::SetupConnection(SOCKET connection)
{
	Shutdown();
	mIsInitialized = true;
	mSocket = connection;
	
	// Enable non-blocking mode on the connected socket
	u_long mode = 1;
	ioctlsocket(mSocket, FIONBIO, &mode);
}

void NetConnection::Shutdown()
{
	if (!mIsInitialized)
		return;
	mIsInitialized = false;

	if (mSocket != INVALID_SOCKET)
	{
		closesocket(mSocket);
		mSocket = INVALID_SOCKET;
	}
}

bool NetConnection::Send(const char* buf, size_t size)
{
	if (!mIsInitialized)
		return false;

	size_t size_remaining = size;
	while (size_remaining > 0)
	{
		int iResult = send(mSocket, buf, size_remaining, 0);
		if (iResult == SOCKET_ERROR)
		{
			// TODO: Do we need to handle EWOULDBLOCK here?
			printf("send failed: %d\n", WSAGetLastError());
			Shutdown();
			return false;
		}

		if (iResult == 0)
		{
			// Failed to send any bytes?
			printf("failed to send anything\n");
			Shutdown();
			return false;
		}

		buf += iResult;
		size_remaining -= iResult;
	}

	return true;
}

bool NetConnection::Receive(char* buf, size_t bufSize, size_t* receivedSize)
{
	if (!mIsInitialized)
		return false;

	int iResult = recv(mSocket, buf, bufSize, 0);
	if (iResult > 0)
	{
		*receivedSize = iResult;
		return true;
	}


	if (iResult == 0)
	{
		// The windows docs make it seems like this happens if the connection is closed
		Shutdown();
		return false;
	}

	int err = WSAGetLastError();
	if (err == WSAEWOULDBLOCK)
	{
		// No data available.
		*receivedSize = 0;
		return true;
	}

	// Otherwise we are dead.
	printf("recv failed with code %d\n", err);
	Shutdown();
	return false;
}

bool NetConnection::IsConnected()
{
	return mIsInitialized;
}

