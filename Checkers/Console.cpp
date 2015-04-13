// Simple Win32 curses-lite library for 1-line input / multiline-output app

#include "stdafx.h"
#include "Console.h"
#include <conio.h>

Console::Console()
	: mHandle(INVALID_HANDLE_VALUE)
	, mCurrentInput()
	, mPendingInput()
{}

Console::~Console()
{
	Shutdown();
}

bool Console::Initialize(HANDLE consoleHandle)
{
	mHandle = consoleHandle;

	CONSOLE_SCREEN_BUFFER_INFO info;
	if (!GetConsoleScreenBufferInfo(mHandle, &info))
		return false;

	mInputLineY = info.dwSize.Y - 1;
	mWidth = info.dwSize.X - 1; // leave off 1 character to avoid scrolling when writing text to final line (TODO)
	mDefaultAttributes = info.wAttributes;

	// Clear screen
	int consoleSize = ((int)info.dwSize.X) * info.dwSize.Y;
	COORD origin = { 0, 0 };
	FillConsoleOutputCharacter(mHandle, TEXT(' '), consoleSize, origin, nullptr);
	FillConsoleOutputAttribute(mHandle, mDefaultAttributes, consoleSize, origin, nullptr);

	// Add line between input/output
	origin.Y = mInputLineY - 1;
	FillConsoleOutputCharacter(mHandle, TEXT('-'), info.dwSize.X, origin, nullptr);

	// We start output at the top
	mOutputX = 0;
	mOutputY = 0;

	// Put the cursor on the input line.
	ResetCursor();

	return true;
}

void Console::Shutdown()
{
	mCurrentInput.clear();
	mPendingInput.clear();
	mHandle = INVALID_HANDLE_VALUE;
}

void Console::Update()
{
	bool inputDirty = false;
	while (_kbhit())
	{
		inputDirty = true;
		AddInputChar(_getch());
	}

	if (inputDirty)
	{
		UpdateInputLine();
		ResetCursor();
	}
}

void Console::AddInputChar(int ch)
{
	if (ch == '\r')
	{
		mPendingInput.push_back(mCurrentInput);
		mCurrentInput.clear();
	}
	else if (ch == '\b')
	{
		if (!mCurrentInput.empty())
			mCurrentInput.pop_back();
	}
	else
	{
		mCurrentInput.push_back(ch);
	}
}

void Console::UpdateInputLine()
{
	std::wstring buf; 	// TODO: Shouldn't malloc this every time
	int start = 0, len = mCurrentInput.length();
	if (len > mWidth)
	{
		start += (len - mWidth);
		len = mWidth;
	}

	buf.append(mCurrentInput.begin() + start, mCurrentInput.begin() + start + len);
	for (int i = len; i < mWidth; ++i)
		buf.push_back(TEXT(' '));

	COORD c;
	c.X = 0;
	c.Y = mInputLineY;
	SetConsoleCursorPosition(mHandle, c);
	WriteConsole(mHandle, buf.c_str(), buf.length(), nullptr, nullptr);
}

bool Console::ReadLine(std::string *pInput)
{
	if (mPendingInput.empty())
		return false;

	*pInput = mPendingInput.front();
	mPendingInput.pop_front();
	return true;
}

void Console::Write(const char *str)
{
	if (!*str)
		return;

	CONSOLE_SCREEN_BUFFER_INFO info;
	if (!GetConsoleScreenBufferInfo(mHandle, &info))
		return;

	while (*str)
	{
		WriteCharInternal(*str++, &info.srWindow);
	}

	ResetCursor();
}

void Console::WriteCharInternal(char c, const SMALL_RECT *pWindow)
{
	// We only handle printable characters and tabs/newlines
	if (c != '\t' && c != '\n' && !isprint(c))
		return;

	// Handle newline
	if (c == '\n' || mOutputX == mWidth)
	{
		mOutputX = 0;
		mOutputY++;

		// Scroll if needed
		if (mOutputY == mInputLineY - 1)
		{
			mOutputY--;
			SMALL_RECT scrollRect = { 0, 0, mWidth, mOutputY };
			COORD destOrigin = { 0, -1 };
			CHAR_INFO chFill;
			chFill.Char.UnicodeChar = TEXT(' ');
			chFill.Attributes = mDefaultAttributes;
			ScrollConsoleScreenBuffer(mHandle, &scrollRect, nullptr, destOrigin, &chFill);
		}

		if (c == '\n')
			return;
	}

	// Handle tab
	if (c == '\t')
	{
		// TODO: Handle end-of-line tab better
		int n = 4 - (mOutputX % 4);
		for (int i = 0; i < n; ++i)
			WriteCharInternal(' ', pWindow);
		return;
	}

	// Write character
	CHAR_INFO ch[3];
	memset(ch, 0, sizeof(ch));
	// Win32 feels like string-type hell.  I'd really like to think in terms of UTF-8
	// here, but I can't find any good APIs for interacting with the console using
	// a more serious string type.
	ch[0].Char.UnicodeChar = (wchar_t)c;
	ch[0].Attributes = mDefaultAttributes;

	COORD chSize = { 1, 1 };
	COORD chPos = { 0, 0 };
	SMALL_RECT consoleRect = { mOutputX, pWindow->Top + mOutputY, mOutputX, pWindow->Top + mOutputY };
	WriteConsoleOutput(mHandle, ch, chSize, chPos, &consoleRect);

	// Move right
	mOutputX++;
}

void Console::ResetCursor()
{
	// set cursor to end of input
	COORD c;
	c.Y = mInputLineY;
	c.X = mCurrentInput.length();
	if (c.X >= mWidth)
		c.X = mWidth - 1;
	SetConsoleCursorPosition(mHandle, c);
}

