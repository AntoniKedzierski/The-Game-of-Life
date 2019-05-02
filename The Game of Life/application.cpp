#include "application.h"
#include <chrono>

#define MAX_SPEED 100.0f
#define FPS_REFRESH_TIME 3

bool CUserWindow::s_IsRegistered = false;
WNDCLASSEX CUserWindow::s_WndClassEx = tagWNDCLASSEXA();
CCellColony g_CellColony = CCellColony();
CUserWindow g_UserWindow = CUserWindow(); 

LRESULT CALLBACK CUserWindow::WindowEventProc(HWND hWindow, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int xMouse = 0;
	static int yMouse = 0;
	static int nWheel = 0;
	static int nTimer = 36;

	switch (uMsg)
	{
	case WM_MOUSEWHEEL:
		nWheel += GET_WHEEL_DELTA_WPARAM(wParam);
		if (abs(nWheel) >= WHEEL_DELTA)
		{
			int nSteps = nWheel / WHEEL_DELTA;
			g_UserWindow.ChangeSize(nSteps);
			nWheel %= WHEEL_DELTA;
		}
		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_DELETE) g_CellColony.RemoveCell();
		if (wParam == VK_RETURN && g_CellColony.Alive()) g_CellColony.Start();
		if (wParam == VK_SPACE) g_CellColony.Pause();
		if (wParam == VK_ESCAPE) g_CellColony.End();
		if (wParam == VK_F1) g_UserWindow.ResetCordsSystem();
		if (wParam == VK_DOWN)
		{
			if (nTimer < 1000)
			{
				KillTimer(hWindow, 1);
				nTimer += 10;
				SetTimer(hWindow, 1, nTimer, NULL);
			}
		}
		if (wParam == VK_UP)
		{
			if (nTimer > 25)
			{
				KillTimer(hWindow, 1);
				nTimer -= 10;
				SetTimer(hWindow, 1, nTimer, NULL);
			}
		}
		return 0;
	case WM_RBUTTONDOWN:
		SetCapture(hWindow);
		xMouse = GET_X_LPARAM(lParam);
		yMouse = GET_Y_LPARAM(lParam);
		return 0;
	case WM_LBUTTONDOWN:
		if (!g_CellColony.Alive())
		{
			if (GET_Y_LPARAM(lParam) >= 40)
			{
				g_UserWindow.SetCordsBegining(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				g_CellColony.AddCell(0, 0);
			}
		}
		else
		{
			if (GET_Y_LPARAM(lParam) >= 40)
			{
				int x = GET_X_LPARAM(lParam);
				int y = GET_Y_LPARAM(lParam);
				g_UserWindow.CalculateCords(x, y);
				g_CellColony.AddCell(x, y);
			}
		}
		g_UserWindow.DrawColony(&g_CellColony);
		return 0;
	case WM_MOUSEMOVE:
		if (GetCapture() == hWindow)
		{
			g_UserWindow.MoveCordsSystem(GET_X_LPARAM(lParam) - xMouse, GET_Y_LPARAM(lParam) - yMouse);
			xMouse = GET_X_LPARAM(lParam);
			yMouse = GET_Y_LPARAM(lParam);
		}
		return 0;
	case WM_RBUTTONUP:
		ReleaseCapture();
		return 0;
	case WM_TIMER:
		g_CellColony.UpdateCells();
		g_UserWindow.DrawColony(&g_CellColony);
		return 0;
	case WM_SIZE:
		g_UserWindow.UpdateRect();
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hWindow, uMsg, wParam, lParam);
	}
}

// This function does not calculate exactly frames per second value, but for us is really sufficent.
int CalculateFPS()
{	
	// Remember the time of the first call and the current frames per second count.
	static auto LastFrame = std::chrono::system_clock::now();
	static int nFPS = 1;

	// Calculate differetion between this and the previous call of the function.
	auto nDiff = std::chrono::system_clock::now() - LastFrame;
	auto MilliSec = std::chrono::duration_cast<std::chrono::milliseconds>(nDiff);	// Hold it as milliseconds.
	
	// Return 1, when nDiff is equal to zero.
	if (MilliSec.count() == 0) return 1;

	// We want to refresh the FPS value only when the FPS_REFRESH_SPEED frames passed.
	static int nCounter = 0;
	if (nCounter % FPS_REFRESH_TIME == 0)
	{
		nCounter = 0;
		nFPS = 1000 / MilliSec.count();
	}
	nCounter++;

	// Change the time of the last (hmmm... present?) call.
	LastFrame = std::chrono::system_clock::now();

	// Return FPS value.
	return nFPS;
}

