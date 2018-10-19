//-*- mode: c++; indent-tabs-mode: t; -*-
//
// Class: window Copyright (c) 2018 Manuel Cano
// Author: manutalcual@gmail.com
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
#ifndef window_h
#define window_h
#include <functional>
#include <map>
#include "resource.h"
#include "common.hh"
#include "window.hh"

namespace mcm {

	using FuncProc = LRESULT CALLBACK (*)(HWND, UINT, WPARAM, LPARAM);

	const char * get_msg (UINT msg);

	template<const char * ClassName,
			 FuncProc WndProc,
			 int Style,
			 const char * WindowTitle>
	class window
	{
		using Func = std::function<DWORD(HWND, UINT, WPARAM, LPARAM)>;
	public:
		window (HINSTANCE instance,
				HINSTANCE previous,
				LPSTR args,
				int cmdShow)
			: _class{0},
			  _instance (instance),
			  _previous (previous),
			  _args (args),
			  _cmd_show (cmdShow),
			  _hwnd{0},
			  _taskbar_created_msg{0},
			  _menu{0},
			  _notify_icon_data{0},
			  _ready{false}
		{
			logf ();
			_class.lpszClassName = ClassName;
			_class.lpfnWndProc = WndProc;
			_class.style = Style;
			_class.cbSize = sizeof(WNDCLASSEX);

			if (!RegisterClassEx(&_class)) {
				logp (sys::e_debug, "Can't register window class.");
				DWORD error = GetLastError();
				LPVOID lpMsgBuf;
				FormatMessage(
					FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					error,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR) &lpMsgBuf,
					0, NULL );
				logp (sys::e_debug, "Error: " << (char *)lpMsgBuf);
				FatalAppExit (0, TEXT("Can't register windows class."));
			}
		}

		~window ()
		{
			logf ();
			if( !IsWindowVisible(_hwnd)) {
				Shell_NotifyIcon (NIM_DELETE, &_notify_icon_data);
			}
		}

		operator bool ()
		{
			return _ready;
		}

		bool create ()
		{
			_hwnd = CreateWindowEx(
				0, ClassName,
				WindowTitle,
				WS_OVERLAPPEDWINDOW /*| WS_VISIBLE */,
				/* CW_USEDEFAULT, CW_USEDEFAULT, */
				0, 0,
				400, 400,
				NULL, NULL,
				_instance, NULL);
			return _hwnd != 0;
		}

		window & register_for_taskbar_message ()
		{
			logp (sys::e_debug, "Registering taskbar creation message.");
			_taskbar_created_msg = RegisterWindowMessageA("TaskbarCreated");
			return *this;
		}

		template<UINT Flags,
				 UINT AppIcon,
				 UINT Message,
				 const char * IconText>
		window & add_taskbar_icon ()
		{
			logp (ss::e_debug, "Adding taskbar icon");
			_notify_icon_data.cbSize = sizeof(NOTIFYICONDATA);
			_notify_icon_data.hWnd = _hwnd;
			_notify_icon_data.uID = AppIcon; //ID_TRAY_APP_ICON;
			_notify_icon_data.uFlags = Flags; //NIF_ICON | NIF_MESSAGE | NIF_TIP;
			_notify_icon_data.uCallbackMessage = Message; //WM_TRAYICON;
			_notify_icon_data.hIcon = (HICON)LoadIcon(_instance,
													  MAKEINTRESOURCE(AppIcon /*ID_TRAY_APP_ICON*/));
			stringcopy(_notify_icon_data.szTip, IconText);
			Shell_NotifyIcon (NIM_ADD, &_notify_icon_data);
			return *this;
		}

		LRESULT handle (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			logp (sys::e_debug, "Message received: "
				  << message
				  << "='" << get_msg(message) << "', "
				  << wParam << ", " << lParam << "'.");

			switch (message) {
			case WM_NULL:
				logp (sys::e_debug, "Null message received.");
				return 0;
				break;
			case WM_CREATE:
				logp (sys::e_debug, "WM_CREATE message received.");
				_ready = true;
				if (! _funcmap[message] (hwnd, message, wParam, lParam)) {
					logp (sys::e_debug, "Error handling message: " << message << ".");
				}
				break;
			case WM_SYSCOMMAND:
				logp (sys::e_debug, "WM_SYSCOMMAND message received.");
				if (! _funcmap[message] (hwnd, message, wParam, lParam)) {
					logp (sys::e_debug, "Error handling message: " << message << ".");
				}
				break;
			case WM_TRAYICON:
				logp (sys::e_debug, "WM_TRAYICON message received.");
				if (lParam == WM_LBUTTONUP) {
					_icon_click (hwnd, message, wParam, lParam);
				} else if (lParam == WM_RBUTTONDOWN) {
					_funcmap[_funcmap[message](hwnd, message, wParam, lParam)]
						(hwnd, message, wParam, lParam);
				}
				break;
			case WM_NCHITTEST:
				logp (sys::e_debug, "WM_NCHITEST message received.");
				_funcmap[message](hwnd, message, wParam, lParam);
				break;
			case WM_CLOSE:
				logp (sys::e_debug, "WM_CLOSE message received.");
				return 0;
				break;
			case WM_DESTROY:
				logp (sys::e_debug, "WM_DESTROY message received.");
				PostQuitMessage (0);
				break;
			case WM_SETFOCUS:
				logp (sys::e_debug, "WM_SETFOCUS message received.");
				break;
			case WM_MENUSELECT:
				logp (sys::e_debug, "WM_MENUSELECT message received.");
				return 0;
				break;
			default:
				logp (sys::e_debug, "Not handled message: " << message);
				break;
			}
			logp (sys::e_debug, "hwnd " << hwnd << ", " << message
				  << ", " << wParam << ", " << lParam << ".");
			return DefWindowProc(hwnd, message, wParam, lParam);
		}

		window & minimize ()
		{
			Shell_NotifyIcon (NIM_ADD, &_notify_icon_data);
			ShowWindow (_hwnd, SW_HIDE);
			return *this;
		}

		window & restore ()
		{
			ShowWindow (_hwnd, SW_SHOW);
			return *this;
		}


		window & create_menu (Func func)
		{
			_menu = CreatePopupMenu();
			_icon_click = func;
			return *this;
		}

		window & add_menu_item (UINT flags, UINT_PTR item, LPCTSTR text,
								Func func)
		{
			if (_menu) {
				AppendMenu (_menu, flags, item, text);
				_funcmap[item] = func;;
			}
			return *this;
		}

		window & add_menu_item (UINT flags, UINT_PTR item, LPCTSTR text)
		{
			if (_menu) {
				AppendMenu (_menu, flags, item, text);
			}
			return *this;
		}

		Func & operator [] (DWORD index)
		{
			return _funcmap[index];
		}

		HMENU get_menu_handle ()
		{
			return _menu;
		}

		WPARAM loop ()
		{
			MSG msg;
			while (GetMessage (&msg, NULL, 0, 0)) {
				TranslateMessage (&msg);
				DispatchMessage (&msg);
			}
			return msg.wParam;
		}
	private:
		WNDCLASSEX _class;
		HINSTANCE _instance;
		HINSTANCE _previous;
		LPSTR _args;
		int _cmd_show;
		HWND _hwnd;
		UINT _taskbar_created_msg;
		HMENU _menu;
		NOTIFYICONDATA _notify_icon_data;
		bool _ready;
		Func _icon_click;
		std::map<DWORD, Func> _funcmap;
	};

} // namespace mcm

#endif // window_h
