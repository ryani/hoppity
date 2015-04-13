// Checkers.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <conio.h>
#include <string>
#include "NetMessage.h"
#include "Console.h"
#include "Rules.h"

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

	console.Write(CheckerState::kInitialState.PrintBoard().c_str());
	for (;;)
	{
		console.Update();
		std::string inputStr;
		if (console.ReadLine(&inputStr))
		{
			break;
		}
	}

	return 0;
}
