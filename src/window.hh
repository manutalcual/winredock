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

#include <winuser.h>
#include <dbt.h>
#include <cfgmgr32.h>
#include <winnt.h>

#include "dev.hh"
#include "poshandler.hh"

// disable size_t to int conversion warning
#pragma warning(disable:4267)

extern const char c_taskbar_icon_text[];

namespace mcm {

	using Func = std::function<DWORD(HWND, UINT, WPARAM, LPARAM)>;
	using FuncProc = LRESULT  (*)(HWND, UINT, WPARAM, LPARAM);
	extern Func noop; // = [](HWND, UINT, WPARAM, LPARAM) -> DWORD

	const char * get_msg (UINT msg);

	template<const char * ClassName,
			 FuncProc WndProc,
			 int Style,
			 const char * WindowTitle>
	class window
	{
		class callable
		{
		public:
			callable () : func (noop) {}
			callable (Func & f) : func (f) {}
			DWORD operator () (HWND a, UINT b, WPARAM c, LPARAM d)
			{
				return func(a,b,c,d);
			}

			Func func;
		};

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
			  _ready{false},
			  _timer (0),
			  _changing_resolution (false),
			  _repositioned (true),
			  _powersetting (false)
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
		}

		operator bool ()
		{
			return _ready;
		}

		bool create ()
		{
			logf ();
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
			
			// It's possible that User Interface Privilege Isolation may block the window
			// from receiving the message, so adjust our message filter.
			if (!ChangeWindowMessageFilterEx(_hwnd, _taskbar_created_msg, MSGFLT_ALLOW, NULL))
			{
				logp(sys::e_debug, "Error updating the window message filter.");
			}

			return *this;
		}

