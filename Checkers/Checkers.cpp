// Checkers.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <conio.h>
#include <string>
#include "NetMessage.h"


// Thanks StackOverflow!
int getch_noblock()
{
	if (_kbhit())
		return _getch();

	return -1;
}

int _tmain(int argc, _TCHAR* argv[])
{
	NetManager netManager;
	if (!netManager.Initialize())
	{
		printf("Couldn't initialize network\n");
		return 1;
	}

	printf("%d\n", argc);

	NetConnection client;
	NetServer server;

	if (1)
	{
		if (!netManager.CreateServer(&server, "8088"))
		{
			printf("Couldn't create server\n");
			return 1;
		}

		server.WaitForConnection(&client);
	}
	else
	{
		if (!netManager.CreateClient(&client, "127.0.0.1", "8088"))
		{
			printf("Couldn't create client\n");
			return 1;
		}
	}

	std::string inputStr;

	for (;;)
	{
		const size_t kBufSize = 1024;
		size_t receivedSize;
		char buf[kBufSize+1];

		if (!client.IsConnected())
			break;

		if (!client.Receive(buf, kBufSize, &receivedSize))
			break;

		buf[receivedSize] = '\0';
		printf("%s", buf);
		fflush(stdout);

		int kb = getch_noblock();
		if (kb == '\n')
		{
			inputStr.push_back('\n');
			client.Send(inputStr.c_str(), inputStr.length());
			inputStr.clear();
		}
		else if (kb != -1)
		{
			inputStr.push_back((char)kb);
		}
	}

	return 0;
}

