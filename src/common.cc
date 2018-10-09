//-*- mode: c++; indent-tabs-mode: t; -*-
//
// File: common Copyright (c) 2018 Manuel Cano
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

namespace mc {
	namespace sys {

#ifdef WITH_LOG
		std::ofstream log ("wm.log");
#endif

		int atoi (std::string & str)
		{
			int num = ::strtol(str.c_str(), NULL, 10);
			return num;
		}



	} // namespace sys
} // namespace mc
