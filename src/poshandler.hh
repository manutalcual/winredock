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
#ifndef poshandler_hh
#define poshandler_hh
#include "common.hh"
#include "serializer.hh"
#include "dev.hh"

BOOL is_alt_tab_window (HWND hwnd);
BOOL CALLBACK Enum (HWND hwnd, LPARAM lParam);

namespace mcm {
	class poshandler
	{
		class rot_2
		{
		public:
			rot_2 (const std::string & str);
			operator std::string () { return _str; }
			std::string get_length(size_t len);
		private:
			std::string _str;
		};
	public:
		poshandler ();
		void reposition ();
		void get_windows ();
		void save_configuration (std::string file_name);
		void load_configuration (std::string file_name);
		bool window_exist (HWND & hwnd);
		void remove_window (HWND & hwnd);
		void uniform_windows (poshandler & pos);
		void uniform_windows ();

		static bool get_class_name (HWND hwnd, LPSTR buf, INT buf_size);
		static bool discard_window_app_frame (const char * class_name, INT buf_size);
		static bool get_window_placement (HWND hwnd, WINDOWPLACEMENT & place);

	private:
		volatile bool _clearing;
		mapwin_t _windows;
		dev _screen_size;
	};
} // namespace mcm
#endif // posthandler_hh
