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
        hwndWalk = GetAncestor(hwnd, GA_ROOTOWNER);
    }

    HWND hwndTry = hwndWalk;
    while ((hwndTry = GetLastActivePopup(hwndWalk)) != hwndTry) {
        if (IsWindowVisible(hwndTry))
			break;
        hwndWalk = hwndTry;
    }
    // tool windows are treated as not visible, they'll appear in the
    // list
    return (hwndWalk == hwnd) && !(exStyles & WS_EX_TOOLWINDOW);
}

BOOL CALLBACK Enum (HWND hwnd, LPARAM lParam)
{
	nlogf ();
	static size_t count = 0;
	mapwin_t & windows = *(mapwin_t *)lParam;
	const int BUF_SIZE = 1024;
	char class_name[BUF_SIZE];

	::memset (class_name, 0, BUF_SIZE);

	nlogp (sys::e_debug, "--- Enum " << ++count << " ---");
	win::poshandler::get_class_name (hwnd, (LPSTR)class_name, BUF_SIZE);
	nlogp (sys::e_debug, "Window visible: " << IsWindowVisible(hwnd)
		  << ", zoomed " << IsZoomed(hwnd)
		  << ", iconic " << IsIconic(hwnd));
	nlogp (sys::e_debug, "Enum: Get class name: '" << class_name << "'");

    if (is_alt_tab_window(hwnd) && IsWindowVisible(hwnd)) {
		if (win::poshandler::discard_window_app_frame((const char *)class_name,
															   ::strlen(class_name)))
		{
			nlogp (sys::e_debug, "Discarding window (because it is an app frame).");
			nlogp (sys::e_debug, "    " << class_name);
			return TRUE;
		}

		win_t win;
        CHAR buf[260];

		dev d;
		std::string config_name = sys::itoa(d.width());
		config_name += "_";
		config_name += sys::itoa(d.height());
		config_name += "_";
		config_name += sys::itoa(d.monitors());
		nlogp (sys::e_debug, "Current configuration: " << config_name);

		win._hwnd = hwnd;
        GetWindowTextA(hwnd, buf, ARRAYSIZE(buf));
		win._title = buf;
		win._class_name = class_name;
		win::poshandler::get_window_placement (hwnd, win._place);

		HMONITOR hmon = MonitorFromRect(&win._place.rcNormalPosition, MONITOR_DEFAULTTONULL);
		MONITORINFO mi;
		mi.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hmon, &mi);

		win._off_screen = (win._place.rcNormalPosition.top > d._bottom
						   || win._place.rcNormalPosition.bottom < d._top
						   || win._place.rcNormalPosition.right < d._left
						   || win._place.rcNormalPosition.left > d._right);
		nlogp (sys::e_debug, "Window monitor info: top "
			  << mi.rcMonitor.top << ", left "
			  << mi.rcMonitor.left << ", right "
			  << mi.rcMonitor.right << ", bottom "
			  << mi.rcMonitor.bottom << ", primary? "
			  << mi.dwFlags);
		nlogp (sys::e_debug, "Window class " << class_name << " position: top "
			  << win._place.rcNormalPosition.top << ", left "
			  << win._place.rcNormalPosition.left << ", right "
			  << win._place.rcNormalPosition.right << ", bottom "
			  << win._place.rcNormalPosition.bottom << ", no monitor? "
			  << (bool)(hmon == NULL) << ", calc offscreen: "
			  << win._off_screen);

		win_t::place_t place;
		place._place = win._place;
		place._hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);

		win._places[config_name] = place;
		windows[win._hwnd] = win;
		//show_status (win._place.showCmd);
		//show_position (&win._place.rcNormalPosition);
	} else {
		nlogp (sys::e_debug, "Window is not alt-tab and/or not visible. (" << class_name << ")");
	}
    return TRUE;
}
namespace win {

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
		: _clearing (false)
	{
		logf ();
		EnumWindows (&Enum, (LPARAM)&_windows);
	}