		template<UINT Flags,
				 UINT AppIcon,
				 UINT Message,
				 const char * IconText>
		window & add_taskbar_icon ()
		{
			logf ();
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
			logf ();
			logp (sys::e_debug, "Message received: "
				  << message
				  << "='" << get_msg(message) << "', "
				  << wParam << ", " << lParam << "'.");

			switch (message) {
			case WM_NULL:
				logp (sys::e_debug, "Null message received.");
				return 0;
				break;
			case WM_CREATE: {
				logp (sys::e_debug, "WM_CREATE message received.");
				logp (sys::e_debug, "Capturing initial configuration.");
				dev d;
				std::string config_name = sys::itoa((int)d.width());
				config_name += "_";
				config_name += sys::itoa(d.height());
				config_name += "_";
				config_name += sys::itoa(d.monitors());
				logp (sys::e_debug, "** Initial configuration name: '"
					  << config_name << "'.");
				_repos[config_name].get_windows ();
				_last_screen = d;
				logp (sys::e_debug, "--------------------------------");
				_ready = true;

				if (! _funcmap[message] (hwnd, message, wParam, lParam)) {
					logp (sys::e_debug, "Error handling message: " << message << ".");
				}
			}
				break;
			case WM_SYSCOMMAND:
				logp (sys::e_debug, "WM_SYSCOMMAND message received.");
				if (! _funcmap[message] (hwnd, message, wParam, lParam)) {
					logp (sys::e_debug, "Error handling message: " << message << ".");
				}
				break;
			case WM_COMMAND:
				logp (sys::e_debug, "WM_COMMAND message received.");
				break;
			case WM_TRAYICON:	//
				nlogp (sys::e_debug, "WM_TRAYICON message received.");
				if (lParam == WM_LBUTTONUP) {
					_icon_click (hwnd, message, wParam, lParam); //
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
				KillTimer(hwnd, _timer);
				Shell_NotifyIcon(NIM_DELETE, &_notify_icon_data);
				PostQuitMessage (0);
				break;
			case WM_SETFOCUS:
				logp (sys::e_debug, "WM_SETFOCUS message received.");
				break;
			case WM_MENUSELECT:
				logp (sys::e_debug, "WM_MENUSELECT message received.");
				return 0;
				break;
			case WM_DISPLAYCHANGE: {
				logp (sys::e_debug, "WM_DISPLAYCHANGE message received.");
				dev d;
				std::string config_name = mcm::sys::itoa(d.width());
				config_name += "_";
				config_name += mcm::sys::itoa(d.height());
				config_name += "_";
				config_name += mcm::sys::itoa(d.monitors());
				logp (sys::e_debug, "Current configuration: " << config_name);
			}
				break;
			case WM_TIMER: {
				logp (sys::e_debug, "Receive WM_TIMER event.");
				logp (sys::e_debug, "Actual screen: ");
				dev d;
				d.print ();
				logp (sys:::e_debug, "Last screen: ");
				_last_screen.print ();
				if (d != _last_screen) {
					logp (sys::e_debug, "Changing resolution.");
					std::string config_name = sys::itoa(d.width());
					config_name += "_";
					config_name += sys::itoa(d.height());
					config_name += "_";
					config_name += sys::itoa(d.monitors());
					if (_repos.find(config_name) != _repos.end()) {
						poshandler & repo = _repos[config_name];
						logp (sys::e_debug, "Repositioning windows: '"
							  << config_name << "'.");
						_repos[config_name].reposition(config_name);
					} else {
						logp (sys::e_debug, "Getting new configuration: '"
							  << config_name << "'.");
						_repos[config_name].get_windows ();
					}
					_last_screen = d;
					_screen_size = d;
					logp (sys::e_debug, "Last screen set to: ");
					_last_screen.print ();
				} else if (d == _last_screen) {
					std::string config_name = sys::itoa(d.width());
					config_name += "_";
					config_name += sys::itoa(d.height());
					config_name += "_";
					config_name += sys::itoa(d.monitors());
					logp (sys::e_debug, "Getting configuration: '"
						  << config_name << "'.");
					_repos[config_name].get_windows ();
					_last_screen = d;
				} 
				/*  
				This code is never called because _screen_size is always equal to _last_screen,
				so the case where the current config differs is handled above.
				
				Since _changing_resolution is only potentially set in WM_DEVICECHANGE, this allows us 
				to remove all of the device notification code and WM_DEVICECHANGE handling.

				Note there were already some issues surrounding the device notification code:
					1. _changing_resolution is set in WM_DEVICECHANGE, but never unset.
					2. RegisterDeviceNotification() is called for each known device, but the class has
					   only one notification handle, which gets overwritten each time Register...() is called.
					   UnregisterDeviceNotification() should be called for each handle, but is only ever called
					   on the last handle.

				} else if (d == _screen_size && _changing_resolution) {

					std::string config_name = sys::itoa(d.width());
					config_name += "_";
					config_name += sys::itoa(d.height());
					config_name += "_";
					config_name += sys::itoa(d.monitors());
					logp (sys::e_debug,
						  "Getting configuration with spurious device notification: '"
						  << config_name << "'.");
					_repos[config_name].get_windows ();
					_last_screen = d;
				} */
				return 0;
			}
				break;
			default:
				if (message == _taskbar_created_msg)
				{
					add_taskbar_icon<NIF_ICON | NIF_MESSAGE | NIF_TIP, ID_TRAY_APP_ICON, WM_TRAYICON, c_taskbar_icon_text>();
				}
				else
				{
					nlogp(sys::e_debug, "Not handled message: " << message);
				}
				break;
			}
			nlogp (sys::e_debug, "hwnd " << hwnd << ", " << message
				   << ", " << wParam << ", " << lParam << ".");
			return DefWindowProc(hwnd, message, wParam, lParam);
		}

		window & start_timer()
		{
			_timer = SetTimer(_hwnd, 1, 1000, NULL);
			if (!_timer) {
				logp(sys::e_debug, "Can't create timer.");
			}
			else {
				logp(sys::e_debug, "Timer created: "
					<< _timer);
			}
			return *this;
		}

		window & create_menu (Func func)
		{
			logf ();
			_menu = CreatePopupMenu();
			_icon_click = func;
			return *this;
		}

		window & add_menu_item (UINT flags, UINT_PTR item, LPCTSTR text,
								Func func)
		{
			if (_menu) {
				AppendMenu (_menu, flags, item, text);
				_funcmap[(unsigned long)item] = func;;
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
			return _funcmap[index].func;
		}

		HMENU get_menu_handle ()
		{
			return _menu;
		}

		WPARAM loop ()
		{
			MSG msg;
			while (GetMessage (&msg, NULL, 0, 0)) {
				if (msg.message == WM_TIMER) {
					msg.hwnd = _hwnd;
				}
				TranslateMessage (&msg);
				DispatchMessage (&msg);
			}
			return msg.wParam;
		}
	private:
		typedef std::map<std::string, mcm::poshandler> maprepohandlers_t;
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
		std::map<DWORD, callable> _funcmap;
		dev _screen_size;
		dev _last_screen;
		UINT_PTR _timer;
		bool _changing_resolution;
		bool _repositioned;
		maprepohandlers_t _repos;
		bool _powersetting;
	};

} // namespace mcm

#endif // window_h
