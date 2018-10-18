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

using namespace mc;

const char c_class_name[] = "WinReDock";
const char c_window_title[] = "WinReDock - restore windows to pre-undock positions";
const char c_taskbar_icon_text[] = "Amadeus tooling: dockerify after undock!";
mcm::window<c_class_name,
			WndProc,
			(CS_HREDRAW | CS_VREDRAW),
			c_window_title> * g_app = nullptr;

mapwin_t windows;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR args, int iCmdShow )
{
	logf ();
	logp (sys::e_debu, "Creating window with class name: " << c_class_name);

	serializer serial (windows);
	if (!serial.deserialize(FILE_NAME, windows)) {
		logp (sys::e_debug, "Ouch! Deserializer failed!");
	}

	EnumWindows (&Enum, (LPARAM)&windows);
	uniform_windows (windows);

	mcm::window<c_class_name,
				WndProc,
				(CS_HREDRAW | CS_VREDRAW),
				c_window_title>
		app (hInstance, hPrevInstance, args, iCmdShow);
	g_app = &app; // for WndProc

	app[WM_CREATE] =
		[&app](HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> DWORD {
			logf ();
			app.create_menu (
				// Context left click function
				[&app] (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> DWORD {
					mapwin_t::iterator begin = windows.begin();
					mapwin_t::iterator end = windows.end();
					for (; begin != end; ++begin) {
						logp (sys::e_debug, "Setting placement for '"
							  << begin->second._title << "'");
						if (begin->second._place.showCmd == SW_MAXIMIZE) {
							WINDOWPLACEMENT wp = begin->second._place;
							wp.showCmd = SW_RESTORE;
							wp.flags = WPF_ASYNCWINDOWPLACEMENT;
							SetWindowPlacement (begin->second._hwnd, &wp);
							ShowWindow (begin->second._hwnd, SW_MINIMIZE);
							ShowWindow (begin->second._hwnd, SW_SHOW);
						}
						if (SetWindowPlacement(begin->second._hwnd, &begin->second._place)) {
							logp (sys::e_debug, "Can't set window placement for last window.");
						} else {
							ShowWindow (begin->second._hwnd, SW_MINIMIZE);
							ShowWindow (begin->second._hwnd, SW_SHOW);
						}
					}
					return app;
				}) // Context menus and functions
				.add_menu_item(MF_STRING,
							   ID_TRAY_LOAD_WINDOWS_MENU,
							   TEXT("Get windows"),
							   [&app] (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> DWORD {
								   logp (sys::e_debug, "Call to load windows.");
								   windows.clear ();
								   // Get windows opened
								   EnumWindows (&Enum, (LPARAM)&windows);
								   return app;
							   })
				.add_menu_item(MF_STRING,
							   ID_TRAY_SAVE_MENU,
							   TEXT("Save config."),
							   [&app] (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> DWORD {
								   logp (sys::e_debug, "Calling to serialize data.");
								   serializer serial (windows);
								   serial (FILE_NAME);
								   logp (sys::e_debug, "Ok. Done.");
								   return app;
							   })
				.add_menu_item(MF_STRING,
							   ID_TRAY_LOAD_MENU,
							   TEXT("Read config."),
							   [&app] (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> DWORD {
								   logp (sys::e_debug, "Calling to read file.");
								   serializer serial (windows);
								   if (!serial.deserialize(FILE_NAME, windows)) {
									   logp (sys::e_debug, "Ouch! Deserializer failed!");
								   }
								   EnumWindows (&Enum, (LPARAM)&windows);
								   uniform_windows (windows);
								   return app;
							   })
				.add_menu_item(MF_SEPARATOR, 0, NULL)
				.add_menu_item(MF_STRING,
							   ID_TRAY_EXIT_CONTEXT_MENU_ITEM,
							   TEXT("Exit"),
							   [&app] (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> DWORD {
								   logp (sys::e_debug, "Calling to exit app.");
								   PostQuitMessage (0) ;
								   return app; // this should never be reached
							   });
			return app;
		};

	app[WM_SYSCOMMAND] =
		[&app] (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> DWORD {
			logf ();
			switch (wParam & 0xfff0) {
			case SC_MINIMIZE:
			case SC_CLOSE:
				app.minimize ();
				break;
			}
			return app;
		};

	app[WM_NCHITTEST] =
		[&app] (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> DWORD {
			logf ();
			UINT uHitTest = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
			if(uHitTest == HTCLIENT)
				return HTCAPTION;
			else
				return uHitTest;
		};

	app[WM_TRAYICON] =
		[&app] (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> DWORD {
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

			//SendMessage (hwnd, WM_NULL, 0, 0);
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

	return app.loop(); //msg.wParam;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return g_app->handle (hwnd, message, wParam, lParam);
}

void uniform_windows (mapwin_t & windows)
{
	for (auto & item : windows) {
		logp (sys::e_debug, "The window is: "
			  << item.second._hwnd << ", "
			  << item.second._deserialized << ", '"
			  << item.second._class_name << "', '"
			  << item.second._title << "'.");
		for (auto & other : windows) {
			if (/* item != other and */
				! other.second._deserialized and
				other.second._class_name == item.second._class_name and
				other.second._title == item.second._title)
			{
				win_t & fakew = item.second; // from file
				win_t & realw = other.second; // from OS
				logp (sys::e_debug, "Nomalize '"
					  << realw._class_name << "', '"
					  << realw._title << "' with '"
					  << fakew._class_name << "', '"
					  << fakew._title << "'.");
				realw._place.length = sizeof(WINDOWPLACEMENT);
				realw._place.flags = fakew._place.flags;
				realw._place.showCmd = fakew._place.showCmd;
				realw._place.ptMinPosition.x = fakew._place.ptMinPosition.x;
				realw._place.ptMinPosition.y = fakew._place.ptMinPosition.y;
				realw._place.ptMaxPosition.x = fakew._place.ptMaxPosition.x;
				realw._place.ptMaxPosition.y = fakew._place.ptMaxPosition.y;
				realw._place.rcNormalPosition.top = fakew._place.rcNormalPosition.top;
				realw._place.rcNormalPosition.left = fakew._place.rcNormalPosition.left;
				realw._place.rcNormalPosition.right = fakew._place.rcNormalPosition.right;
				realw._place.rcNormalPosition.bottom = fakew._place.rcNormalPosition.bottom;
				break;
			}
		}
	}
	mapwin_t::iterator b = windows.begin();
	mapwin_t::iterator e = windows.end();

	for ( ; b != e; ) {
		if (b->second._deserialized) {
			logp (sys::e_debug, "Deleting '"
				  << b->second._class_name << "', "
				  << b->second._deserialized << ", as a deserialized window.");
			windows.erase (b++);
		} else {
			++b;
		}
	}
}

BOOL CALLBACK Enum (HWND hwnd, LPARAM lParam)
{
	mapwin_t & windows = *(mapwin_t *)lParam;

    if (IsAltTabWindow(hwnd) && IsWindowVisible(hwnd)) {
		const int BUF_SIZE = 1024;
		char class_name[BUF_SIZE];

		get_class_name (hwnd, (LPSTR)class_name, BUF_SIZE);
		if (discard_window_app_frame((const char *)class_name, ::strlen(class_name)))
			return TRUE;

		win_t win;
        CHAR buf[260];

		win._hwnd = hwnd;
        GetWindowTextA(hwnd, buf, ARRAYSIZE(buf));
		win._title = buf;
		win._class_name = class_name;
		logp (sys::e_debug, "Adding window with class '" << class_name << "'.");
		get_window_placement (hwnd, win._place);
		windows[win._hwnd] = win;
		//show_status (win._place.showCmd);
		//show_position (&win._place.rcNormalPosition);
    }
    return TRUE;
}

bool get_window_placement (HWND hwnd, WINDOWPLACEMENT & place)
{
	place.length = sizeof(place);
	return GetWindowPlacement(hwnd, &place);
}

bool get_class_name (HWND hwnd, LPSTR buf, INT buf_size)
{
	bool result = true;
	UINT length = GetClassNameA(hwnd, buf, buf_size);

	if (! length) {
		result = false;
	}

	return result;
}

bool discard_window_app_frame (const char * class_name, INT buf_size)
{
	const char * cn = "ApplicationFrameWindow";

	if (::strncmp((const char *)class_name, cn, sys::amin(::strlen(class_name), ::strlen(cn))) == 0)
		return true;

	return false;
}

BOOL IsAltTabWindow(HWND hwnd)
{
    LONG_PTR exStyles = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    // Start at the root owner
    HWND hwndWalk = hwnd;
    // appwindows treated as no owner, so don't try and find one
    if(!(exStyles & WS_EX_APPWINDOW))
    {
        hwndWalk = GetAncestor(hwnd, GA_ROOTOWNER);
    }

    // See if we are the last active visible popup
    HWND hwndTry = hwndWalk;
    while ((hwndTry = GetLastActivePopup(hwndWalk)) != hwndTry) {
        if (IsWindowVisible(hwndTry)) break;
        hwndWalk = hwndTry;
    }
    // tool windows are treated as not visible so they'll never appear in the list
    // fail them here
    return (hwndWalk == hwnd) && !(exStyles & WS_EX_TOOLWINDOW);
}

void show_status (UINT flag)
{
	switch (flag) {
	case SW_HIDE:
		std::cout << "Status: SW_HIDE" << std::endl;
		break;
	case SW_MAXIMIZE:
		// case SW_SHOWMAXIMIZED:
		std::cout << "Status: SW_MAXIMIZE" << std::endl;
		// std::cout << "Status: SW_SHOWMAXIMIZED" << std::endl;
		break;
	case SW_MINIMIZE:
		std::cout << "Status: SW_MINIMIZE" << std::endl;
		break;
	case SW_RESTORE:
		std::cout << "Status: SW_RESTORE" << std::endl;
		break;
	case SW_SHOW:
		std::cout << "Status: SW_SHOW" << std::endl;
		break;
	case SW_SHOWMINIMIZED:
		std::cout << "Status: SW_SHOWMINIMIZED" << std::endl;
		break;
	case SW_SHOWMINNOACTIVE:
		std::cout << "Status: SW_SHOWMINNOACTIVE" << std::endl;
		break;
	case SW_SHOWNA:
		std::cout << "Status: SW_SHOWNA" << std::endl;
		break;
	case SW_SHOWNOACTIVATE:
		std::cout << "Status: SW_SHOWNOACTIVATE" << std::endl;
		break;
	case SW_SHOWNORMAL:
		std::cout << "Status: SW_SHOWNORMAL" << std::endl;
		break;
	default:
		std::cout << "Status: unknown" << std::endl;
		break;
	}
}

void show_position (RECT * rect)
{
	std::cout << "Window position: "
			  << "\n top: " << rect->top
			  << "\n left: " << rect->left
			  << "\n right: " << rect->right
			  << "\n bottom: " << rect->bottom
			  << std::endl;

}

serializer::serializer (mapwin_t & map)
	: _mapwin (map)
{
}

bool serializer::operator () (std::string file_name)
{
	std::ofstream output (file_name, std::ios::out);
	mapwin_t::iterator b = _mapwin.begin();
	mapwin_t::iterator e = _mapwin.end();

	output << "[\n";
	for ( ; b != e; ++b) {
		if (b != _mapwin.begin())
			output << ",\n";
		output << "{ \"class\" : \"" << b->second._class_name << "\",\n"
			   << "\t\"data\" : {\n"
			   << "\t\t\"title\" : \"" << b->second._title << "\", \n"
			   << "\t\t\"flags\" : " << b->second._place.flags << ", \n"
			   << "\t\t\"show\" : " << b->second._place.showCmd << ", \n"
			   << "\t\t\"min_position\" : { \"x\" : " << b->second._place.ptMinPosition.x << ", "
			   << "\"y\" : " << b->second._place.ptMinPosition.y << "}, \n"
			   << "\t\t\"max_position\" : { \"x\" : " << b->second._place.ptMaxPosition.x << ", "
			   << "\"y\" : " << b->second._place.ptMaxPosition.y << "}, \n"
			   << "\t\t\"placement\" : { \"top\" : " << b->second._place.rcNormalPosition.top << ", "
			   << "\"left\" : " << b->second._place.rcNormalPosition.left << ", "
			   << "\"bottom\" : " << b->second._place.rcNormalPosition.bottom << ", "
			   << "\"right\" : " << b->second._place.rcNormalPosition.right << " } } }";
	}
	output << "\n]" << std::endl;

	return true;
}

bool serializer::deserialize (std::string file_name, mapwin_t & windows)
{
	ds::deserializer_t des (file_name, windows);

	logp (sys::e_debug, "Deserializer created.");
	if (! des) {
		logp (sys::e_debug, "Can't operate on file.");
		return false;
	}
	logp (sys::e_debug, "Calling deserializer to deserialize.");
	if (! des())
		return false;

	return true;
}


#if 0
	// I want to be notified when windows explorer
	// crashes and re-launches the taskbar.  the WM_TASKBARCREATED
	// event will be sent to my WndProc() AUTOMATICALLY whenever
	// explorer.exe starts up and fires up the taskbar again.
	// So its great, because now, even if explorer crashes,
	// I have a way to re-add my system tray icon in case
	// the app is already in the "minimized" (hidden) state.
	// if we did not do this an explorer crashed, the application
	// would remain inaccessible!!
	WM_TASKBARCREATED = RegisterWindowMessageA("TaskbarCreated") ;

	// add a console, because I love consoles.
	// To disconnect the console, just comment out
	// the next 3 lines of code.
	//AllocConsole ();
	//AttachConsole (GetCurrentProcessId());
	//freopen ("CON", "w", stdout);
	WNDCLASSEX wnd = { 0 };
	g_hInstance = hInstance;
	wnd.hInstance = hInstance;
	wnd.lpszClassName = c_class_name;
	wnd.lpfnWndProc = WndProc;
	wnd.style = CS_HREDRAW | CS_VREDRAW ;
	wnd.cbSize = sizeof (WNDCLASSEX);

	wnd.hIcon = LoadIcon (NULL, IDI_APPLICATION);
	wnd.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
	wnd.hCursor = LoadCursor (NULL, IDC_ARROW);
	wnd.hbrBackground = (HBRUSH)COLOR_APPWORKSPACE ;

	if (!RegisterClassEx(&wnd)) {
		FatalAppExit( 0, TEXT("Couldn't register window class!") );
	}

	g_hwnd = CreateWindowEx(
		0, c_class_name,
		TEXT( "Using the system tray" ),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		400, 400,
		NULL, NULL,
		hInstance, NULL
		);

	// Add the label with instruction text
	CreateWindow (TEXT("static"), TEXT("right click the system tray icon to close"),
				  WS_CHILD /* | WS_VISIBLE */ | SS_CENTER,
                  0, 0, 400, 400, g_hwnd, 0, hInstance, NULL);

	// Initialize the NOTIFYICONDATA structure once
	InitNotifyIconData ();

	Shell_NotifyIcon (NIM_ADD, &g_notifyIconData);
	//ShowWindow (g_hwnd, iCmdShow);

	MSG msg;
	while (GetMessage (&msg, NULL, 0, 0)) {
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}


	// Once you get the quit message, before exiting the app,
	// clean up and remove the tray icon
	if( !IsWindowVisible(g_hwnd)) {
		Shell_NotifyIcon (NIM_DELETE, &g_notifyIconData);
	}
#endif
