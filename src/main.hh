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
#include "common.hh"
#include "deserializer.hh"
#include "window.hh"


class serializer
{
public:
	serializer (mapwin_t & map);
	bool operator () (std::string file_name);
	bool deserialize (std::string file_name, mapwin_t & windows);
private:
	mapwin_t & _mapwin;
};

UINT WM_TASKBARCREATED = 0;
HWND g_hwnd;
HMENU g_menu;
NOTIFYICONDATA g_notifyIconData;
HINSTANCE g_hInstance;

void uniform_windows (mapwin_t & windows);
BOOL IsAltTabWindow (HWND hwnd);
void show_status (UINT flag);
void show_position (RECT * rect);
bool get_class_name (HWND hwnd, LPSTR buf, INT buf_size);
bool discard_window_app_frame (const char * class_name, INT buf_size);
BOOL CALLBACK Enum (HWND hwnd, LPARAM lParam);
bool get_window_placement (HWND hwnd, WINDOWPLACEMENT & place);
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);

void Restore ();
void Minimize ();
void InitNotifyIconData ();
