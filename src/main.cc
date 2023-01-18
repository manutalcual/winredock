//-*- mode: c++; indent-tabs-mode: t; -*-
//
// Program: wm Copyright (c) 2018 Manuel Cano
// Author: manuel.cano@amadeus.com
// Date: Tue Sep 18 16:57:22 2018
// Time-stamp: <>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
//   02110-1301	USA
//
//
// Includes
//
#include "main.hh"
#include "dev.hh"

// Enable the latest common controls
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' ""version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Link with necessary libraries
#pragma comment (lib, "comctl32.lib")
#pragma comment (lib, "version.lib")


const char c_class_name[] = "WinReDock";
const char c_window_title[] = "WinReDock - restore windows to pre-undock positions";
const char c_taskbar_icon_text[] = "WinReDock -- tooling; dockerify after undock!";
mcm::window<c_class_name,
			WndProc,
			(CS_HREDRAW | CS_VREDRAW),
			c_window_title> * g_app = nullptr;

std::string file_name{"window_list.json"};

INT_PTR CALLBACK AboutProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR args, int iCmdShow )
{
	// Only allow one instance of the app to run
	HANDLE mutex = CreateMutex(NULL, TRUE, "WinReDock:{0C023FEF-AC05-47A7-BC3A-5270916D768C}");
	if (ERROR_ALREADY_EXISTS == GetLastError())
	{
		// Another instance is already running
		return 1;
	}

	logf ();
	logp (sys::e_debu, "Creating window with class name: " << c_class_name);

	mcm::sys::set_cwd changedir;

	if (! changedir(mcm::sys::set_cwd::cwd::home)) {
		FatalAppExit (0, TEXT("Can't change working dir."));
	}

	file_name  = changedir.path() + "\\" + file_name;

	mcm::window<c_class_name,
				WndProc,
				(CS_HREDRAW | CS_VREDRAW),
				c_window_title>
		app (hInstance, hPrevInstance, args, iCmdShow);
	g_app = &app; // global for use by WndProc

	logp (sys::e_debug, "Setting windows message handlers");
	app[WM_CREATE] =
		[&app](HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> DWORD {
			logf ();
			app.create_menu (
				// Context left click function
				[&] (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> DWORD {
					return app;
				}) // Context menus and functions
				.add_menu_item(MF_STRING,
							   ID_TRAY_ABOUT_MENU_ITEM,
							   TEXT("About WinReDock..."),
							   [&] (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> DWORD {
								   logp (sys::e_debug, "Display about dialog.");
								   DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG1), hwnd, AboutProc);
								   return app;
							   })
				.add_menu_item(MF_SEPARATOR, 0, NULL)
				/* Exit menu does something */
				.add_menu_item(MF_STRING,
							   ID_TRAY_EXIT_CONTEXT_MENU_ITEM,
							   TEXT("Exit"),
							   [&] (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> DWORD {
								   logp (sys::e_debug, "Calling to exit app.");
								   DestroyWindow(hwnd);
								   return app;
							   })
				.start_timer();
				
			return app;
		};

	/*
	  These are Windows operating system messages we react on.
	*/

	app[WM_TRAYICON] =
		[&] (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> DWORD {
			/* Traybar icon clicks and so */
			logf ();
			POINT curPoint;
			GetCursorPos (&curPoint);
			SetForegroundWindow (hwnd);
			UINT clicked = TrackPopupMenu(
				app.get_menu_handle(),
				TPM_RETURNCMD | TPM_NONOTIFY,
				curPoint.x,
				curPoint.y,
				0,
				hwnd,
				NULL
				);
			return clicked;
		};

	if (! app.create()) {
		mcm::win_error err ("Can't create window object.");
		err ();
	}

	logp (sys::e_debug, "Registering for taskbar creation message.");
	app.register_for_taskbar_message ();
	app.add_taskbar_icon<NIF_ICON | NIF_MESSAGE | NIF_TIP,
						 ID_TRAY_APP_ICON,
						 WM_TRAYICON,
						 c_taskbar_icon_text> ();

	logp (sys::e_debug, "En creations, begin the main loop.");

	int app_result = (int) app.loop(); //msg.wParam;

	ReleaseMutex(mutex);
	CloseHandle(mutex);
	return app_result;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	logf ();
	logp (sys::e_debug, "Message received: "
		  << message << ", "
		  << wParam << ", " << lParam << ".");
	LRESULT r = g_app->handle (hwnd, message, wParam, lParam);
	logp (sys::e_debug, "Return from handled: "
		  << r);
	return r;
}

INT_PTR CALLBACK AboutProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			// Clear the version info
			SetDlgItemText(hwnd, IDC_VERSION_TEXT, "");

			// Attempt to get the current product version
			char path[MAX_PATH + 1] = { 0 };
			GetModuleFileName(NULL, path, MAX_PATH);
			DWORD dummy;
			DWORD dwSize = GetFileVersionInfoSize(path, &dummy);
			if (dwSize == 0)
			{
				return TRUE;
			}
			std::vector<BYTE> versionInfo(dwSize);

			if (!GetFileVersionInfo(path, NULL, dwSize, &versionInfo[0]))
			{
				return TRUE;
			}
			UINT versionLen = 0;
			VS_FIXEDFILEINFO* pffi = 0;
			if (VerQueryValue(&versionInfo[0], TEXT("\\"), (void**)&pffi, (UINT*)&versionLen) == 0)
			{
				return TRUE;
			}

			// Set the version info in the about box
			std::ostringstream vs;
			vs << "Version " << HIWORD(pffi->dwProductVersionMS) << "."
				<< LOWORD(pffi->dwProductVersionMS) << "."
				<< HIWORD(pffi->dwProductVersionLS) << "."
				<< LOWORD(pffi->dwProductVersionLS);
			SetDlgItemText(hwnd, IDC_VERSION_TEXT, vs.str().c_str());

			return TRUE;
		}
		break;

		case WM_NOTIFY:
		{
			switch (((LPNMHDR)lParam)->code)
			{
			case NM_CLICK:
			case NM_RETURN:
			{
				char url[512];
				GetDlgItemText(hwnd, IDC_SYSLINK1, url, 512);
				lstrcpyn(url, url + 3, lstrlen(url) - 7 + 1);
				ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOW);
				return TRUE;
			}
			break;
			}
		}
		break;

		case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDOK)
			{
				EndDialog(hwnd, IDOK);
				return TRUE;
			}
		}
		break;

		case WM_SYSCOMMAND:
		{
			if (wParam == SC_CLOSE)
			{
				EndDialog(hwnd, IDOK);
				return TRUE;
			}
		}
		break;

	}
	return FALSE;
}
