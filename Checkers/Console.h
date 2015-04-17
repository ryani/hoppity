
#ifndef CONSOLE_H
#define CONSOLE_H

#include <windows.h>
#include <string>
#include <deque>

class Console
{
public:
	Console();
	~Console();

	bool Initialize(HANDLE screenHandle);
	void Update();
	void Shutdown();

	bool ReadLine(std::string *pInput);
	void Write(const char* str);

private:
	Console(const Console&) = delete;
	Console& operator=(const Console&) = delete;

	void AddInputChar(int ch);
	void UpdateInputLine();
	void ResetCursor();
	void WriteCharInternal(char c);

	HANDLE mHandle;
	std::string mCurrentInput;
	std::deque<std::string> mPendingInput;
	int mInputLineY;
	int mWidth;
	int mOutputX;
	int mOutputY;
	WORD mDefaultAttributes;
};

#endif // CONSOLE_H