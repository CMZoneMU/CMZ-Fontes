#pragma once
// join
#define MAX_LOG_TEXT_LINE 30
#define MAX_LOG_TEXT_SIZE 256

enum eLogColor
{
	LOG_BLACK = 0,
	LOG_RED = 1,
	LOG_GREEN = 2,
	LOG_BLUE = 3,
};

struct LOG_DISPLAY_INFO
{
	char text[MAX_LOG_TEXT_SIZE];
	eLogColor color;
};

class CServerDisplayer
{
public:

	CServerDisplayer();

	~CServerDisplayer();

	void Init(HWND hWnd);

	void Run();

	void SetWindowName();

	void ImgTop();

	void DrawLogs(HDC hdcMem, RECT rect);

	void ConfigureWindow(HWND hWnd);

	void LogAddText(eLogColor color, char* text, int size);

	void InitializeBuffer();

private:

	HDC m_hdcBuffer; // (new)

	HBITMAP m_hbmBuffer; // (new)

	HBITMAP m_hbmOldBuffer; // (new)

	HBITMAP m_hTopImage; // (new)

	HWND m_hwnd;

	HFONT m_font;

	HFONT m_logfont;

	HBRUSH m_brush[2];

	LOG_DISPLAY_INFO m_log[MAX_LOG_TEXT_LINE];

	int m_count;

	char m_DisplayerText[2][64];

	WNDPROC m_OldWndProc; // (new)

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam); // (new)
};

extern CServerDisplayer gServerDisplayer;