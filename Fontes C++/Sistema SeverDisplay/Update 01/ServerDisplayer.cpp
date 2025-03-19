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

	this->m_font = CreateFont(70, 0, 0, 0, FW_THIN, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Verdana");

	this->m_logfont = CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Verdana");

	this->m_hTopImage = NULL;  // Inicializa o handle da imagem

	// Carregar a imagem
	this->m_hTopImage = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1));

	// Inicializa variáveis de buffer

	this->m_hdcBuffer = NULL;

	this->m_hbmBuffer = NULL;

	this->m_hbmOldBuffer = NULL;
}

CServerDisplayer::~CServerDisplayer()
{
	DeleteObject(this->m_font);

	DeleteObject(this->m_logfont);

	DeleteObject(this->m_brush[0]);

	DeleteObject(this->m_brush[1]);

	// ... código existente ...

	if (this->m_hTopImage)
	{
		DeleteObject(this->m_hTopImage);
	}

	// Limpa recursos de buffer
	if (this->m_hdcBuffer)
	{
		SelectObject(this->m_hdcBuffer, this->m_hbmOldBuffer);

		DeleteObject(this->m_hbmBuffer);

		DeleteDC(this->m_hdcBuffer);
	}

}

void CServerDisplayer::Init(HWND hWnd)
{
	this->m_hwnd = hWnd;

	// Configurar estilo e posição da janela
	const int windowWidth = 800; // Largura da janela

	const int windowHeight = 750; // Altura da janela

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


	gLog.AddLog(true, "LOG");

	gLog.AddLog(gServerInfo.m_WriteChatLog, "CHAT_LOG");

	gLog.AddLog(gServerInfo.m_WriteCommandLog, "COMMAND_LOG");

	gLog.AddLog(gServerInfo.m_WriteTradeLog, "TRADE_LOG");

	gLog.AddLog(gServerInfo.m_WriteConnectLog, "CONNECT_LOG");

	gLog.AddLog(gServerInfo.m_WriteHackLog, "HACK_LOG");

	gLog.AddLog(gServerInfo.m_WriteChaosMixLog, "CHAOS_MIX_LOG");
}

void CServerDisplayer::Run()
{
	if (this->m_hwnd == NULL)
	{
		return;
	}

	// Força atualização da janela
	InvalidateRect(this->m_hwnd, NULL, FALSE);

	UpdateWindow(this->m_hwnd);
}

void CServerDisplayer::SetWindowName()
{
	char buff[256];

	wsprintf(buff, "[%s] MuEmu GameServer Ex097 (PlayerCount : %d/%d) (MonsterCount : %d/%d)", gServerInfo.m_WindowName, gObjTotalUser, gServerInfo.m_ServerMaxUserNumber, gObjTotalMonster, MAX_OBJECT_MONSTER);

	SetWindowText(this->m_hwnd, buff);
}

