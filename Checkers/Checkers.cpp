// Checkers.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <conio.h>
#include <string>
#include "NetMessage.h"
#include "Console.h"
#include "Rules.h"

void ReadLine(Console& console, std::string* pInputStr)
{
	for (;;)
	{
		console.Update();
		if (console.ReadLine(pInputStr))
			break;
	}
}

bool DecodeMove(const std::string& str, CheckerMove* /*out*/ pMove)
{
	// TODO
	pMove->fromSpace = pMove->toSpace = 0;
	pMove->jumpSpace = -1;
	return true;
}

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
	for (;;)
	{
		console.Write(gameState.PrintBoard().c_str());
		auto moves = gameState.GetLegalMoves();
		for (auto move : moves)
		{
			CheckerState::EncodeSpaceName(move.fromSpace, &moveBuf[0]);
			CheckerState::EncodeSpaceName(move.toSpace, &moveBuf[4]);
			console.Write(moveBuf);
		}
		console.Write("\n");

		std::string inputStr;
		ReadLine(console, &inputStr);

		CheckerMove move;
		if (!DecodeMove(inputStr, &move))
		{
			console.Write("Couldn't understand that move\n");
			continue;
		}

		if (!gameState.IsLegalMove(move))
		{
			console.Write("Illegal move!\n");
			continue;
		}

		gameState = gameState.ApplyMove(move);
	}

	return 0;
}
