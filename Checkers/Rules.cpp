
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

	for (int spaceId = 0; spaceId < kNumSpaces; ++spaceId)
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

	switch (mTurn)
	{
	case TURN_BLACK:
		board.append(" X TO MOVE\n");
		break;
	case TURN_RED:
		board.append(" O TO MOVE\n");
		break;
	case END_BLACK:
		board.append(" X WINS!\n");
		break;
	case END_RED:
		board.append(" O WINS!\n");
		break;
	case END_DRAW:
		board.append(" DRAW!\n");
		break;
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

	if (spaceId >= 0 && spaceId < kNumSpaces)
		return spaceId;

	return -1;
}

void CheckerState::EncodeSpaceName(int spaceId, char buf[2])
{
	// handle invalid space ids
	if (spaceId < 0 || spaceId >= kNumSpaces)
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

	Space turnColor =
		(mTurn == TURN_BLACK) ? BLACK
		: (mTurn == TURN_RED) ? RED
		: EMPTY;

	if (turnColor == EMPTY)
	{
		// game is over, no legal moves
		return results;
	}

	Space oppColor = (turnColor == BLACK) ? RED : BLACK;

	// First check for capture moves
	for (int spaceId = 0; spaceId < kNumSpaces; ++spaceId)
	{
		Space space = GetSpace(spaceId);
		if ((space & kColorMask) != turnColor)
			continue;

		// TODO: Refactor (DRY); capture code copy-and-pasted.

		// Check for upwards capture
		if (turnColor == BLACK || (space & kKingFlag) != 0)
		{
			int adjacentSpaceId, jumpSpaceId;
			Space adjacentSpace, jumpSpace;

			adjacentSpaceId = UpLeft(spaceId);
			jumpSpaceId = (adjacentSpaceId >= 0) ? UpLeft(adjacentSpaceId) : -1;
			if (jumpSpaceId >= 0)
			{
				assert(adjacentSpaceId >= 0);
				adjacentSpace = GetSpace(adjacentSpaceId);
				jumpSpace = GetSpace(jumpSpaceId);

				if (jumpSpace == EMPTY && (adjacentSpace & kColorMask) == oppColor)
				{
					CheckerMove move = { spaceId, jumpSpaceId, adjacentSpaceId };
					results.push_back(move);
				}
			}

			adjacentSpaceId = UpRight(spaceId);
			jumpSpaceId = (adjacentSpaceId >= 0) ? UpRight(adjacentSpaceId) : -1;
			if (jumpSpaceId >= 0)
			{
				assert(adjacentSpaceId >= 0);
				adjacentSpace = GetSpace(adjacentSpaceId);
				jumpSpace = GetSpace(jumpSpaceId);

				if (jumpSpace == EMPTY && (adjacentSpace & kColorMask) == oppColor)
				{
					CheckerMove move = { spaceId, jumpSpaceId, adjacentSpaceId };
					results.push_back(move);
				}
			}
		}

		// Check for downwards capture
		if (turnColor == RED || (space & kKingFlag) != 0)
		{
			int adjacentSpaceId, jumpSpaceId;
			Space adjacentSpace, jumpSpace;

			adjacentSpaceId = DownLeft(spaceId);
			jumpSpaceId = (adjacentSpaceId >= 0) ? DownLeft(adjacentSpaceId) : -1;
			if (jumpSpaceId >= 0)
			{
				assert(adjacentSpaceId >= 0);
				adjacentSpace = GetSpace(adjacentSpaceId);
				jumpSpace = GetSpace(jumpSpaceId);

				if (jumpSpace == EMPTY && (adjacentSpace & kColorMask) == oppColor)
				{
					CheckerMove move = { spaceId, jumpSpaceId, adjacentSpaceId };
					results.push_back(move);
				}
			}

			adjacentSpaceId = DownRight(spaceId);
			jumpSpaceId = (adjacentSpaceId >= 0) ? DownRight(adjacentSpaceId) : -1;
			if (jumpSpaceId >= 0)
			{
				assert(adjacentSpaceId >= 0);
				adjacentSpace = GetSpace(adjacentSpaceId);
				jumpSpace = GetSpace(jumpSpaceId);

				if (jumpSpace == EMPTY && (adjacentSpace & kColorMask) == oppColor)
				{
					CheckerMove move = { spaceId, jumpSpaceId, adjacentSpaceId };
					results.push_back(move);
				}
			}
		}
	}

	// if there are any legal captures, no other moves are legal
	if (!results.empty())
		return results;

	// Add regular moves
	for (int spaceId = 0; spaceId < kNumSpaces; ++spaceId)
	{
		Space space = GetSpace(spaceId);
		if ((space & kColorMask) != turnColor)
			continue;

		// TODO: Refactor (DRY); move code copy-and-pasted.

		// Check for upwards move
		if (turnColor == BLACK || (space & kKingFlag) != 0)
		{
			int adjacentSpaceId;

			adjacentSpaceId = UpLeft(spaceId);
			if (adjacentSpaceId >= 0 && GetSpace(adjacentSpaceId) == EMPTY)
			{
				CheckerMove move = { spaceId, adjacentSpaceId, -1 };
				results.push_back(move);
			}

			adjacentSpaceId = UpRight(spaceId);
			if (adjacentSpaceId >= 0 && GetSpace(adjacentSpaceId) == EMPTY)
			{
				CheckerMove move = { spaceId, adjacentSpaceId, -1 };
				results.push_back(move);
			}
		}

		// Check for downwards move
		if (turnColor == RED || (space & kKingFlag) != 0)
		{
			int adjacentSpaceId;

			adjacentSpaceId = DownLeft(spaceId);
			if (adjacentSpaceId >= 0 && GetSpace(adjacentSpaceId) == EMPTY)
			{
				CheckerMove move = { spaceId, adjacentSpaceId, -1 };
				results.push_back(move);
			}

			adjacentSpaceId = DownRight(spaceId);
			if (adjacentSpaceId >= 0 && GetSpace(adjacentSpaceId) == EMPTY)
			{
				CheckerMove move = { spaceId, adjacentSpaceId, -1 };
				results.push_back(move);
			}
		}
	}

	return results;
}

bool CheckerState::IsLegalMove(CheckerMove move) const
{
	// TODO: This is just about the worst possible way to go about this.
	//
	// The difficult part of deciding if a move is legal is checking for another legal
	// capture move.
	//
	// IDEA: Should cache whether a capture move is available or not in the
	//       game state.  Should be easy to calculate with some masking operations.
	std::vector<CheckerMove> moves = GetLegalMoves();
	return (std::find(moves.begin(), moves.end(), move) != moves.end());
}

CheckerState CheckerState::ApplyMove(CheckerMove move) const
{
	// TODO
	return *this;
}