void CServerDisplayer::PaintAllInfo()
{
	RECT rect;

	GetClientRect(this->m_hwnd, &rect);

	rect.top = 0;

	rect.bottom = 80;

	HDC hdc = GetDC(this->m_hwnd);

	HDC hdcMem = CreateCompatibleDC(hdc);

	HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rect.right - rect.left, rect.bottom - rect.top);

	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

	// Desenhar a imagem redimensionada
	if (this->m_hTopImage)
	{
		BITMAP bmp;
		GetObject(this->m_hTopImage, sizeof(BITMAP), &bmp);

		HDC hdcBmp = CreateCompatibleDC(hdcMem);
		HBITMAP hbmOldBmp = (HBITMAP)SelectObject(hdcBmp, this->m_hTopImage);

		// Usar StretchBlt para redimensionar a imagem
		StretchBlt(hdcMem, 0, 0, rect.right - rect.left, rect.bottom,
			hdcBmp, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

		SelectObject(hdcBmp, hbmOldBmp);
		DeleteDC(hdcBmp);
	}

	// Configurações do buffer de memória
	SetBkMode(hdcMem, TRANSPARENT);
	HFONT OldFont = (HFONT)SelectObject(hdcMem, this->m_font);

	// Renderizar título no buffer de memória
	if (gJoinServerConnection.CheckState() == 0 || gDataServerConnection.CheckState() == 0)
	{
		SetTextColor(hdcMem, RGB(200, 200, 200));
		DrawText(hdcMem, this->m_DisplayerText[0], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}
	else
	{
		gGameServerDisconnect = 0;
		SetTextColor(hdcMem, RGB(250, 250, 250));
		DrawText(hdcMem, this->m_DisplayerText[1], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}

	// Copiar do buffer de memória para a tela
	BitBlt(hdc, 0, rect.top, rect.right - rect.left, rect.bottom - rect.top,
		hdcMem, 0, 0, SRCCOPY);

	// Restaurar e limpar recursos
	SelectObject(hdcMem, OldFont);

	SelectObject(hdcMem, hbmOld);

	DeleteObject(hbmMem);

	DeleteDC(hdcMem);

	ReleaseDC(this->m_hwnd, hdc);

}

void CServerDisplayer::LogTextPaint()
{
	RECT rect;
	GetClientRect(this->m_hwnd, &rect);
	rect.top = 80;

	HDC hdc = GetDC(this->m_hwnd);
	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rect.right - rect.left, rect.bottom - rect.top);
	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

	// Configurações iniciais no buffer
	SetBkMode(hdcMem, TRANSPARENT);

	HFONT OldFont = (HFONT)SelectObject(hdcMem, this->m_logfont);

	SetTextColor(hdcMem, RGB(255, 255, 255));

	SetBkColor(hdcMem, RGB(0, 0, 0));

	// Preencher fundo
	FillRect(hdcMem, &rect, (HBRUSH)COLOR_CAPTIONTEXT);

	int line = MAX_LOG_TEXT_LINE;
	int count = (((this->m_count - 1) >= 0) ? (this->m_count - 1) : (MAX_LOG_TEXT_LINE - 1));

	for (int n = 0; n < MAX_LOG_TEXT_LINE; n++)
	{
		switch (this->m_log[count].color)
		{
		case LOG_BLACK:
			SetBkMode(hdcMem, TRANSPARENT);
			SetTextColor(hdcMem, RGB(255, 255, 255));
			break;

		case LOG_RED:
			SetBkMode(hdcMem, TRANSPARENT);
			SetTextColor(hdcMem, RGB(255, 0, 0));
			break;

		case LOG_GREEN:
			SetBkMode(hdcMem, TRANSPARENT);
			SetTextColor(hdcMem, RGB(0, 255, 0));
			break;

		case LOG_BLUE:
			SetBkMode(hdcMem, TRANSPARENT);
			SetTextColor(hdcMem, RGB(0, 0, 255));
			break;

		case LOG_ALERT:
			SetBkMode(hdcMem, OPAQUE);
			SetBkColor(hdcMem, RGB(255, 0, 0));
			SetTextColor(hdcMem, RGB(0, 0, 0));
			break;

		case LOG_USER:
			SetBkMode(hdcMem, TRANSPARENT);
			SetTextColor(hdcMem, RGB(140, 30, 160));
			break;

		case LOG_EVENT:
			SetBkMode(hdcMem, TRANSPARENT);
			SetTextColor(hdcMem, RGB(64, 192, 192));
			break;
		}

		int size = strlen(this->m_log[count].text);
		if (size > 1)
		{
			TextOut(hdcMem, 10, (rect.bottom - 475 + (line * 15)),
				this->m_log[count].text, size);
			line--;
		}

		count = (((--count) >= 0) ? count : (MAX_LOG_TEXT_LINE - 1));
	}

	// Copiar do buffer de memória para a tela
	BitBlt(hdc, 0, rect.top, rect.right - rect.left, rect.bottom - rect.top,
		hdcMem, 0, 0, SRCCOPY);

	// Limpar recursos
	SelectObject(hdcMem, OldFont);

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

	this->Run();
}

LRESULT CALLBACK CServerDisplayer::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

			displayer->PaintAllInfo();

			displayer->LogTextPaint();

			EndPaint(hwnd, &ps);

			return 0;
		}
	}

	return CallWindowProc(displayer->m_OldWndProc, hwnd, msg, wParam, lParam);
}