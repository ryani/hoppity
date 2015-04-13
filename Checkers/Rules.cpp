
#include "stdafx.h"
#include "Rules.h"

// white spaces in top 3 rows, black spaces in bottom 3 rows
static const int64_t kInitialBoard = (int64_t(0xffffff) << 0x28) | 0xaaaaaa;

const CheckerState CheckerState::kInitialState = CheckerState(kInitialBoard, 0, TURN_BLACK);
const CheckerState CheckerState::kEmptyState = CheckerState(0, 0, END_DRAW);

std::string CheckerState::PrintBoard() const
{
	std::string board =
		" - - - - 8\n"
		"- - - -  7\n"
		" - - - - 6\n"
		"- - - -  5\n"
		" - - - - 4\n"
		"- - - -  3\n"
		" - - - - 2\n"
		"- - - -  1\n"
		"abcdefgh  \n";

	for (int spaceId = 0; spaceId < 32; ++spaceId)
	{
		Space space = GetSpace(spaceId);
		const char* kSpaceTypes = "-?xo#!XO";
		char spaceChar = kSpaceTypes[space];

		int x, y;
		ToXY(spaceId, x, y);

		// row 0 at the bottom
		y = 7 - y;

		// Set the board spot
		board[y*11 + x] = spaceChar;
	}

	return board;
}

int CheckerState::DecodeSpaceName(const char* name)
{
	if (!name)
		return -1;

	if (*name == '\0')
		return -1;

	int spaceId;

	// Check for integer
	if (name[0] >= '0' && name[0] <= '9')
	{
		spaceId = name[0] - '0';
		if (name[1] >= '0' && name[1] <= '9')
		{
			spaceId = spaceId * 10 + (name[1] - '0');

			// 2 digit spaces only
			if (name[2] != '\0')
				return -1;
		}
		else if (name[1] != '\0')
		{
			return -1;
		}

		// board space numerals are 1-based.
		// Note: this seems to be different than the coordinate system used on 
		// (for example) http://www.usacheckers.com/famouspositions.php
		// which seems to use 1 for the bottom right coordinate, increasing
		// from right to left.
		spaceId--;
	}
	else if (name[1] >= '1' && name[1] <= '8'
		&& ((name[0] >= 'a' && name[0] <= 'h')
		    || name[0] >= 'A' && name[0] <= 'H'))
	{
		int x = name[0] >= 'a' ? (name[0] - 'a') : (name[0] - 'A');
		int y = name[1] - '1';

		// x and y must have the same parity
		if ((x & 1) != (y & 1))
			return -1;

		spaceId = y * 4 + x / 2;
	}

	if (spaceId >= 0 && spaceId < 32)
		return spaceId;

	return -1;
}

void CheckerState::EncodeSpaceName(int spaceId, char buf[2])
{
	// handle invalid space ids
	if (spaceId < 0 || spaceId >= 32)
	{
		buf[0] = buf[1] = '?';
		return;
	}

	// Ugly, but gets the coordinate
	int x, y;
	ToXY(spaceId, x, y);
	buf[0] = x + 'a';
	buf[1] = y + '1';
}

// Get the moves legal from a given state
std::vector<CheckerMove> CheckerState::GetLegalMoves() const
{
	std::vector<CheckerMove> results;

	// no moves if game is over
	if (mTurn >= END_BLACK)
		return results;

	// TODO
	return results;
}