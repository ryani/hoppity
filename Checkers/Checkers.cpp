// Checkers.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <conio.h>
#include <string>
#include "NetMessage.h"
#include "Console.h"

const char* kPort = "8088";

int _tmain(int argc, _TCHAR* argv[])
{
	Console console;
	console.Initialize(GetStdHandle(STD_OUTPUT_HANDLE));

	NetManager netManager;
	if (!netManager.Initialize())
	{
		printf("Couldn't initialize network\n");
		return 1;
	}

	NetConnection client;
	NetServer server;
	if (argc == 1)
	{
		console.Write("Starting as server...\n");
		if (!netManager.CreateServer(&server, "8088"))
		{
			console.Write("Couldn't create server\n");
			return 1;
		}

		server.WaitForConnection(&client);
	}
	else
	{
		std::string host;
		host.append(argv[1], argv[1] + lstrlen(argv[1]));

		// std::string should have printf-formatting.  EASTL does it!
		console.Write("Connecting to '");
		console.Write(host.c_str());
		console.Write("'...\n");

		if (!netManager.CreateClient(&client, host.c_str(), "8088"))
		{
			printf("Couldn't create client\n");
			return 1;
		}
	}

	std::string inputStr;

	for (;;)
	{
		console.Update();

		const size_t kBufSize = 1024;
		size_t receivedSize;
		char buf[kBufSize+1];

		if (!client.IsConnected())
			break;

		if (!client.Receive(buf, kBufSize, &receivedSize))
			break;

		buf[receivedSize] = '\0';
		console.Write(buf);
		if (console.ReadLine(&inputStr))
		{
			inputStr.push_back('\r');
			inputStr.push_back('\n');
			console.Write(inputStr.c_str());
			client.Send(inputStr.c_str(), inputStr.length());
			inputStr.clear();
		}
	}

	return 0;
}
