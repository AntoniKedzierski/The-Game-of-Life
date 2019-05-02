#include "application.h"

bool CUserWindow::s_IsRegistered = false;
WNDCLASSEX CUserWindow::s_WndClassEx = tagWNDCLASSEXA();
CCellColony g_CellColony = CCellColony();
CUserWindow g_UserWindow = CUserWindow(); 

LRESULT CALLBACK CUserWindow::WindowEventProc(HWND hWindow, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int xMouse = 0;
	static int yMouse = 0;

	switch (uMsg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_DELETE) g_CellColony.RemoveCell();
		if (wParam == VK_RETURN && g_CellColony.Alive()) g_CellColony.Start();
		return 0;
	case WM_RBUTTONDOWN:
		SetCapture(hWindow);
		xMouse = GET_X_LPARAM(lParam);
		yMouse = GET_Y_LPARAM(lParam);
		return 0;
	case WM_LBUTTONDOWN:
		if (!g_CellColony.Alive())
		{
			g_UserWindow.SetCordsBegining(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			g_CellColony.AddCell(0, 0);
		}
		else
		{
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);
			g_UserWindow.CalculateCords(x, y);
			g_CellColony.AddCell(x, y);
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

void CUserWindow::GetCells()
{
	// A window has its own DC.
	//HDC hdcKontekst = GetDC(m_hWindow);
	

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

	// Define cells positions and sizes.
	int x, y;
	double fRadius = 3 * m_CordsSystem.size / 8.0f;

	// Draw all cells.
	for (int i = 0; i < pColony->Size(); ++i)
	{
		x = pColony->GetX(i) * m_CordsSystem.size + m_CordsSystem.x;
		y = pColony->GetY(i) * m_CordsSystem.size + m_CordsSystem.y;
		Ellipse(hdcBuffer, x - fRadius, y - fRadius, x + fRadius, y + fRadius);
	}

	BitBlt(hdcWindow, 0, 0, m_prcSize.right, m_prcSize.bottom, hdcBuffer, 0, 0, SRCCOPY);
	DeleteObject(SelectObject(hdcBuffer, hbrOld));
	DeleteObject(SelectObject(hdcBuffer, hbmpOld));
	DeleteDC(hdcBuffer);
	ReleaseDC(m_hWindow, hdcWindow);
}
