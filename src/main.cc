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

const char c_class_name[] = "WinReDock";
const char c_window_title[] = "WinReDock - restore windows to pre-undock positions";
const char c_taskbar_icon_text[] = "WinReDock -- tooling; dockerify after undock!";
mcm::window<c_class_name,
			WndProc,
			(CS_HREDRAW | CS_VREDRAW),
			c_window_title> * g_app = nullptr;

std::string file_name{"window_list.json"};

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR args, int iCmdShow )
{
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
					//positioner.reposition ();
					return app;
				}) // Context menus and functions
				/*
				  There is no use for menu entries.
				  In previous versions window repositioning was done
				  by menu clicking, now it is automatic, so these
				  are no more useful. I left them here just becaus I
				  have some plans for the future that involve menu
				  handling and I don´t want to rewrite them.
				 */
				.add_menu_item(MF_STRING,
							   ID_TRAY_LOAD_WINDOWS_MENU,
							   TEXT("Get windows"),
							   [&] (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> DWORD {
								   logp (sys::e_debug, "Call to load windows.");
								   //positioner.get_windows ();
								   return app;
							   })
				.add_menu_item(MF_STRING,
							   ID_TRAY_SAVE_MENU,
							   TEXT("Save config."),
							   [&] (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> DWORD {
								   logp (sys::e_debug, "Calling to serialize data.");
								   //positioner.save_configuration (file_name);
								   logp (sys::e_debug, "Ok. Done.");
								   return app;
							   })
				.add_menu_item(MF_STRING,
							   ID_TRAY_LOAD_MENU,
							   TEXT("Read config."),
							   [&] (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> DWORD {
								   logp (sys::e_debug, "Calling to read file.");
								   //positioner.load_configuration (file_name);
								   return app;
							   })
				.add_menu_item(MF_SEPARATOR, 0, NULL)
				/* Exit menu does something */
				.add_menu_item(MF_STRING,
							   ID_TRAY_EXIT_CONTEXT_MENU_ITEM,
							   TEXT("Exit"),
							   [&] (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> DWORD {
								   logp (sys::e_debug, "Calling to exit app.");
								   PostQuitMessage (0) ;
								   return app; // this should never be reached
							   })
				.register_all_guids ();
			return app;
		};

	/*
	  These are Windows operating system messages we react on.
	*/

	app[WM_SYSCOMMAND] =
		[&] (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> DWORD {
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
		[&] (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> DWORD {
			logf ();
			LRESULT uHitTest = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
			if(uHitTest == HTCLIENT)
				return HTCAPTION;
			else
				return (DWORD)uHitTest;
		};

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

	return (int) app.loop(); //msg.wParam;
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
