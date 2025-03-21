#include "stdafx.h"
#include "ServerDisplayer.h"
#include "GameMain.h"
#include "Log.h"
#include "ServerInfo.h"
#include "SocketManager.h"
#include "User.h"
#include "resource.h"

CServerDisplayer gServerDisplayer;

// ============================================================================================================================
// Inicializa os componentes da janela do Servidor
// ============================================================================================================================
CServerDisplayer::CServerDisplayer()
{
	this->m_hwnd = NULL;

	for (int n = 0; n < MAX_LOG_TEXT_LINE; n++)
	{
		memset(&this->m_log[n], 0, sizeof(this->m_log[n]));
	}

	this->m_logfont = CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Verdana");

	this->m_hTopImage = NULL; 

	this->m_hTopImage = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1));

	this->m_brush[0] = CreateSolidBrush(RGB(28, 27, 28)); // Cor de fundo do Display

	this->m_hdcBuffer = NULL;

	this->m_hbmBuffer = NULL;

	this->m_hbmOldBuffer = NULL;
}

// ============================================================================================================================
// Libera os recursos utilizados pela janela do Servidor
// ============================================================================================================================
CServerDisplayer::~CServerDisplayer()
{
	DeleteObject(this->m_font);

	DeleteObject(this->m_logfont);

	DeleteObject(this->m_brush[0]);

	if (this->m_hTopImage)
	{
		DeleteObject(this->m_hTopImage);
	}

}

// ============================================================================================================================
// Inicialização que configura a janela e carrega as configurações de log
// ============================================================================================================================
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

	this->ImgTop();
}

void CServerDisplayer::SetWindowName()
{
	char buff[256];

	wsprintf(buff, "[%s] MuEmu GameServer Ex097 (PlayerCount : %d/%d) (MonsterCount : %d/%d)", gServerInfo.m_WindowName, gObjTotalUser, gServerInfo.m_ServerMaxUserNumber, gObjTotalMonster, MAX_OBJECT_MONSTER);

	SetWindowText(this->m_hwnd, buff);
}

// ============================================================================================================================
// Confiruação de Resolução da Janela
// ============================================================================================================================
void CServerDisplayer::ConfigureWindow(HWND hWnd) // (new)
{
	this->m_hwnd = hWnd;

	Sleep(500);

	const int windowWidth = 800;
	const int windowHeight = 850;

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	int posX = (screenWidth - windowWidth) / 2;
	int posY = (screenHeight - windowHeight) / 2;

	// Primeiro remove todos os estilos existentes
	LONG style = GetWindowLong(this->m_hwnd, GWL_STYLE);
	style &= ~(WS_MAXIMIZEBOX | WS_THICKFRAME | WS_SIZEBOX);

	// Adiciona os estilos desejados
	style |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX |
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	// Aplica os estilos
	SetWindowLong(this->m_hwnd, GWL_STYLE, style);

	// Força o tamanho da janela em duas etapas
	SetWindowPos(this->m_hwnd, NULL, 0, 0, windowWidth, windowHeight,
		SWP_NOMOVE | SWP_FRAMECHANGED);

	Sleep(50);

	// Define a posição final
	SetWindowPos(this->m_hwnd, HWND_TOP, posX, posY, windowWidth, windowHeight,
		SWP_SHOWWINDOW | SWP_FRAMECHANGED);

	// Força atualização
	RedrawWindow(this->m_hwnd, NULL, NULL,
		RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
}

// ============================================================================================================================
// Inicializa ou reinicializa o buffer de exibição 
// ============================================================================================================================
void CServerDisplayer::InitializeBuffer()
{
	if (this->m_hwnd == NULL) return;

	HDC hdc = GetDC(this->m_hwnd);

	RECT rect;

	GetClientRect(this->m_hwnd, &rect);

	if (this->m_hdcBuffer)
	{
		SelectObject(this->m_hdcBuffer, this->m_hbmOldBuffer);

		DeleteObject(this->m_hbmBuffer);

		DeleteDC(this->m_hdcBuffer);
	}

	this->m_hdcBuffer = CreateCompatibleDC(hdc);

	this->m_hbmBuffer = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);

	this->m_hbmOldBuffer = (HBITMAP)SelectObject(this->m_hdcBuffer, this->m_hbmBuffer);

	ReleaseDC(this->m_hwnd, hdc);
}

// ============================================================================================================================
// Desenhar imagem
// ============================================================================================================================
void CServerDisplayer::ImgTop()
{
	if (!this->m_hwnd) return;

	RECT rect;

	GetClientRect(this->m_hwnd, &rect);

	HDC hdc = GetDC(this->m_hwnd);

	HDC hdcMem = CreateCompatibleDC(hdc);

	HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);

	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

	FillRect(hdcMem, &rect, m_brush[0]);


	// Desenhar logs
	if (this->m_log != nullptr)
	{
		DrawLogs(hdcMem, rect);
	}

	if (this->m_hTopImage)
	{
		const int imageTop = 0; // Topo da imagem
		const int imageBottom = 80;  // altura da imagem

		BITMAP bmp;
		GetObject(this->m_hTopImage, sizeof(BITMAP), &bmp);

		HDC hdcBmp = CreateCompatibleDC(hdcMem);

		HBITMAP hbmOldBmp = (HBITMAP)SelectObject(hdcBmp, this->m_hTopImage);

		StretchBlt(hdcMem, 0, imageTop, rect.right, imageBottom,

			hdcBmp, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

		SelectObject(hdcBmp, hbmOldBmp);

		DeleteDC(hdcBmp);
	}

	BitBlt(hdc, 0, 0, rect.right, rect.bottom, hdcMem, 0, 0, SRCCOPY);

	SelectObject(hdcMem, hbmOld);

	DeleteObject(hbmMem);

	DeleteDC(hdcMem);

	ReleaseDC(this->m_hwnd, hdc);
}

// ============================================================================================================================
// Desenhar Logs
// ============================================================================================================================
void CServerDisplayer::DrawLogs(HDC hdcMem, RECT rect)
{
	const int logStartX = 10;

	const int logStartY = rect.bottom - 555;

	const int logLineSpacing = 15;

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
			int currentY = logStartY + (line * logLineSpacing);

			TextOut(hdcMem, logStartX, currentY, this->m_log[count].text, size);

			line--;
		}

		count = (((--count) >= 0) ? count : (MAX_LOG_TEXT_LINE - 1));
	}
}

// ============================================================================================================================
// Adiciona um novo texto ao log com cor específica
// ============================================================================================================================
void CServerDisplayer::LogAddText(eLogColor color, char* text, int size)
{
	size = ((size >= MAX_LOG_TEXT_SIZE) ? (MAX_LOG_TEXT_SIZE - 1) : size);

	memset(&this->m_log[this->m_count].text, 0, sizeof(this->m_log[this->m_count].text));

	memcpy(&this->m_log[this->m_count].text, text, size);

	this->m_log[this->m_count].color = color;

	this->m_count = (((++this->m_count) >= MAX_LOG_TEXT_LINE) ? 0 : this->m_count);

	gLog.Output(LOG_GENERAL, "%s", &text[9]);

	InvalidateRect(this->m_hwnd, NULL, FALSE); 
}

// ============================================================================================================================
// Procedimento de Janela que processa as mensagens do Windows
// ============================================================================================================================
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

		displayer->ImgTop();

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