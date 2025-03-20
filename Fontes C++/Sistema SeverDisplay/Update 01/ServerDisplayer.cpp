#include "stdafx.h"
#include "ServerDisplayer.h"
#include "GameMain.h"
#include "Log.h"
#include "ServerInfo.h"
#include "SocketManager.h"
#include "User.h"
#include "resource.h"

CServerDisplayer gServerDisplayer;

CServerDisplayer::CServerDisplayer()
{
	this->m_hwnd = NULL;

	for (int n = 0; n < MAX_LOG_TEXT_LINE; n++)
	{
		memset(&this->m_log[n], 0, sizeof(this->m_log[n]));
	}

	this->m_logfont = CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Verdana");

	this->m_hTopImage = NULL;  // Inicializa o handle da imagem

	this->m_hTopImage = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1));

	this->m_brush = CreateSolidBrush(RGB(207,207, 203)); 

	// Inicializa variáveis de buffer
	this->m_hdcBuffer = NULL;

	this->m_hbmBuffer = NULL;

	this->m_hbmOldBuffer = NULL;
}

CServerDisplayer::~CServerDisplayer()
{
	DeleteObject(this->m_font);

	DeleteObject(this->m_logfont);

	DeleteObject(this->m_brush);

	if (this->m_hTopImage)
	{
		DeleteObject(this->m_hTopImage);
	}

}

void CServerDisplayer::ConfigureWindow(HWND hWnd) // (new)
{
	this->m_hwnd = hWnd;

	// Configurar estilo e posição da janela
	const int windowWidth = 800;

	const int windowHeight = 900;

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);

	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	int posX = (screenWidth - windowWidth) / 2;

	int posY = (screenHeight - windowHeight) / 2;

	// Estilos de janela aprimorados
	LONG style = GetWindowLong(this->m_hwnd, GWL_STYLE);

	style &= ~(WS_MAXIMIZEBOX | WS_THICKFRAME);

	style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	SetWindowLong(this->m_hwnd, GWL_STYLE, style);

	// Definir a posição e o tamanho da janela
	SetWindowPos(this->m_hwnd, NULL, posX, posY, windowWidth, windowHeight,
		SWP_NOZORDER | SWP_FRAMECHANGED);

	// Configurar procedimento de janela personalizada
	m_OldWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);
}

void CServerDisplayer::InitializeBuffer()
{
	if (this->m_hwnd == NULL) return;

	HDC hdc = GetDC(this->m_hwnd);

	RECT rect;

	GetClientRect(this->m_hwnd, &rect);

	// Limpar buffer antigo se existir
	if (this->m_hdcBuffer)
	{
		SelectObject(this->m_hdcBuffer, this->m_hbmOldBuffer);

		DeleteObject(this->m_hbmBuffer);

		DeleteDC(this->m_hdcBuffer);
	}

	// Criar novo buffer
	this->m_hdcBuffer = CreateCompatibleDC(hdc);

	this->m_hbmBuffer = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);

	this->m_hbmOldBuffer = (HBITMAP)SelectObject(this->m_hdcBuffer, this->m_hbmBuffer);

	ReleaseDC(this->m_hwnd, hdc);
}

void CServerDisplayer::Init(HWND hWnd)
{
	this->m_hwnd = hWnd;

	ConfigureWindow(hWnd);

	gLog.AddLog(true, "LOG");

	gLog.AddLog(gServerInfo.m_WriteChatLog, "CHAT_LOG");

	gLog.AddLog(gServerInfo.m_WriteCommandLog, "COMMAND_LOG");

	gLog.AddLog(gServerInfo.m_WriteTradeLog, "TRADE_LOG");

	gLog.AddLog(gServerInfo.m_WriteConnectLog, "CONNECT_LOG");

	gLog.AddLog(gServerInfo.m_WriteHackLog, "HACK_LOG");

	gLog.AddLog(gServerInfo.m_WriteChaosMixLog, "CHAOS_MIX_LOG");

	gServerDisplayer.InitializeBuffer();
}

void CServerDisplayer::Run()
{
	if (this->m_hwnd == NULL)
	{
		return;
	}

	this->PaintAllInfo();
}

void CServerDisplayer::SetWindowName()
{
	char buff[256];

	wsprintf(buff, "[%s] MuEmu GameServer Ex097 (PlayerCount : %d/%d) (MonsterCount : %d/%d)", gServerInfo.m_WindowName, gObjTotalUser, gServerInfo.m_ServerMaxUserNumber, gObjTotalMonster, MAX_OBJECT_MONSTER);

	SetWindowText(this->m_hwnd, buff);
}

