/*
@author		Antoni Kêdzierski
@version	Ver.1.0

A header contains all nessecary parts of the appliaction lauch and display process.
All graphics issuses are included here, also WindowEventProc() and so on. 
*/

#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>
#include <iostream>
#include <string>
#include "cellworld.h"

#define REGISTERED true
#define NOT_REGISTERED false

// Let's make some windows classes, to ease a process of creating windows, buttons, etc.
typedef class CWindow
{
protected:
	std::string m_strName;
	RECT m_prcSize;
	int m_nVisible;
	HWND m_hWindow;

public:
	CWindow() : m_strName(""), m_prcSize(tagRECT()), m_nVisible(SW_HIDE), m_hWindow(NULL) { }
	CWindow(std::string strName, RECT rcSize) : m_strName(strName), m_nVisible(SW_HIDE), m_hWindow(NULL)
	{
		m_prcSize = rcSize;
	}

	virtual void Show()
	{
		m_nVisible = SW_SHOW;
	}

	virtual void Hide()
	{
		m_nVisible = SW_HIDE;
	}
} *LPWINDOW;

// It is our main window class.
typedef class CUserWindow : public CWindow
{
public:
	static WNDCLASSEX s_WndClassEx;
	static bool s_IsRegistered;

private:
	DWORD m_dwStyle;
	DWORD m_dwStyleEx;
	bool m_bGetMode;
	static LRESULT CALLBACK WindowEventProc(HWND, UINT, WPARAM, LPARAM);
	struct CORD_SYS
	{
		double x = 0, y = 0;
		int size = 0;
	} m_CordsSystem;

public:
	CUserWindow() : CWindow(), m_dwStyle(0), m_dwStyleEx(0), m_bGetMode(false) { }
	CUserWindow(std::string strName, int nWidth, int nHeight, bool bFullScreen = false) 
		: CWindow(strName, { 200, 200, nWidth, nHeight }), m_dwStyle(WS_OVERLAPPEDWINDOW | WS_MAXIMIZE), m_dwStyleEx(NULL), m_bGetMode(false)
	{
		if (!s_IsRegistered)
		{
			CreateWindowsClass();
			s_IsRegistered = true;
		}
		m_hWindow = CreateWindowEx(m_dwStyleEx, "user_window", strName.c_str(), m_dwStyle, 200, 200, nWidth, nHeight, NULL, NULL, GetModuleHandle(NULL), NULL);
		SetTimer(m_hWindow, 1, 36, NULL); // About 20 ticks per second
	}

	static ATOM CreateWindowsClass()
	{
		s_IsRegistered = NOT_REGISTERED;
		ZeroMemory(&s_WndClassEx, sizeof(WNDCLASSEX));
		s_WndClassEx.cbSize = sizeof(WNDCLASSEX);
		s_WndClassEx.hInstance = GetModuleHandle(NULL);
		s_WndClassEx.lpszClassName = "user_window";
		s_WndClassEx.lpfnWndProc = CUserWindow::WindowEventProc;
		s_WndClassEx.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
		s_WndClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		s_WndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
		s_WndClassEx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

		ATOM bResult = RegisterClassEx(&s_WndClassEx);
		if (!bResult) MessageBox(NULL, "B³¹d podczas inicjacji klasy okna", "Fatalny b³¹d", MB_ICONERROR);
		s_IsRegistered = REGISTERED;
		return bResult;
	}

	void Show() override
	{
		CWindow::Show();
		ShowWindow(m_hWindow, m_nVisible);
	}

	void Hide() override
	{
		CWindow::Hide();
		ShowWindow(m_hWindow, m_nVisible);
	}

	void GetModeOn()
	{
		m_bGetMode = true;
	}

	void GetModeOff()
	{
		m_bGetMode = false;
	}

	void UpdateRect()
	{
		GetWindowRect(m_hWindow, &m_prcSize);
	}

	// Managing a cordinate system.
	void MoveCordsSystem(int dx, int dy)
	{
		m_CordsSystem.x += dx;
		m_CordsSystem.y += dy;
	}

	void ChangeSize(int nDelta)
	{
		if (m_CordsSystem.size * pow(1.1, nDelta) < 11 || m_CordsSystem.size * pow(1.1, nDelta) > 51) return;
		m_CordsSystem.size *= pow(1.1, nDelta);
	}

	void SetCordsBegining(int x, int y)
	{
		m_CordsSystem.x = x;
		m_CordsSystem.y = y;
		m_CordsSystem.size = 16; // Default distance between fields is 16.
	}

	// Caltulate position of a cell in a current cordinate system. The center of the cordinate system
	// is shifted on a vector (-0.5*t, -0.5*t), where t = m_CordsSystem.size, so we must reshift it into a right place.
	void CalculateCords(int& x, int& y)
	{
		x = floor((x - m_CordsSystem.x + 0.5 * m_CordsSystem.size) / (m_CordsSystem.size * 1.0));
		y = floor((y - m_CordsSystem.y + 0.5 * m_CordsSystem.size) / (m_CordsSystem.size * 1.0));
	}

	void DrawColony(CCellColony*);

} *LPUSERWINDOW;

// The main user window.
extern CUserWindow g_UserWindow;
