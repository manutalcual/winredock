//-*- mode: c++; indent-tabs-mode: t; -*-
//
// Class: poshandler Copyright (c) 2018 Manuel Cano
// Author: manutalcual@gmail.com
// Date: Tue Sep 18 14:33:22 2018
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
#include "poshandler.hh"

BOOL is_alt_tab_window(HWND hwnd)
{
    LONG_PTR exStyles = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    HWND hwndWalk = hwnd;

    if(!(exStyles & WS_EX_APPWINDOW))
    {
		nlogp(sys::_debug, "Not an APPWINDOW, getting ancestor.");
        hwndWalk = GetAncestor(hwnd, GA_ROOTOWNER);
    }

    HWND hwndTry = hwndWalk;
    while ((hwndTry = GetLastActivePopup(hwndWalk)) != hwndTry) {
		if (IsWindowVisible(hwndTry)) {
			nlogp(sys::e_debug, " this window is visible.");
			break;
		}
        hwndWalk = hwndTry;
    }
    // tool windows are treated as not visible, they'll appear in the
    // list
	nlogp(sys::e_debug, "Return window info: " << (hwndWalk == hwnd) << ", " << (exStyles & WS_EX_TOOLWINDOW));
    return (hwndWalk == hwnd) && !(exStyles & WS_EX_TOOLWINDOW);
}

