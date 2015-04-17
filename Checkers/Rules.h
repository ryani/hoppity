
#ifndef RULES_H
#define RULES_H

#include <string>
#include <vector>


// The board is represented as an array of 32 elements
// stored in bits in an __int64 and int32.
//
//    White side
//   1c  1d  1e  1f
// 18  19  1a  1b
//   14  15  16  17
// 10  11  12  13 
//   0c  0d  0e  0f
// 08  09  0a  0b
//   04  05  06  07
// 00  01  02  03 
//   Black side
//
// (mPieces >> (space*2)) & 3
// => 0 if empty
// => 2 if black
// => 3 if red
//
// (mKings >> space) & 1
// => whether or not that space
//    has a king in it.

// A single move (including jumps), but no double jumps
struct CheckerMove
{
	uint8_t fromSpace;
	uint8_t toSpace;
};

class CheckerState
{
public:
	enum Turn {
		TURN_BLACK,
		TURN_RED,
		END_BLACK,
		END_RED,
		END_DRAW
	};

	CheckerState() = default;
	CheckerState(int64_t board, int32_t kings, Turn turn);

	static const int kNumSpaces = 32;

	static const int kKingFlag = 4;
	static const int kColorMask = 3;
	enum Space {
		EMPTY = 0,
		BLACK = 2,
		RED = 3,
		BLACK_KING = 6,
		RED_KING = 7
	};

	// All the state variables here:
	int64_t mPieces; // 0=empty, 1=unused, 2=black, 3=red
	int32_t mKings; // flag for spaces containing kings
	Turn mTurn;
	// That's it!
	//
	// Note that we aren't handling draws due to repeated board positions,
	// at least not in this class.

	Space GetSpace(int space) const;
	void SetSpace(int space, Space value);

	std::string PrintBoard() const;
	std::vector<CheckerMove> GetLegalMoves() const;

public:
	// Static stuff
	static const CheckerState kInitialState;
	static const CheckerState kEmptyState;

	static int UpLeft(int space);
	static int UpRight(int space);
	static int DownLeft(int space);
	static int DownRight(int space);

	// a1 -> 0, for example
	static int DecodeSpaceName(const char* name);
	static void EncodeSpaceName(int spaceId, char buf[2]);

	static void ToXY(int spaceId, int& x, int& y);
};


inline int CheckerState::UpLeft(int space)
{
	if ((space & 7) == 0 || (space & 0x1c) == 0)
		return -1;
	if (space & 4)
		return space + 4;
	return space + 3;
}

inline int CheckerState::UpRight(int space)
{
	if ((space & 7) == 7 || (space & 0x1c) == 0x1c)
		return -1;
	if (space & 4)
		return space + 5;
	return space + 4;
}

inline int CheckerState::DownLeft(int space)
{
	if ((space & 7) == 0 || (space & 0x1c) == 0)
		return -1;
	if (space & 4)
		return space - 4;
	return space - 5;
}

inline int CheckerState::DownRight(int space)
{
	if ((space & 7) == 7 || (space & 0x1c) == 0x1c)
		return -1;
	if (space & 4)
		return space - 3;
	return space - 4;
}

inline CheckerState::Space CheckerState::GetSpace(int space) const
{
	int pieceType = (mPieces >> (space * 2)) & 0x3;
	int isKing = (mKings >> space) & 1;

	return (Space)(pieceType | (isKing << 2));
}

inline void CheckerState::SetSpace(int space, Space value)
{
	// check for invalid piece types
	assert(value != 4 && (value & 3) != 1);

	int64_t spaceMask = int64_t(kColorMask) << (space * 2);
	mPieces = (mPieces & ~spaceMask) | (int64_t(value & kColorMask) << (space * 2));
	int32_t kingMask = int32_t(1) << space;
	mKings = (mKings & ~kingMask) | ((int32_t(value & kKingFlag) >> 2) << space);
}

inline CheckerState::CheckerState(int64_t board, int32_t kings, Turn turn)
	: mPieces(board)
	, mKings(kings)
	, mTurn(turn)
{}

inline void CheckerState::ToXY(int spaceId, int& x, int& y)
{
	// Gets the board xy position (0-7) given a space Id
	x = (spaceId & 3) * 2;
	y = (spaceId >> 2);
	x += (y & 1);
}
#endif // RULES_H