//-*- mode: c++; indent-tabs-mode: t; -*-
//
// Class: deserializer Copyright (c) 2018 Manuel Cano
// Author: manuel.cano@amadeus.com
// Date: Tue Oct 09 10:00:22 2018
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

#ifndef deserializer_h
#define deserializer_h

namespace mc {
	namespace ds {

		const char c_class[] = "class";
		const char c_data[] = "data";
		const char c_title[] = "title";
		const char c_flags[] = "flags";
		const char c_show[] = "show";
		const char c_min_position[] = "min_position";
		const char c_max_position[] = "max_position";
		const char c_placement[] = "placement";
		const char c_x[] = "x";
		const char c_y[] = "y";
		const char c_top[] = "top";
		const char c_left[] = "left";
		const char c_right[] = "right";
		const char c_bottom[] = "bottom";

		class deserializer_t
		{
		public:
			deserializer_t (std::string file_name, mapwin_t & windows);
			operator bool () { return _good; }
			bool operator () ();
		private:
			bool _good;
			sys::file_t _in;
			size_t _i;
			mapwin_t _win;

			bool match (char ch);
			void skip_blanks ();
			std::string get_string ();
			std::string get_number ();
			std::string get_value ();
		};

	} // namespace ds
} // nammespace m

#endif
