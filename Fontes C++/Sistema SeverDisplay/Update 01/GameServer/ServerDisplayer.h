#pragma once

#define MAX_LOG_TEXT_LINE 36
#define MAX_LOG_TEXT_SIZE 256

enum eLogColor
{
	LOG_BLACK = 0,
	LOG_RED = 1,
	LOG_GREEN = 2,
	LOG_BLUE = 3,
	LOG_USER = 4,
	LOG_EVENT = 5,
	LOG_ALERT = 6,
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

	void LogAddText(eLogColor color, char* text, int size);

	void InitializeBuffer(); // (new)
	
	void ConfigureWindow(HWND hWnd);// (ne)

	void DrawLogs(HDC hdcMem, RECT rect);

private:


	HDC m_hdcBuffer; // (new)

	HBITMAP m_hbmBuffer; // (new)

	HBITMAP m_hbmOldBuffer; // (new)

	HBITMAP m_hTopImage; // (new)

	HWND m_hwnd;

	HFONT m_font;

	HFONT m_logfont;

	HBRUSH m_brush[3];

	LOG_DISPLAY_INFO m_log[MAX_LOG_TEXT_LINE];

	int m_count;

	char m_DisplayerText[2][64];

	WNDPROC m_OldWndProc; // (new)

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam); // (new)
};

extern CServerDisplayer gServerDisplayer;