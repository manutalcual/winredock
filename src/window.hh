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

namespace mcm {

	extern GUID power;

	using Func = std::function<DWORD(HWND, UINT, WPARAM, LPARAM)>;
	//using FuncProc = LRESULT CALLBACK (*)(HWND, UINT, WPARAM, LPARAM);
	using FuncProc = LRESULT (*)(HWND, UINT, WPARAM, LPARAM);
	//typedef LRESULT (*FuncProc)(HWND, UINT, WPARAM, LPARAM); // CALLBACK
	extern Func noop; // = [](HWND, UINT, WPARAM, LPARAM) -> DWORD

	const char * get_msg (UINT msg);

	//TIMERPROC Timerproc;

	void Timerproc (HWND Arg1, UINT Arg2, UINT_PTR Arg3, DWORD Arg4);

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
		window (const char* class_name,
				FuncProc WndProc,
				int Style,
				const char* window_title,
				HINSTANCE instance,
				HINSTANCE previous,
				LPSTR args,
				int cmdShow)
			: _class{0},
			  _instance (instance),
			  _previous (previous),
			  _args (args),
			  _cmd_show (cmdShow),
			  _hwnd{0},
			  _class_name (class_name),
			  _window_title (window_title),
			  _taskbar_created_msg{0},
			  _menu{0},
			  _notify_icon_data{0},
			  _ready{false},
			  _timer (0),
			  _changing_resolution (false),
			  _repositioned (true),
			  _powersetting (false)
		{
			nlogf ();
			_class.lpszClassName = _class_name;
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
			nlogf ();
			if( !IsWindowVisible(_hwnd)) {
				Shell_NotifyIcon (NIM_DELETE, &_notify_icon_data);
			}
			if (KillTimer(_hwnd, 1)) {
				logp (sys::e_debug, "Timer killed.");
			} else {
				logp (sys::e_debug, "Can't kill timer.");
				if (KillTimer(_hwnd, _timer)) {
					logp (sys::e_debug, "Second try killing the timer failed also.");
				} else {
					logp (sys::e_debug, "Timer killed on second try.");
				}
			}
		}

		operator bool ()
		{
			return _ready;
		}

		bool create ()
		{
			nlogf ();
			_hwnd = CreateWindowExA(
				0, _class_name,
				_window_title,
				WS_OVERLAPPEDWINDOW /*| WS_VISIBLE */
				/* CW_USEDEFAULT, CW_USEDEFAULT, */
				,0, 0,
				400, 400,
				NULL, NULL,
				_instance, NULL);
			return _hwnd != 0;
		}

		window & register_for_taskbar_message ()
		{
			nlogp (sys::e_debug, "Registering taskbar creation message.");
			_taskbar_created_msg = RegisterWindowMessageA("TaskbarCreated");
			return *this;
		}