	void poshandler::get_windows (bool get_desktop)
	{
		logf ();
		if (_clearing) {
			logp (sys::e_debug, "We are clearing, don't get more windows");
			return;
		}
		nlogp (sys::e_debug, "Getting current desktop windows. (clearing windows map)");
		_clearing = true;
		_windows.clear ();
		// Get windows opened
		EnumWindows (&Enum, (LPARAM)&_windows);

		if (get_desktop) {
			virt_desktop_t virtual_desktops;
			dev d;
			std::string config_name = sys::itoa(d.width());
			config_name += "_";
			config_name += sys::itoa(d.height());
			config_name += "_";
			config_name += sys::itoa(d.monitors());
			//auto cur_desktop = virtual_desktops.get_current_desktop_id();
			auto cur_desktop = virtual_desktops.get_desktop_id_from_current_session();
			//win::virt_desktop_t::opt_guid_vect vect = virtual_desktops.get_virtual_desktop_ids_from_registry(win::virt_desktop_t::get_virtual_desktops_reg_key());

			for (auto& [key, window] : _windows) {
				nlogp(sys::e_trace, "Window '" << window._title << "'");
				virt_desktop_t::opt_guid win_desktop_guid = virtual_desktops.get_desktop_id_for_window(key);
				if (win_desktop_guid.has_value()) {
					window._places[config_name]._desktop = win_desktop_guid.value();
					nlogp(sys::e_trace, "   GUID: "
						<< win::guid_to_string(&window._places[config_name]._desktop));
				}
				else if (virtual_desktops.is_window_in_current_desktop(key)) {
					window._places[config_name]._desktop = cur_desktop.value();
				}
			}
		}
		_clearing = false;
		nlogp (sys::e_debug, "Got current desktop windows. (not clearing anymore)");
	}

	void poshandler::save_configuration (std::string file_name)
	{
		sys::serializer serial (_windows);
		serial (file_name);
	}

	void poshandler::load_configuration (std::string file_name)
	{
		if (_clearing)
			return;
		sys::serializer serial (_windows);
		if (!serial.deserialize(file_name)) {
			logp (sys::e_debug, "Ouch! Deserializer failed!");
		}
		for (auto & w : _windows) {
			nlogp (sys::e_debug, "Poshandler window: " << w.second._deserialized
				  << ", '" << w.second._title << "'.");
		}
		uniform_windows ();
	}

	void poshandler::reposition ()
	{
		logf ();
		if (_clearing) {
			size_t clearing_count = 0;
			while (_clearing and ++clearing_count < 1000)
				;
			if (clearing_count > 999) {
				logp (sys::e_debug,
					  "Clearing has reached 1000 (so reposition wait has to be forced.");
			}
		}
		virt_desktop_t virtual_desktops;
		dev d;
		std::string config_name = sys::itoa(d.width());
		config_name += "_";
		config_name += sys::itoa(d.height());
		config_name += "_";
		config_name += sys::itoa(d.monitors());
		//auto cur_desktop = virtual_desktops.get_current_desktop_id();
		//auto cur_desktop = virtual_desktops.get_desktop_id_from_current_session();
		logp (sys::e_debug, "Repositioning.");
		mapwin_t::iterator begin = _windows.begin();
		mapwin_t::iterator end = _windows.end();
		for (; begin != end; ++begin) {
			logp(sys::e_debug, "Setting placement for '"
				<< begin->second._title << "' class '"
				<< begin->second._class_name << "': "
				<< ", top " << begin->second._place.rcNormalPosition.top
				<< ", left " << begin->second._place.rcNormalPosition.left
				<< ", right " << begin->second._place.rcNormalPosition.right
				<< ", bottom " << begin->second._place.rcNormalPosition.bottom);
#if 0
			if (begin->second._place.showCmd == SW_MAXIMIZE) {
				WINDOWPLACEMENT wp = begin->second._place;
				wp.showCmd = SW_RESTORE;
				wp.flags = WPF_ASYNCWINDOWPLACEMENT;
				SetWindowPlacement (begin->second._hwnd, &wp);
				//ShowWindow (begin->second._hwnd, SW_RESTORE);
				ShowWindow (begin->second._hwnd, SW_HIDE);
				ShowWindow (begin->second._hwnd, SW_SHOW);
			}
#endif
			begin->second._place.flags |= WPF_ASYNCWINDOWPLACEMENT;
			SetWindowPlacement(begin->second._hwnd, &begin->second._place);
			if (! SetWindowPlacement(begin->second._hwnd, &begin->second._place)) {
				logp (sys::e_debug, "Can't set window placement for last window.");
				win::error error ("Can't reposition window.");
			} else {
				if (begin->second._places[config_name]._desktop != GUID_NULL)
				{
					if (!virtual_desktops.move_window_to_desktop(begin->second._hwnd, begin->second._places[config_name]._desktop))
					{
						logp(sys::e_warning, "Can't move window '"
							<< begin->second._title << "' to desktop "
							<< guid_to_string(&begin->second._places[config_name]._desktop));
					}
					else
					{
						logp(sys::e_info, "Window moved '"
							<< begin->second._title << "' to desktop "
							<< guid_to_string(&begin->second._places[config_name]._desktop));
					}
				}
				ShowWindow (begin->second._hwnd, SW_HIDE);
				//ShowWindow (begin->second._hwnd, SW_RESTORE);
				ShowWindow (begin->second._hwnd, SW_SHOW);
			}
		}
	}

