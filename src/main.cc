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

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR args, int iCmdShow )
{
	TCHAR className[] = TEXT( "WindowsRestorer" );

	logp (sys::e_debu, "Creating window with class name: " << className);

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
	wnd.lpszClassName = className;
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
		0, className,
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

	return msg.wParam;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static mapwin_t windows;

	if (message == WM_TASKBARCREATED && !IsWindowVisible(g_hwnd)) {
		Minimize ();
		return 0;
	}

	serializer serial (windows);

	switch (message) {
	case WM_CREATE:
		// create the menu once.
		// oddly, you don't seem to have to explicitly attach
		// the menu to the HWND at all.  This seems so ODD.
		g_menu = CreatePopupMenu();

		AppendMenu (g_menu, MF_STRING, ID_TRAY_LOAD_WINDOWS_MENU, TEXT("Get windows"));
		AppendMenu (g_menu, MF_STRING, ID_TRAY_SAVE_MENU, TEXT("Save config."));
		AppendMenu (g_menu, MF_STRING, ID_TRAY_LOAD_MENU, TEXT("Read config."));
		AppendMenu (g_menu, MF_SEPARATOR, NULL, NULL);
		AppendMenu (g_menu, MF_STRING, ID_TRAY_EXIT_CONTEXT_MENU_ITEM,  TEXT("Exit"));

		logp (sys::e_debug, "Window WM_CREATEd, going to analyze windows positions.");

		serial.deserialize (FILE_NAME, windows);
		EnumWindows (&Enum, (LPARAM)&windows);
		uniform_windows (windows);

		break;

	case WM_SYSCOMMAND:
		// (filter out reserved lower 4 bits:  see msdn remarks
		// http://msdn.microsoft.com/en-us/library/ms646360(VS.85).aspx)
		switch (wParam & 0xfff0) {
		case SC_MINIMIZE:
		case SC_CLOSE:  // redundant to WM_CLOSE, it appears
			Minimize ();
			return 0;
			break;
		}
		break;

	case WM_TRAYICON:
    {
		switch(wParam) {
		case ID_TRAY_APP_ICON:
			break;
		}

		// the mouse button has been released.

		// I'd LIKE TO do this on WM_LBUTTONDOWN, it makes
		// for a more responsive-feeling app but actually
		// the guy who made the original post is right.
		// Most apps DO respond to WM_LBUTTONUP, so if you
		// restore your window on WM_LBUTTONDOWN, then some
		// other icon will scroll in under your mouse so when
		// the user releases the mouse, THAT OTHER ICON will
		// get the WM_LBUTTONUP command and that's quite annoying.
		if (lParam == WM_LBUTTONUP) {
			mapwin_t::iterator begin = windows.begin();
			mapwin_t::iterator end = windows.end();
			for (; begin != end; ++begin) {
				logp (sys::e_debug, "Setting placement for '"
					  << begin->second._title << "'");
				if (SetWindowPlacement(begin->second._hwnd, &begin->second._place)) {
					logp (sys::e_debug, "Can't set window placement for last window.");
				} else {
					ShowWindow (begin->second._hwnd, SW_MINIMIZE);
					ShowWindow (begin->second._hwnd, SW_SHOW);
				}
			}
		} else if (lParam == WM_RBUTTONDOWN) {
			// I'm using WM_RBUTTONDOWN here because
			// it gives the app a more responsive feel.  Some apps
			// DO use this trick as well.  Right clicks won't make
			// the icon disappear, so you don't get any annoying behavior
			// with this (try it out!)

			// Get current mouse position.
			POINT curPoint;
			GetCursorPos (&curPoint);

			// should SetForegroundWindow according
			// to original poster so the popup shows on top
			SetForegroundWindow (hwnd);

			// TrackPopupMenu blocks the app until TrackPopupMenu returns
			UINT clicked = TrackPopupMenu(
				g_menu,
				TPM_RETURNCMD | TPM_NONOTIFY, // don't send me
											  // WM_COMMAND messages
											  // about this window,
											  // instead return the
											  // identifier of the
											  // clicked menu item
				curPoint.x,
				curPoint.y,
				0,
				hwnd,
				NULL
				);

			// Original poster's line of code.  Haven't deleted it,
			// but haven't seen a need for it.
			//SendMessage(hwnd, WM_NULL, 0, 0); // send benign message
			// to window to make sure the menu goes away.
			if (clicked == ID_TRAY_EXIT_CONTEXT_MENU_ITEM) {
				// quit the application.
				PostQuitMessage (0) ;
			} else if (clicked == ID_TRAY_SAVE_MENU) {
				logp (sys::e_debug, "Calling to serialize data.");
				serializer serial (windows);
				serial (FILE_NAME);
				logp (sys::e_debug, "Ok. Done.");
			} else if (clicked == ID_TRAY_LOAD_WINDOWS_MENU) {
				logp (sys::e_debug, "Call to load windows.");
				windows.clear ();
				// Get windows opened
				EnumWindows (&Enum, (LPARAM)&windows);
			} else if (clicked == ID_TRAY_LOAD_MENU) {
				serial.deserialize (FILE_NAME, windows);
				EnumWindows (&Enum, (LPARAM)&windows);
				uniform_windows (windows);
			} else {
				logp (sys::e_debug, "Clicked " << clicked << " element.");
			}
		}
    }
    break;

	// intercept the hittest message.. making full body of
	// window draggable.
	case WM_NCHITTEST:
	{
		// http://www.catch22.net/tuts/tips
		// this tests if you're on the non client area hit test
		UINT uHitTest = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
		if(uHitTest == HTCLIENT)
			return HTCAPTION;
		else
			return uHitTest;
	}

	case WM_CLOSE:
		Minimize ();
		return 0;
		break;

	case WM_DESTROY:
		PostQuitMessage (0);
		break;

	}

	return DefWindowProc( hwnd, message, wParam, lParam ) ;
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

// Initialize the NOTIFYICONDATA structure.
// See MSDN docs http://msdn.microsoft.com/en-us/library/bb773352(VS.85).aspx
// for details on the NOTIFYICONDATA structure.
void InitNotifyIconData ()
{
	memset (&g_notifyIconData, 0, sizeof( NOTIFYICONDATA));

	g_notifyIconData.cbSize = sizeof(NOTIFYICONDATA);

	/////
	// Tie the NOTIFYICONDATA struct to our
	// global HWND (that will have been initialized
	// before calling this function)
	g_notifyIconData.hWnd = g_hwnd;
	// Now GIVE the NOTIFYICON.. the thing that
	// will sit in the system tray, an ID.
	g_notifyIconData.uID = ID_TRAY_APP_ICON;
	// The COMBINATION of HWND and uID form
	// a UNIQUE identifier for EACH ITEM in the
	// system tray.  Windows knows which application
	// each icon in the system tray belongs to
	// by the HWND parameter.
	/////

	/////
	// Set up flags.
	g_notifyIconData.uFlags = NIF_ICON | // promise that the hIcon
										 // member WILL BE A VALID
										 // ICON!!
		NIF_MESSAGE | // when someone clicks on the system tray icon,
		// we want a WM_ type message to be sent to our WNDPROC
		NIF_TIP;      // we're gonna provide a tooltip as well, son.

	g_notifyIconData.uCallbackMessage = WM_TRAYICON; //this message
													 //must be handled
													 //in hwnd's
													 //window
													 //procedure. more
													 //info below.

	// Load da icon.  Be sure to include an icon "icon.ico" .. get one
	// from the internet if you don't have an icon
	/*
	g_notifyIconData.hIcon = (HICON)LoadImage(NULL, TEXT("icon.ico"),
											  IMAGE_ICON, 0, 0, LR_LOADFROMFILE );
	*/
	g_notifyIconData.hIcon = (HICON)LoadIcon(g_hInstance, MAKEINTRESOURCE(ID_TRAY_APP_ICON));

	// set the tooltip text.  must be LESS THAN 64 chars
	stringcopy(g_notifyIconData.szTip, TEXT("Amadeus tooling: dockerify after undock!"));
}

void Minimize ()
{
	// add the icon to the system tray
	Shell_NotifyIcon (NIM_ADD, &g_notifyIconData);

	// ..and hide the main window
	ShowWindow (g_hwnd, SW_HIDE);
}

// Basically bring back the window (SHOW IT again)
// and remove the little icon in the system tray.
void Restore ()
{
	// Remove the icon from the system tray
	Shell_NotifyIcon (NIM_DELETE, &g_notifyIconData);

	// ..and show the window
	ShowWindow (g_hwnd, SW_SHOW);
}


BOOL CALLBACK Enum (HWND hwnd, LPARAM lParam)
{
	mapwin_t & windows = *(mapwin_t *)lParam;

    if (IsAltTabWindow(hwnd) && IsWindowVisible(hwnd)) {
		const int BUF_SIZE = 124;
		CHAR class_name[BUF_SIZE];

		get_class_name (hwnd, class_name, BUF_SIZE);
		if (discard_window_app_frame(class_name, ::strlen(class_name)))
			return TRUE;

		win_t win;
        CHAR buf[260];

		win._hwnd = hwnd;
        GetWindowText(hwnd, buf, ARRAYSIZE(buf));
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

bool get_class_name (HWND hwnd, LPTSTR buf, INT buf_size)
{
	bool result = true;
	UINT length = GetClassName(hwnd, buf, buf_size);

	if (! length) {
		result = false;
	}

	return result;
}

bool discard_window_app_frame (LPTSTR class_name, INT buf_size)
{
	const char cn[] = "ApplicationFrameWindow";

	if (::strncmp(class_name, cn, sys::min(::strlen(class_name), ::strlen(cn))) == 0)
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