		window & add_taskbar_icon (UINT Flags,
			UINT AppIcon,
			UINT Message,
			const char * IconText)
		{
			nlogf ();
			nlogp (ss::e_debug, "Adding taskbar icon");
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

		void register_all_guids ()
		{
			nlogf ();
			CONFIGRET ret { 0 };

			logp (sys::e_debug, "Eumerate GUIDs.");
			for (size_t i = 0; ret != CR_NO_SUCH_VALUE; ++i) {
				GUID guid;
				ret = CM_Enumerate_Classes(i,
										   &guid,
										   0);
				if (ret != CR_NO_SUCH_VALUE) {
					nlogp (sys::e_debug, "Registering for GUID: " << mcm::guid_to_string(&guid));
					register_notification (&guid);
				}
			}
			_timer = SetTimer (_hwnd, 1, 1500, (TIMERPROC)NULL);
			if (! _timer) {
				logp (sys::e_debug, "Can't create timer.");
			} else {
				logp (sys::e_debug, "Timer created: "
					  << _timer);
			}
		}

		LRESULT handle (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			nlogf ();
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
				std::string config_name = sys::itoa(d.width());
				config_name += "_";
				config_name += sys::itoa(d.height());
				config_name += "_";
				config_name += sys::itoa(d.monitors());
				logp (sys::e_debug, "** Initial configuration name: '"
					  << config_name << "'.");
				_repo.get_windows ();
				_last_screen = d;
				logp (sys::e_debug, "--------------------------------");
				_ready = true;

				RegisterPowerSettingNotification (_hwnd,
												  &power,
												  DEVICE_NOTIFY_WINDOW_HANDLE);

				if (! _funcmap[message] (hwnd, message, wParam, lParam)) {
					logp (sys::e_debug, "Error handling message: " << message << ".");
				}
				_changing_resolution = false;
				break;
			}
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
				if (_hdev_notify) {
					if (! UnregisterDeviceNotification(_hdev_notify)) {
						logp (sys::e_deug, "UnregisterDeviceNotification failed");
					}
				}
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
			case WM_DISPLAYCHANGE: {
				logp (sys::e_debug, "WM_DISPLAYCHANGE message received.");
				_changing_resolution = true;
				_repo.changing_resolution(_changing_resolution);
				mcm::poshandler::conf_t config_name;
				logp (sys::e_debug, "Current configuration: " << static_cast<std::string>(config_name));
				break;
			}
			case WM_TIMER: {
				logp (sys::e_debug, "Receive WM_TIMER event.");
				logp(sys:::e_debug, "Last screen: ");
				_last_screen.print();
				dev d;
				logp(sys::e_debug, "Current screen: ");
				d.print ();
				mcm::poshandler::conf_t config_name;
				if (! _repo.has_resolution(config_name)) {
					logp(sys::e_debug, "[0] New resolution.");
					_changing_resolution = false;
					_repo.changing_resolution(_changing_resolution);
					_repo.get_windows();
					_last_screen = d;
					_screen_size = d;
				} else if (_changing_resolution && d != _last_screen) {
					logp(sys::e_debug, "[1] Changing resolution.");
					_repo.reposition();
					_last_screen = d;
					_screen_size = d;
					_changing_resolution = false;
					_repo.changing_resolution(_changing_resolution);
				} else if (d != _last_screen) {
					logp (sys::e_debug, "[2] Changing resolution (spurious.)");
					_repo.reposition ();
					_last_screen = d;
					_screen_size = d;
					_changing_resolution = false;
					_repo.changing_resolution(_changing_resolution);
				} else if (d == _last_screen) {
					logp(sys::e_debug, "[3] No resolution change.");
					_repo.get_windows ();
					_last_screen = d;
				}
				return 0;
				break;
			}
			case WM_DEVICECHANGE: {
				logp (sys::e_debug, "WM_DEVICECHANGE received!!!!");
				// Output some messages to the window.
				switch (wParam)
				{
				case DBT_DEVICEARRIVAL:
					logp(sys::e_debug, "Message: DBT_DEVICEARRIVAL");
					break;
				case DBT_DEVICEREMOVECOMPLETE:
					logp(sys::e_debug, "Message: DBT_DEVICEREMOVECOMPLETE");
					break;
				case DBT_DEVNODES_CHANGED: {
					logp (sys::e_debug, "Changing resolution...");
					//_changing_resolution = true;
					PDEV_BROADCAST_DEVICEINTERFACE b = (PDEV_BROADCAST_DEVICEINTERFACE) lParam;
					if (b) {
						logp (sys::e_debug, "Device param size: " << b->dbcc_size << ", "
							  << sizeof(DEV_BROADCAST_DEVICEINTERFACE));
						if (sizeof(*b) > sizeof(DEV_BROADCAST_DEVICEINTERFACE))
							logp (sys::e_debug, "Device name: " << b->dbcc_name);
						else
							logp (sys::e_debug, "Unkown device param type.");
					} else {
						logp (sys::e_debug, "There is no device identity param!");
						dev d;
						std::string config_name = mcm::sys::itoa(d.width());
						config_name += "_";
						config_name += mcm::sys::itoa(d.height());
						config_name += "_";
						config_name += mcm::sys::itoa(d.monitors());
						logp (sys::e_debug, "Current configuration: " << config_name);
					}
					break;
				}
				default:
					logp(sys::e_debug, "Message " << wParam << " unhandled: WM_DEVICECHANGE");
					break;
				}
				break;
			}
			case WM_POWERBROADCAST: {
				logp (sys::e_debug, "Received a WM_POWERBROADCAST");
				//_powersetting = !_powersetting;
				switch (wParam) {
				case PBT_POWERSETTINGCHANGE: {
					POWERBROADCAST_SETTING * p = (POWERBROADCAST_SETTING *)lParam;
					logp (sys::e_debug, "Changing powesettings.");
					break;
				}
				}
				break;
			}
			default:
				nlogp (sys::e_debug, "Not handled message: " << message);
				break;
			}
			nlogp (sys::e_debug, "hwnd " << hwnd << ", " << message
				   << ", " << wParam << ", " << lParam << ".");
			return DefWindowProc(hwnd, message, wParam, lParam);
		}

		window & minimize ()
		{
			nlogf ();
			Shell_NotifyIcon (NIM_ADD, &_notify_icon_data);
			ShowWindow (_hwnd, SW_HIDE);
			return *this;
		}

		window & restore ()
		{
			nlogf ();
			ShowWindow (_hwnd, SW_SHOW);
			return *this;
		}


		window & create_menu (Func func)
		{
			nlogf ();
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
		const char* _class_name;
		const char* _window_title;
		UINT _taskbar_created_msg;
		HMENU _menu;
		NOTIFYICONDATA _notify_icon_data;
		bool _ready;
		Func _icon_click;
		std::map<DWORD, callable> _funcmap;
		HDEVNOTIFY _hdev_notify;
		dev _screen_size;
		dev _last_screen;
		UINT_PTR _timer;
		bool _changing_resolution;
		bool _repositioned;
		//maprepohandlers_t _repos;
		mcm::poshandler _repo;
		bool _powersetting;

		bool register_notification (GUID * guid)
		{
			nlogf ();
			DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

			ZeroMemory (&NotificationFilter, sizeof(NotificationFilter));
			// sizeof(DEV_BROADCAST_HDR);
			NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
			NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
			//NotificationFilter.dbcc_devicetype = DBT_DEVTYP_HANDLE;
			NotificationFilter.dbcc_classguid = *guid;

			_hdev_notify = RegisterDeviceNotification(
				_hwnd,                       // events recipient
				&NotificationFilter,        // type of device
				DEVICE_NOTIFY_WINDOW_HANDLE // type of recipient
											// handle
				| DEVICE_NOTIFY_ALL_INTERFACE_CLASSES
				);

			if (NULL == _hdev_notify)
			{
				logp (sys::e_debug, "Register notification failed for GUID: "
					  << mcm::guid_to_string(guid));
				return FALSE;
			}
			return true;
		}

	};

} // namespace mcm

#endif // window_h