BOOL CALLBACK Enum(HWND hwnd, LPARAM lParam)
{
	nlogf();
	static size_t count = 0;
	mcm::poshandler::handler_ref_t& poshand{ *((mcm::poshandler::handler_ref_t*)lParam) };

	if (poshand._changing) {
		logp(sys::e_debug, "Stop Enum windows because of a resolution change.");
		return false;
	}

	const int BUF_SIZE = 1024;
	char class_name[BUF_SIZE];
	::memset(class_name, 0, BUF_SIZE);

	mapwin_t& windows = poshand._windows;

	bool visible = IsWindowVisible(hwnd);
	bool alt_tab_win = is_alt_tab_window(hwnd);
	bool iconic = IsIconic(hwnd);
	bool zoomed = IsZoomed(hwnd);
	LONG_PTR exStyles = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
	LONG_PTR styles = GetWindowLongPtr(hwnd, GWL_STYLE);

	nlogp(sys::e_debug, "--- Enum " << ++count << " ---");
	mcm::poshandler::get_class_name(hwnd, (LPSTR)class_name, BUF_SIZE);
	bool frame_win = mcm::poshandler::discard_window_app_frame((const char*)class_name,
		::strlen(class_name));
	bool special{ false };
#if 1
	if ((styles& WS_POPUP) && !(exStyles& WS_EX_TOOLWINDOW) && visible) {
		alt_tab_win = true;
		frame_win = false;
		special = true;
	}
#endif
	nlogp(sys::e_debug, "-- Window "
		<< std::hex << hwnd << std::dec
		<< " '" << class_name
		<< "': visible " << visible
		<< ", alt tab " << alt_tab_win
		<< ", frame " << frame_win
		<< ", popup " << (styles & WS_POPUP)
		<< ", toolwin " << (exStyles & WS_EX_TOOLWINDOW)
		<< ", noactivate " << (exStyles & WS_EX_NOACTIVATE)
		<< ", zoomed " << zoomed
		<< ", iconic " << iconic);
	nlogp (sys::e_debug, "Enum: Get class name: '" << class_name << "'");

	if (alt_tab_win && visible && !(exStyles & WS_EX_NOACTIVATE) && !frame_win) {

		win_t& win = windows[hwnd];

		if (win._hwnd == 0) {
			CHAR buf[260];
			logp(sys::e_debug, "-- New window");
			win._hwnd = hwnd;
			GetWindowTextA(win._hwnd, buf, ARRAYSIZE(buf));
			win._title = buf;
			win._class_name = class_name;
		}
		logp(sys::e_debug, "-- Window "
			<< std::hex << hwnd << std::dec
			<< " '" << class_name
			<< "'");
		if (special) {
			logp(sys::e_debug, "   this is a special window.");
		}
		nlogp(sys::e_debug, "WS_OVERLAPPED " << (WS_OVERLAPPED & styles));
		nlogp(sys::e_debug, "WS_POPUP " << (WS_POPUP & styles));
		nlogp(sys::e_debug, "WS_CHILD " << (WS_CHILD & styles));
		nlogp(sys::e_debug, "WS_MINIMIZE " << (WS_MINIMIZE & styles));
		nlogp(sys::e_debug, "WS_VISIBLE " << (WS_VISIBLE & styles));
		nlogp(sys::e_debug, "WS_DISABLED " << (WS_DISABLED & styles));
		nlogp(sys::e_debug, "WS_CLIPSIBLINGS " << (WS_CLIPSIBLINGS & styles));
		nlogp(sys::e_debug, "WS_CLIPCHILDREN " << (WS_CLIPCHILDREN & styles));
		nlogp(sys::e_debug, "WS_MAXIMIZE " << (WS_MAXIMIZE & styles));
		nlogp(sys::e_debug, "WS_CAPTION " << (WS_CAPTION & styles));
		nlogp(sys::e_debug, "WS_BORDER " << (WS_BORDER & styles));
		nlogp(sys::e_debug, "WS_DLGFRAME " << (WS_DLGFRAME & styles));
		nlogp(sys::e_debug, "WS_VSCROLL " << (WS_VSCROLL & styles));
		nlogp(sys::e_debug, "WS_HSCROLL " << (WS_HSCROLL & styles));
		nlogp(sys::e_debug, "WS_SYSMENU " << (WS_SYSMENU & styles));
		nlogp(sys::e_debug, "WS_THICKFRAME " << (WS_THICKFRAME & styles));
		nlogp(sys::e_debug, "WS_GROUP " << (WS_GROUP & styles));
		nlogp(sys::e_debug, "WS_TABSTOP " << (WS_TABSTOP & styles));
		nlogp(sys::e_debug, "WS_MINIMIZEBOX " << (WS_MINIMIZEBOX & styles));
		nlogp(sys::e_debug, "WS_MAXIMIZEBOX " << (WS_MAXIMIZEBOX & styles));

		nlogp(sys::e_debug, "WS_EX_DLGMODALFRAME " << (WS_EX_DLGMODALFRAME & exStyles));
		nlogp(sys::e_debug, "WS_EX_NOPARENTNOTIFY " << (WS_EX_NOPARENTNOTIFY & exStyles));
		nlogp(sys::e_debug, "WS_EX_TOPMOST " << (WS_EX_TOPMOST & exStyles));
		nlogp(sys::e_debug, "WS_EX_ACCEPTFILES " << (WS_EX_ACCEPTFILES & exStyles));
		nlogp(sys::e_debug, "WS_EX_TRANSPARENT " << (WS_EX_TRANSPARENT & exStyles));
		nlogp(sys::e_debug, "WS_EX_MDICHILD " << (WS_EX_MDICHILD & exStyles));
		nlogp(sys::e_debug, "WS_EX_TOOLWINDOW " << (WS_EX_TOOLWINDOW & exStyles));
		nlogp(sys::e_debug, "WS_EX_WINDOWEDGE " << (WS_EX_WINDOWEDGE & exStyles));
		nlogp(sys::e_debug, "WS_EX_CLIENTEDGE " << (WS_EX_CLIENTEDGE & exStyles));
		nlogp(sys::e_debug, "WS_EX_CONTEXTHELP " << (WS_OVERLAPPED & exStyles));
		nlogp(sys::e_debug, "WS_EX_RIGHT " << (WS_EX_RIGHT & exStyles));
		nlogp(sys::e_debug, "WS_EX_LEFT " << (WS_EX_LEFT & exStyles));
		nlogp(sys::e_debug, "WS_EX_RTLREADING " << (WS_EX_RTLREADING & exStyles));
		nlogp(sys::e_debug, "WS_EX_LTRREADING " << (WS_EX_LTRREADING & exStyles));
		nlogp(sys::e_debug, "WS_EX_LEFTSCROLLBAR " << (WS_EX_LEFTSCROLLBAR & exStyles));
		nlogp(sys::e_debug, "WS_EX_RIGHTSCROLLBAR " << (WS_EX_RIGHTSCROLLBAR & exStyles));
		nlogp(sys::e_debug, "WS_EX_CONTROLPARENT " << (WS_EX_CONTROLPARENT & exStyles));
		nlogp(sys::e_debug, "WS_EX_STATICEDGE " << (WS_EX_STATICEDGE & exStyles));
		nlogp(sys::e_debug, "WS_EX_APPWINDOW " << (WS_EX_APPWINDOW & exStyles));


		mcm::poshandler::conf_t config_name;
		mcm::poshandler::get_window_placement(win._hwnd, win._places[config_name]._place);
		DPI_AWARENESS_CONTEXT dpi_context = GetWindowDpiAwarenessContext(hwnd);
		DPI_AWARENESS dpiAwareness = GetAwarenessFromDpiAwarenessContext(dpi_context);

		win._places[config_name]._dpi = GetDpiForWindow(win._hwnd);
		win._places[config_name]._dpi_aware = dpiAwareness;
		win._send_dpi = (WS_MINIMIZEBOX & styles) != 0;

		HMONITOR hmon = MonitorFromRect(&win._places[config_name]._place.rcNormalPosition, MONITOR_DEFAULTTONULL);
		MONITORINFO mi;
		mi.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hmon, &mi);
		win._places[config_name]._hmon = MonitorFromWindow(win._hwnd, MONITOR_DEFAULTTOPRIMARY);

		logp(sys::e_debug, "     visible " << visible
			<< ", alt tab " << alt_tab_win
			<< ", frame " << frame_win
			<< ", popup " << (styles & WS_POPUP)
			<< ", toolwin " << (exStyles & WS_EX_TOOLWINDOW)
			<< ", noactivate " << (exStyles & WS_EX_NOACTIVATE)
			<< ", zoomed " << zoomed
			<< ", iconic " << iconic);
		logp(sys::e_debug, "     position: top " << win._places[config_name]._place.rcNormalPosition.top
			<< ", left " << win._places[config_name]._place.rcNormalPosition.left
			<< ", right " << win._places[config_name]._place.rcNormalPosition.right
			<< ", bottom " << win._places[config_name]._place.rcNormalPosition.bottom);
	}
    return TRUE;
}
namespace mcm {