void CUserWindow::DrawColony(CCellColony* pColony)
{
	// Get DC from a window.
	HDC hdcWindow = GetDC(m_hWindow);
	HDC hdcBuffer = CreateCompatibleDC(hdcWindow);
	HBITMAP hbmpBuffer = CreateCompatibleBitmap(hdcWindow, m_prcSize.right, m_prcSize.bottom);
	HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcBuffer, hbmpBuffer);
	HBRUSH hbrWhite = (HBRUSH)GetStockObject(WHITE_BRUSH);
	HBRUSH hbrOld = (HBRUSH)SelectObject(hdcBuffer, hbrWhite);
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	HPEN hOldPen = (HPEN)SelectObject(hdcBuffer, hPen);
	HFONT hFont = CreateFont(16, 0, 0, 0, 0, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
	HFONT hOldFont = (HFONT)SelectObject(hdcBuffer, hFont);

	// Define cells positions and sizes.
	int x, y;
	double fRadius = 3 * m_CordsSystem.size / 8.0f;

	// Draw user interface.
	RECT rcClient;
	GetClientRect(m_hWindow, &rcClient);
	RECT rcGrowthSpeed = { 5, 5, rcClient.right / 4 - 1, 35 };
	SelectObject(hdcBuffer, GetStockObject(DC_BRUSH));

	if (g_CellColony.GrowthSpeed() > 0)
	{
		rcGrowthSpeed = { rcClient.right / 8 + 1, 5, 
			rcClient.right / 8 + (int)((rcClient.right / 8 - 2) * g_CellColony.GrowthSpeed() / MAX_SPEED), 35 };
		FillRect(hdcBuffer, &rcGrowthSpeed, CreateSolidBrush(RGB(30, 187, 45)));
	}
	if (g_CellColony.GrowthSpeed() < 0)
	{
		rcGrowthSpeed = { rcClient.right / 8, 5,
			rcClient.right / 8 - (int)((rcClient.right / 8 - 2) * (-1) * g_CellColony.GrowthSpeed() / MAX_SPEED), 35 };
		FillRect(hdcBuffer, &rcGrowthSpeed, CreateSolidBrush(RGB(210, 13, 45)));
	}

	rcGrowthSpeed = { 4, 4, rcClient.right / 4, 36 };
	SetDCBrushColor(hdcBuffer, RGB(255, 255, 255));
	HRGN hrgnFrame = CreateRoundRectRgn(rcGrowthSpeed.left, rcGrowthSpeed.top, rcGrowthSpeed.right, rcGrowthSpeed.bottom, 20, 20);
	FrameRgn(hdcBuffer, hrgnFrame, hbrWhite, 1, 1);

	HRGN hrgnRect = CreateRectRgn(0, 0, rcClient.right / 4 + 3, 39);
	HRGN hrgnBorder = CreateRectRgn(0, 0, 0, 0);
	CombineRgn(hrgnBorder, hrgnRect, hrgnFrame, RGN_DIFF);
	FillRgn(hdcBuffer, hrgnBorder, (HBRUSH)GetStockObject(BLACK_BRUSH));

	MoveToEx(hdcBuffer, 0, 40, NULL);
	LineTo(hdcBuffer, rcClient.right, 40);
	MoveToEx(hdcBuffer, rcClient.right / 8, 5, NULL);
	LineTo(hdcBuffer, rcClient.right / 8, 35);

	SetBkMode(hdcBuffer, TRANSPARENT);
	SetTextColor(hdcBuffer, RGB(255, 255, 255));
	std::string strNumber = std::to_string(g_CellColony.Size());
	std::string strInfo = "Colony size: " + strNumber;
	TextOut(hdcBuffer, rcClient.right / 4 + 5, 12, strInfo.c_str(), strInfo.length());
	strNumber = std::to_string(CalculateFPS());
	strInfo = "FPS: " + strNumber;
	TextOut(hdcBuffer, rcClient.right / 4 + 100, 12, strInfo.c_str(), strInfo.length());
	strInfo = "Press LMB to put a cell, press RMB to move the view and use the mouse scroll to zoom. Use arrows to regulate simulation speed.";
	TextOut(hdcBuffer, rcClient.right / 4 + 160, 4, strInfo.c_str(), strInfo.length());
	strInfo = "Press DEL to remove the last put cell, press ENTER to start simulation, SPACE to pause it and ESC to end.";
	TextOut(hdcBuffer, rcClient.right / 4 + 160, 22, strInfo.c_str(), strInfo.length());

	// Draw all cells.
	SelectObject(hdcBuffer, hbrWhite);
	for (int i = 0; i < pColony->Size(); ++i)
	{
		x = pColony->GetX(i) * m_CordsSystem.size + m_CordsSystem.x;
		y = pColony->GetY(i) * m_CordsSystem.size + m_CordsSystem.y;
		if (y - fRadius < 40) continue;
		Ellipse(hdcBuffer, x - fRadius, y - fRadius, x + fRadius, y + fRadius);
	}

	BitBlt(hdcWindow, 0, 0, m_prcSize.right, m_prcSize.bottom, hdcBuffer, 0, 0, SRCCOPY);
	DeleteObject(SelectObject(hdcBuffer, hOldFont));
	DeleteObject(SelectObject(hdcBuffer, hOldPen));
	DeleteObject(SelectObject(hdcBuffer, hbrOld));
	DeleteObject(SelectObject(hdcBuffer, hbmpOld));
	DeleteDC(hdcBuffer);
	ReleaseDC(m_hWindow, hdcWindow);
}
