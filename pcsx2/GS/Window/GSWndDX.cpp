/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2021 PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"
#include "GSWndDX.h"

#ifdef _WIN32
GSWndDX::GSWndDX()
	: m_hWnd(NULL)
	, m_frame(true)
{
}

GSWndDX::~GSWndDX()
{
}

LRESULT CALLBACK GSWndDX::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	GSWndDX* wnd = NULL;

	if (message == WM_NCCREATE)
	{
		wnd = (GSWndDX*)((LPCREATESTRUCT)lParam)->lpCreateParams;

		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)wnd);

		wnd->m_hWnd = hWnd;
	}
	else
	{
		wnd = (GSWndDX*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	}

	if (wnd == NULL)
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return wnd->OnMessage(message, wParam, lParam);
}

LRESULT GSWndDX::OnMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CLOSE:
			Hide();
			// DestroyWindow(m_hWnd);
			return 0;
		case WM_DESTROY:
			// This kills the emulator when GS is closed, which *really* isn't desired behavior,
			// especially in STGS mode (worked in MTGS mode since it only quit the thread, but even
			// that wasn't needed).
			//PostQuitMessage(0);
			return 0;
		default:
			break;
	}

	return DefWindowProc((HWND)m_hWnd, message, wParam, lParam);
}

bool GSWndDX::Create(const std::string& title, int w, int h)
{
	if (m_hWnd)
		throw GSRecoverableError();

	m_managed = true;

	WNDCLASS wc;

	memset(&wc, 0, sizeof(wc));

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = theApp.GetModuleHandle();
	// TODO: wc.hIcon = ;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszClassName = L"GSWndDX";

	if (!GetClassInfo(wc.hInstance, wc.lpszClassName, &wc))
	{
		if (!RegisterClass(&wc))
		{
			throw GSRecoverableError();
		}
	}

	DWORD style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_OVERLAPPEDWINDOW | WS_BORDER;

	GSVector4i r;

	GetWindowRect(GetDesktopWindow(), r);

	bool remote = !!GetSystemMetrics(SM_REMOTESESSION);

	if (w <= 0 || h <= 0 || remote)
	{
		w = r.width() / 3;
		h = r.width() / 4;

		if (!remote)
		{
			w *= 2;
			h *= 2;
		}
	}

	r.left = (r.left + r.right - w) / 2;
	r.top = (r.top + r.bottom - h) / 2;
	r.right = r.left + w;
	r.bottom = r.top + h;

	AdjustWindowRect(r, style, FALSE);
	std::wstring tmp = std::wstring(title.begin(), title.end());
	m_hWnd = CreateWindow(wc.lpszClassName, tmp.c_str(), style, r.left, r.top, r.width(), r.height(), NULL, NULL, wc.hInstance, (LPVOID)this);

	if (!m_hWnd)
		throw GSRecoverableError();

	return true;
}

bool GSWndDX::Attach(void* handle, bool managed)
{
	// TODO: subclass

	m_hWnd = (HWND)handle;
	m_managed = managed;

	return true;
}

void GSWndDX::Detach()
{
	if (m_hWnd && m_managed)
	{
		// close the window, since it's under GS care.  It's not taking messages anyway, and
		// that means its big, ugly, and in the way.

		DestroyWindow(m_hWnd);
	}

	m_hWnd = NULL;
	m_managed = true;
}

GSVector4i GSWndDX::GetClientRect()
{
	GSVector4i r;

	::GetClientRect(m_hWnd, r);

	return r;
}

// Returns FALSE if the window has no title, or if th window title is under the strict
// management of the emulator.

bool GSWndDX::SetWindowText(const char* title)
{
	if (!m_managed)
		return false;

	const size_t tmp_size = strlen(title) + 1;
	std::wstring tmp(tmp_size, L'#');
	mbstowcs(&tmp[0], title, tmp_size);
	::SetWindowText(m_hWnd, tmp.c_str());

	return m_frame;
}

void GSWndDX::Show()
{
	if (!m_managed)
		return;

	SetForegroundWindow(m_hWnd);
	ShowWindow(m_hWnd, SW_SHOWNORMAL);
	UpdateWindow(m_hWnd);
}

void GSWndDX::Hide()
{
	if (!m_managed)
		return;

	ShowWindow(m_hWnd, SW_HIDE);
}

void GSWndDX::HideFrame()
{
	if (!m_managed)
		return;

	SetWindowLong(m_hWnd, GWL_STYLE, GetWindowLong(m_hWnd, GWL_STYLE) & ~(WS_CAPTION | WS_THICKFRAME));
	SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	SetMenu(m_hWnd, NULL);

	m_frame = false;
}
#endif
