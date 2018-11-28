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
		if (_monitors == 1)
			enum_monitors ();
	}
	void read ()
	{
		RECT desktop;
		_monitors = GetSystemMetrics(SM_CMONITORS);
		// Get a handle to the desktop window
		const HWND hDesktop = GetDesktopWindow();
		// Get the size of screen to the variable desktop
		GetWindowRect (hDesktop, &desktop);
		// The top left corner will have coordinates (0,0)
		// and the bottom right corner will have coordinates
		// (horizontal, vertical)
		_right = desktop.right;
		_bottom = desktop.bottom;
		// SM_CYVIRTUALSCREEN
		// SM_CXVIRTUALSCREEN
		_width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		_height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		if (_monitors == 1)
			enum_monitors ();
		logp (sys::e_debug, "Metrics: width: " << _width
			  << ", height: " << _height
			  << ", right: " << _right
			  << ", bottom: " << _bottom
			  << ", monitors: " << _monitors);
	}
	void print ()
	{
		logp (sys::e_debug, "[dev] Screen: "
			  << _width << ", " << _height << ", "
			  << _right << ", " << _bottom
			  << " (mon " << _monitors << ")");
	}
	void operator = (const dev & d)
	{
		_right = d._right;
		_bottom = d._bottom;
	}
	operator == (const dev & d)
	{
		return _monitors == d._monitors;
	}
	operator != (const dev & d)
	{
		return _monitors != d._monitors;
	}
 	operator < (const dev & d)
	{
		return _monitors < d._monitors;
	}
 	operator <= (const dev & d)
	{
		return _monitors <= d._monitors;
	}
 	operator > (const dev & d)
	{
		return _monitors > d._monitors and (_width > d._width or _height > d._height);
	}
 	operator >= (const dev & d)
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
private:
	int _monitors;
	size_t _width;
	size_t _height;
	size_t _right;
	size_t _bottom;

};


#endif // dev_hh
