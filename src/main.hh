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
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <map>
#include <fstream>

#include <windows.h>
#include "resource.h"

#define logp(p, str) log << "[" << __FILE__								\
	<< ":" << __LINE__ << "] " << str << std::endl
#define logf(p) log << "[" << __FILE__								\
	<< ":" << __LINE__ << ": "											\
	<< __PRETTY_FUNCTION__ << "] " << std::endl

const std::string FILE_NAME ("window_list.json");

template<typename Type>
Type min (Type a, Type b)
{
	return a < b ? a : b;
}

namespace sys {
	enum e_prio {
		e_debug,
		e_warning,
		e_error,
		e_critical,
		e_failure
	};

	int atoi (std::string & str);
}

class win_t
{
public:
	HWND _hwnd;
	WINDOWPLACEMENT _place;
	bool _deserialized;
	std::string _title;
	std::string _class_name;

	win_t ()
		: _hwnd{},
		  _deserialized{}
		{}
};
typedef std::map<HWND, win_t> mapwin_t;

class serializer
{
	class stat_t
	{
	public:
		stat_t (std::string file_name);
		operator bool () { return _good; }
		size_t size ();
	private:
		bool _good;
		struct stat _st;
	};
	class file_t
	{
	public:
		file_t (std::string file_name);
		~file_t () { if (_file) ::fclose(_file); }
		operator bool () { return _good; }
		size_t size () { return _size; }
		char & operator [] (int i) { return _buf[i]; }
	private:
		bool _good;
		FILE * _file;
		size_t _size;
		char * _buf;
	};
	class deserial_t
	{
	public:
		deserial_t (std::string file_name, mapwin_t & windows);
		operator bool () { return _good; }
		bool operator () ();
	private:
		bool _good;
		file_t _in;
		size_t _i;
		mapwin_t & _win;

		bool match (char c);
		void skip_blanks ();
		std::string get_string ();
		std::string get_number ();
		std::string get_value ();

	};
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
bool get_class_name (HWND hwnd, LPTSTR buf, INT buf_size);
bool discard_window_app_frame (LPTSTR class_name, INT buf_size);
BOOL CALLBACK Enum (HWND hwnd, LPARAM lParam);
bool get_window_placement (HWND hwnd, WINDOWPLACEMENT & place);
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);

void Restore ();
void Minimize ();
void InitNotifyIconData ();

#ifdef UNICODE
#define stringcopy wcscpy
#else
#define stringcopy strcpy
#endif