	poshandler::rot_2::rot_2 (const std::string & str)
		: _str(str)
	{

		for (size_t i = 0; i < _str.size(); ++i) {
			if ((_str[i] + 2) > 126)
				_str[i] -= 2;
			else
				_str[i] += 2;
		}
	}

	std::string poshandler::rot_2::get_length(size_t len)
	{
		return _str.substr(0, len);
	}

	poshandler::poshandler ()
	{
		nlogf ();
		//EnumWindows (&Enum, (LPARAM)&_windows);
	}

	bool poshandler::has_resolution(const std::string & conf)
	{
		return _resolutions[conf] != "";
	}

	void poshandler::get_windows ()
	{
		nlogf ();
		mcm::sys::elapsed_t get_timer("get_windows");
		handler_ref_t poshand(_windows, _changing_resolution);
		EnumWindows (&Enum, (LPARAM)&poshand);
		conf_t conf;
		_resolutions[conf] = conf;
	}

	void poshandler::reposition ()
	{
		nlogf ();
		logp (sys::e_debug, "Repositioning.");
		conf_t config_name;

		for (auto win : _windows) {
			if (win.second._places[config_name]._dpi) {
				logp(sys::e_debug, "Setting placement for '"
					<< win.second._class_name
					<< "; DPI " << win.second._places[config_name]._dpi
					<< "': "
					<< ", top " << win.second._places[config_name]._place.rcNormalPosition.top
					<< ", left " << win.second._places[config_name]._place.rcNormalPosition.left
					<< ", right " << win.second._places[config_name]._place.rcNormalPosition.right
					<< ", bottom " << win.second._places[config_name]._place.rcNormalPosition.bottom);

				if (!SetWindowPlacement(win.second._hwnd, &win.second._places[config_name]._place)) {
					logp(sys::e_debug, "Can't set window placement for last window.");
					win_error error("Can't reposition window.");
				}
				else if (win.second._send_dpi
					&& win.second._places[config_name]._dpi != 96
					&& win.second._places[config_name]._dpi_aware == DPI_AWARENESS_PER_MONITOR_AWARE)
				{
					logp(sys::e_debug, "Sending DPI changing message.");
					LPARAM lp = (LPARAM)&win.second._places[config_name]._place.rcNormalPosition;
					SendMessage(win.second._hwnd, WM_DPICHANGED,
						MAKEWPARAM(win.second._places[config_name]._place.rcNormalPosition.top, win.second._places[config_name]._place.rcNormalPosition.bottom),
						lp);
				}
			}
		}
		_changing_resolution = false;
	}

