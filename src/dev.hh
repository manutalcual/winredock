//-*- mode: c++; indent-tabs-mode: t; -*-
//
// Class: dev Copyright (c) 2018 Manuel Cano
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
#ifndef dev_hh
#define dev_hh
#include "common.hh"

bool get_enum_monitors (HMONITOR mon, HDC hdc, LPRECT rect, LPARAM data);

class dev
{
public:
	dev ()
		: _monitors (0),
		  _right (0),
		  _bottom (0)
	{
		read ();
	}
	void read ()
	{
		RECT desktop;
		//_monitors = GetSystemMetrics(SM_CMONITORS);
		// Get a handle to the desktop window
		const HWND hDesktop = GetDesktopWindow();
		// Get the size of screen to the variable desktop
		GetWindowRect (hDesktop, &desktop);
		// The top left corner will have coordinates (0,0)
		// and the bottom right corner will have coordinates
		// (horizontal, vertical)
		_top = GetSystemMetrics(SM_XVIRTUALSCREEN); //desktop.top;
		_left = GetSystemMetrics(SM_YVIRTUALSCREEN); //desktop.left;
		_top = GetSystemMetrics(SM_CXMAXTRACK);
		_left = GetSystemMetrics(SM_CYMAXTRACK);
		_right = desktop.right;
		_bottom = desktop.bottom;
		// SM_CYVIRTUALSCREEN
		// SM_CXVIRTUALSCREEN
		_width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		_height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		_monitors = 0;
#if 0
		logp (sys::e_debug, "Enumerating display devices");
		DWORD           DispNum = 0;
		DISPLAY_DEVICE  DisplayDevice;

		DisplayDevice.cb = sizeof(DISPLAY_DEVICE);
		while (EnumDisplayDevicesA(NULL, DispNum, &DisplayDevice, 0)) {
			++DispNum;
			logp (sys::e_debug, "Display name: '" << DisplayDevice.DeviceName << "' "
				  << DispNum);
		}
#endif
		enum_monitors ();
		nlogp (sys::e_debug, "Metrics [read]: width: " << _width
			  << ", height: " << _height
			  << ", top: " << _top
			  << ", left: " << _left
			  << ", right: " << _right
			  << ", bottom: " << _bottom
			  << ", monitors: " << _monitors);
	}
	void print ()
	{
		logp (sys::e_debug, "Metrics [print]: width: " << _width
			  << ", height: " << _height
			  << ", top: " << _top
			  << ", left: " << _left
			  << ", right: " << _right
			  << ", bottom: " << _bottom
			  << ", monitors: " << _monitors);
	}
	size_t width ()
	{
		return _width;
	}
	size_t height ()
	{
		return _height;
	}
	size_t monitors ()
	{
		return _monitors;
	}
	void operator = (const dev & d)
	{
		_monitors = d._monitors;
		_width = d._width;
		_height = d._height;
		_top = d._top;
		_left = d._left;
		_right = d._right;
		_bottom = d._bottom;
	}
	bool operator == (const dev & d)
	{
		return _monitors == d._monitors;
	}
	bool operator != (const dev & d)
	{
		return _monitors != d._monitors;
	}
 	bool operator < (const dev & d)
	{
		return _monitors < d._monitors;
	}
 	bool operator <= (const dev & d)
	{
		return _monitors <= d._monitors;
	}
 	bool operator > (const dev & d)
	{
		return _monitors > d._monitors and (_width > d._width or _height > d._height);
	}
 	bool operator >= (const dev & d)
	{
		return _monitors >= d._monitors;
	}
	void enum_monitors ()
	{
		EnumDisplayMonitors(
			NULL,
			NULL,
			(MONITORENUMPROC)get_enum_monitors,
			(LPARAM) this
			);
	}
	void add_monitor_count ()
	{
		++_monitors;
	}
	void update_monitors (HMONITOR hmon)
	{
		nlogf ();
		MONITORINFO mi;
		mi.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hmon, &mi);

		if (mi.rcMonitor.top != _top or
			mi.rcMonitor.left != _left or
			mi.rcMonitor.right != _right or
			mi.rcMonitor.bottom != _bottom)
		{
			nlogp (sys::e_debug, "Monitor data has changed: ");
			nlogp (sys::e_debug, "  previous data: "
				  << _top << ", "
				  << _left << ", "
				  << _right << ", "
				  << _bottom);

			nlogp (sys::e_debug, "  actual data: "
				  << mi.rcMonitor.top << ", "
				  << mi.rcMonitor.left << ", "
				  << mi.rcMonitor.right << ", "
				  << mi.rcMonitor.bottom);
			nlogp (sys::e_debug, "  monitor data is going to be updated");
		}

		if (mi.rcWork.left < _left)
			_left = mi.rcMonitor.left;
		if (mi.rcWork.top < _top)
			_top = mi.rcMonitor.top;
		if (mi.rcWork.right > _right)
			_right = mi.rcMonitor.right;
		if (mi.rcWork.bottom > _bottom)
			_bottom = mi.rcMonitor.bottom;
	}
//private:
	LONG _monitors;
	LONG _top;
	LONG _left;
	LONG _width;
	LONG _height;
	LONG _right;
	LONG _bottom;

};


#endif // dev_hh