void CServerDisplayer::PaintAllInfo()
{
	if (!this->m_hwnd) return;

	RECT rect;

	GetClientRect(this->m_hwnd, &rect);

	HDC hdc = GetDC(this->m_hwnd);

	// Criar buffer único
	HDC hdcMem = CreateCompatibleDC(hdc);

	HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);

	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

	// Limpar fundo
	HBRUSH hBrush = CreateSolidBrush(RGB(207, 207, 203));

	FillRect(hdcMem, &rect, hBrush);

	DeleteObject(hBrush);

	// Desenhar imagem
	if (this->m_hTopImage)
	{
		BITMAP bmp;
		GetObject(this->m_hTopImage, sizeof(BITMAP), &bmp);

		HDC hdcBmp = CreateCompatibleDC(hdcMem);

		HBITMAP hbmOldBmp = (HBITMAP)SelectObject(hdcBmp, this->m_hTopImage);

		StretchBlt(hdcMem, 0, 0, rect.right, 120,
			hdcBmp, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

		SelectObject(hdcBmp, hbmOldBmp);

		DeleteDC(hdcBmp);
	}

	// Desenhar logs
	int line = MAX_LOG_TEXT_LINE;

	int count = (((this->m_count - 1) >= 0) ? (this->m_count - 1) : (MAX_LOG_TEXT_LINE - 1));

	SetBkMode(hdcMem, TRANSPARENT);

	SelectObject(hdcMem, this->m_logfont);

	SetTextColor(hdcMem, RGB(255, 255, 255));

	for (int n = 0; n < MAX_LOG_TEXT_LINE; n++)
	{
		switch (this->m_log[count].color)
		{
		case LOG_BLACK:

			SetTextColor(hdcMem, RGB(255, 255, 255));

			break;

		case LOG_RED:

			SetTextColor(hdcMem, RGB(255, 0, 0));

			break;

		case LOG_GREEN:

			SetTextColor(hdcMem, RGB(0, 255, 0));

			break;

		case LOG_BLUE:

			SetTextColor(hdcMem, RGB(0, 0, 255));

			break;

		case LOG_ALERT:

			SetBkMode(hdcMem, OPAQUE);

			SetBkColor(hdcMem, RGB(255, 0, 0));

			SetTextColor(hdcMem, RGB(0, 0, 0));

			break;

		case LOG_USER:

			SetTextColor(hdcMem, RGB(140, 30, 160));

			break;

		case LOG_EVENT:

			SetTextColor(hdcMem, RGB(64, 192, 192));

			break;
		}

		int size = strlen(this->m_log[count].text);
		if (size > 1)
		{
			TextOut(hdcMem, 10, (rect.bottom - 554 + (line * 15)), this->m_log[count].text, size);
			line--;
		}

		count = (((--count) >= 0) ? count : (MAX_LOG_TEXT_LINE - 1));
	}

	// Copiar buffer para a tela
	BitBlt(hdc, 0, 0, rect.right, rect.bottom, hdcMem, 0, 0, SRCCOPY);

	// Limpar recursos
	SelectObject(hdcMem, hbmOld);

	DeleteObject(hbmMem);

	DeleteDC(hdcMem);

	ReleaseDC(this->m_hwnd, hdc);
}

void CServerDisplayer::LogAddText(eLogColor color, char* text, int size)
{
	size = ((size >= MAX_LOG_TEXT_SIZE) ? (MAX_LOG_TEXT_SIZE - 1) : size);

	memset(&this->m_log[this->m_count].text, 0, sizeof(this->m_log[this->m_count].text));

	memcpy(&this->m_log[this->m_count].text, text, size);

	this->m_log[this->m_count].color = color;

	this->m_count = (((++this->m_count) >= MAX_LOG_TEXT_LINE) ? 0 : this->m_count);

	gLog.Output(LOG_GENERAL, "%s", &text[9]);

	//this->Run();

	InvalidateRect(this->m_hwnd, NULL, FALSE);  // Substitui this->Run()
}

LRESULT CALLBACK CServerDisplayer::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) // (new)
{
	CServerDisplayer* displayer = (CServerDisplayer*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (msg)
	{
	case WM_ERASEBKGND:
		return 1;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;

		HDC hdc = BeginPaint(hwnd, &ps);

		displayer->PaintAllInfo();  // Chama apenas PaintAllInfo

		EndPaint(hwnd, &ps);

		return 0;
	}

	case WM_SIZE:
	{
		if (displayer) displayer->InitializeBuffer();

		InvalidateRect(hwnd, NULL, FALSE);

		break;
	}
	}

	return CallWindowProc(displayer->m_OldWndProc, hwnd, msg, wParam, lParam);
}