	bool poshandler::window_exist (HWND & hwnd)
	{
		return _windows.find(hwnd) != _windows.end();
	}

	void poshandler::remove_window (HWND & hwnd)
	{
		nlogf ();
		if (window_exist(hwnd))
			_windows.erase(_windows.find(hwnd));
	}

	bool poshandler::get_window_placement (HWND hwnd, WINDOWPLACEMENT & place)
	{
		place.length = sizeof(place);
		RECT r;
		bool getr = GetWindowRect(hwnd, &r);
		bool ret = GetWindowPlacement(hwnd, &place);
		RECT& p = place.rcNormalPosition;

		logp(sys::e_debug, "[1] Window rect:      top " << r.top << " left " << r.left << " right " << r.right << " bottom " << r.bottom);
		logp(sys::e_debug, "[1] Window placement: top " << p.top << " left " << p.left << " right " << p.right << " bottom " << p.bottom);
#if 1
		DPI_AWARENESS_CONTEXT previousDpiContext = GetThreadDpiAwarenessContext();
		DPI_AWARENESS_CONTEXT dpi_context = GetWindowDpiAwarenessContext(hwnd);
		DPI_AWARENESS dpiAwareness = GetAwarenessFromDpiAwarenessContext(dpi_context);
		switch (dpiAwareness) {
			// Scale the window to the system DPI
		case DPI_AWARENESS_SYSTEM_AWARE:
			logp(sys::e_debug, "DPI System aware");
			//SetThreadDpiAwarenessContext(dpi_context);
			break;
			// Scale the window to the monitor DPI
		case DPI_AWARENESS_PER_MONITOR_AWARE:
			logp(sys::e_debug, "DPI Per monitor aware");
			//SetThreadDpiAwarenessContext(dpi_context);
			//uDpi = GetDpiForWindow(hwnd);
			break;
		case DPI_AWARENESS_UNAWARE:
			logp(sys::e_debug, "DPI Unaware");
			//SetThreadDpiAwarenessContext(dpi_context);
			break;
		default:
			logp(sys::e_debug, "DPI Unknown");
			break;
		}
		//getr = GetWindowRect(hwnd, &r);
		//ret = GetWindowPlacement(hwnd, &place);
#endif
		if (GetDpiForWindow(hwnd) != 96)
			p = r;
		//p = place.rcNormalPosition;
		logp(sys::e_debug, "[2] Window rect:      top " << r.top << " left " << r.left << " right " << r.right << " bottom " << r.bottom);
		logp(sys::e_debug, "[2] Window placement: top " << p.top << " left " << p.left << " right " << p.right << " bottom " << p.bottom);
		
		//SetThreadDpiAwarenessContext(previousDpiContext);
		return ret;
	}

	bool poshandler::get_class_name (HWND hwnd, LPSTR buf, INT buf_size)
	{
		bool result = true;
		char window_title[200];
		UINT class_length = 0;
		UINT title_length = 0;

		::memset (window_title, 0, 200);

		class_length = GetClassNameA(hwnd, buf, buf_size);
		title_length = GetWindowTextA(hwnd, window_title, ARRAYSIZE(window_title));

		if (! class_length) {
			result = false;
		}
		std::string sum;

		sum = buf;
		
		nlogp (sys::e_debug, "Window class name: " << sum);
		if (title_length) {
#ifdef _DEBUG
			std::string tmp;
			nlogp (sys::e_debug, "  window title: " << window_title);
			tmp = window_title;
			tmp += " (class: ";
			sum = tmp + sum;
			sum += ")";
#else
			rot_2 r(window_title);
			sum += r;
#endif
			::strncpy (buf, sum.c_str(), sum.size());
		}

		return result;
	}

	bool poshandler::discard_window_app_frame (const char * class_name, INT buf_size)
	{
		const char * cn = "ApplicationFrameWindow";

		if (::strncmp((const char *)class_name, cn,
					  mcm::sys::amin(::strlen(class_name), ::strlen(cn))) == 0)
		{
			return true;
		}

		return false;
	}

} // namespace mcm