	bool poshandler::window_exist (HWND & hwnd)
	{
		return _windows.find(hwnd) != _windows.end();
	}

	void poshandler::remove_window (HWND & hwnd)
	{
		logf ();
		logp (sys::e_debug, "Remove windows does nothing actually");
	}

	void poshandler::uniform_windows (poshandler & pos)
	{
		logf ();
		if (_clearing) {
			logp (sys::e_debug, "There is a clearing ongoing so no uniform windows");
			return;
		}
		_clearing = true;
		logp (sys::e_debug,
			  "We have to check if one window in current config has been "
			  "deleted in another screen configuration.");
		for (auto & item : _windows) {
			if (! pos.window_exist(item.second._hwnd)) {
				item.second._erase = true;
			}
		}

		mapwin_t::iterator b = _windows.begin();
		mapwin_t::iterator e = _windows.end();
		for ( ; b != e; ) {
			if (b->second._erase) {
				logp (sys::e_debug, "Deleting window '"
					  << b->second._class_name
					  << "', because deleted in other screen config.");
				_windows.erase (b++);
			} else {
				++b;
			}
		}
		logp (sys::e_debug, "Clearing uniformed windows done");
		_clearing = false;
	}
	void poshandler::uniform_windows ()
	{
		logf ();
		if (_clearing) {
			logp (sys::e_debug, "no uniform windows as we are clearing windows map");
			return;
		}

		logp (sys::e_debug, "Go to unform windows between several screen configurations");
		for (auto & item : _windows) {
			logp (sys::e_debug, "The window is: "
				  << item.second._hwnd << ", "
				  << item.second._deserialized << ", '"
				  << item.second._class_name << "'.");
			for (auto & other : _windows) {
				if (/* item != other and */
					!item.second._deserialized and
					other.second._deserialized and
					other.second._class_name == item.second._class_name and
					other.second._title == item.second._title)
				{
					win_t & realw = item.second; // from file
					win_t & fakew = other.second; // from OS
					logp (sys::e_debug, "\tnomalize '"
						  << realw._deserialized << "', '"
						  << realw._class_name << "', with '"
						  << fakew._deserialized << "', '"
						  << fakew._class_name << "', '"
						  << fakew._title << "'.");
					logp (sys::e_debug, "\tMin position: x "
						  << fakew._place.ptMinPosition.x
						  << ", y " << fakew._place.ptMinPosition.y);
					logp (sys::e_debug, "\tMax position: "
						  << fakew._place.ptMaxPosition.x
						  << ", y " << fakew._place.ptMaxPosition.y);
					logp (sys::e_debug, "\tPlacement: "
						  << fakew._place.ptMaxPosition.x
						  << ", top " << fakew._place.rcNormalPosition.top
						  << ", left " << fakew._place.rcNormalPosition.left
						  << ", right " << fakew._place.rcNormalPosition.right
						  << ", bottom " << fakew._place.rcNormalPosition.bottom);

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
		logp (sys::e_debug, "Uniform windows got ready to do actually the work");
		mapwin_t::iterator b = _windows.begin();
		mapwin_t::iterator e = _windows.end();

		for ( ; b != e; ) {
			if (b->second._deserialized) {
				logp (sys::e_debug, "Deleting '"
					  << b->second._class_name << "', "
					  << b->second._deserialized << ", as a deserialized window.");
				_windows.erase (b++);
			} else {
				++b;
			}
		}
		logp (sys::e_debug, "Uniform windows finished");
	}

	bool poshandler::get_window_placement (HWND hwnd, WINDOWPLACEMENT & place)
	{
		place.length = sizeof(place);
		return GetWindowPlacement(hwnd, &place);
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
			rot_2 r(window_title);
			nlogp (sys::e_debug, "  window title: " << window_title);
			sum += r;
			::strncpy (buf, sum.c_str(), sum.size());
		}

		return result;
	}

	bool poshandler::discard_window_app_frame (const char * class_name, INT buf_size)
	{
		const char * cn = "ApplicationFrameWindow";

		if (::strncmp((const char *)class_name, cn,
					  sys::amin(::strlen(class_name), ::strlen(cn))) == 0)
		{
			return true;
		}

		return false;
	}

} // namespace win
