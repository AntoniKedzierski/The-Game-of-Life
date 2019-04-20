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

// Let's make some windows classes, to ease a process of creating windows, buttons, etc.
typedef class CWindow
{
protected:
	std::string m_strName;
	RECT* m_prcSize;
	int m_nVisible;

public:
	CWindow() : m_strName(""), m_prcSize(nullptr), m_nVisible(SW_HIDE) { }
	CWindow(std::string strName, RECT rcSize) : m_strName(strName), m_nVisible(SW_HIDE)
	{
		m_prcSize = new RECT();
		*m_prcSize = rcSize;
	}
	~CWindow()
	{
		delete m_prcSize;
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

typedef class CUserWindow : public CWindow
{
static WNDCLASSEX m_WndClassEx;

private:
	DWORD m_dwStyle;
	DWORD m_dwStyleEx;
	static LRESULT CALLBACK WindowEventProc(HWND, UINT, WPARAM, LPARAM);

public:
	CUserWindow() : CWindow(), m_dwStyle(0), m_dwStyleEx(0) { }
	CUserWindow(std::string strName, int nWidth, int nHeigth, bool bFullScreen = false) 
		: CWindow(strName, { 200, 200, nWidth, nHeigth }), m_dwStyle(WS_OVERLAPPEDWINDOW), m_dwStyleEx(NULL)
	{
		

	}

	static ATOM CreateWindowsClass()
	{
		ZeroMemory(&m_WndClassEx, sizeof(WNDCLASSEX));
		m_WndClassEx.cbSize = sizeof(WNDCLASSEX);
		m_WndClassEx.hInstance = GetModuleHandle(NULL);
		m_WndClassEx.lpszClassName = "user_window";
		m_WndClassEx.lpfnWndProc = CUserWindow::WindowEventProc;
		m_WndClassEx.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
		m_WndClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		m_WndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
		m_WndClassEx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

		ATOM bResult = RegisterClassEx(&m_WndClassEx);
		if (!bResult) MessageBox(NULL, "B³¹d podczas inicjacji klasy okna", "Fatalny b³¹d", MB_ICONERROR);
		return bResult;
	}

} *LPUSERWINDOW;

