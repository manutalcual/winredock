//-*- mode: c++; indent-tabs-mode: t; -*-
//
// Class: serializer Copyright (c) 2018 Manuel Cano
// Author: manutalcual@gmail.com
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
#ifndef serializer_h
#define serializer_h
#include "common.hh"
#include "deserializer.hh"

namespace mcm {

	class serializer
	{
	public:
		serializer (mapwin_t & map);
		bool operator () (std::string file_name);
		bool deserialize (std::string file_name);
	private:
		mapwin_t & _mapwin;
	};

} // namespace mcm
#endif // serializer_h
