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

	// I wish std::string had printf formatting.
	char moveBuf[] = "  ->   ";

	CheckerState gameState = CheckerState::kInitialState;

	console.Write(gameState.PrintBoard().c_str());
	auto moves = gameState.GetLegalMoves();
	for (auto move : moves)
	{
		CheckerState::EncodeSpaceName(move.fromSpace, &moveBuf[0]);
		CheckerState::EncodeSpaceName(move.toSpace, &moveBuf[4]);
		console.Write(moveBuf);
	}
	console.Write("\n");
	
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
