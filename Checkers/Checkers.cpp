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

void DisplayHelp(Console& console)
{
	console.Write(
		"?: this help\n"
		"m: list legal moves\n"
		"b: display board\n"
		"n: new game\n"
		"xx yy: move from space xx to space yy.\n"
		"       Spaces are labeled a1-h8,\n"
		"       or 01-32 (left-to-right, bottom-to-top)\n"
		"q: quit\n"
		"\n"
		"Disclaimer: currently can't parse or apply moves!\n"
		);
}

void PrintBoard(Console& console, CheckerState gameState)
{
	console.Write("\n");
	console.Write(gameState.PrintBoard().c_str());
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
	PrintBoard(console, gameState);
	for (;;)
	{
		std::string inputStr;
		ReadLine(console, &inputStr);

		if (inputStr == "q")
		{
			// quit
			break;
		}
		else if (inputStr == "?" || inputStr == "help")
		{
			// help
			DisplayHelp(console);
		}
		else if (inputStr == "n")
		{
			// new game
			gameState = CheckerState::kInitialState;
			PrintBoard(console, gameState);
		}
		else if (inputStr == "b")
		{
			// draw board
			PrintBoard(console, gameState);
		}
		else if (inputStr == "m")
		{
			// show moves
			auto moves = gameState.GetLegalMoves();
			for (CheckerMove move : moves)
			{
				CheckerState::EncodeSpaceName(move.fromSpace, &moveBuf[0]);
				CheckerState::EncodeSpaceName(move.toSpace, &moveBuf[4]);
				console.Write(moveBuf);
			}
			console.Write("\n");
		}
		else
		{
			// Try to move
			CheckerMove move;
			if (!DecodeMove(inputStr, &move))
			{
				console.Write("Invalid command (? for help)\n");
			}
			else if(!gameState.IsLegalMove(move))
			{
				console.Write("Illegal move!\n");
			}
			else
			{
				gameState = gameState.ApplyMove(move);
				PrintBoard(console, gameState);
			}
		}
	}

	return 0;
}
