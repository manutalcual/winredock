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


class dev
{
public:
	dev ()
	{
		read ();
	}
	void read ()
	{
		RECT desktop;
		// Get a handle to the desktop window
		const HWND hDesktop = GetDesktopWindow();
		// Get the size of screen to the variable desktop
		GetWindowRect (hDesktop, &desktop);
		// The top left corner will have coordinates (0,0)
		// and the bottom right corner will have coordinates
		// (horizontal, vertical)
		_right = desktop.right;
		_bottom = desktop.bottom;
	}
	void print ()
	{
		logp (sys::e_debug, "[dev] Screen: "
			  << _right << ", " << _bottom);
	}
	void operator = (const dev & d)
	{
		_right = d._right;
		_bottom = d._bottom;
	}
	operator == (const dev & d)
	{
		return _right == d._right && _bottom == d._bottom;
	}
	operator != (const dev & d)
	{
		return _right != d._right && _bottom != d._bottom;
	}
 	operator < (const dev & d)
	{
		bool minor = _right < d._right && _bottom < d._bottom;
		return minor;
	}
 	operator <= (const dev & d)
	{
		bool minor = _right <= d._right && _bottom <= d._bottom;
		return minor;
	}
 	operator > (const dev & d)
	{
		bool greater = _right > d._right && _bottom > d._bottom;
		return greater;
	}
 	operator >= (const dev & d)
	{
		bool greater = _right >= d._right && _bottom >= d._bottom;
		return greater;
	}
private:
	size_t _right;
	size_t _bottom;

};

#endif // dev_hh
