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
#include "serializer.hh"

namespace sys {

	serializer::serializer (mapwin_t & map)
		: _mapwin (map)
	{
	}

	bool serializer::operator () (std::string file_name)
	{
		std::ofstream output (file_name, std::ios::out);
		mapwin_t::iterator b = _mapwin.begin();
		mapwin_t::iterator e = _mapwin.end();

		if (!output) {
			MessageBoxExW ((HWND)0, L"Can't write config. file", L"Seting working dir",
						   MB_OK,
						   MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
			return false;
		}

		output << "[\n";
		for ( ; b != e; ++b) {
			if (b != _mapwin.begin())
				output << ",\n";
			logp (sys::e_debug, "Adding '"
				  << b->second._class_name << "'.");
			output << "{ \"class\" : \"" << b->second._class_name << "\",\n"
				   << "\t\"data\" : {\n"
				   << "\t\t\"title\" : \"" << b->second._title << "\", \n"
				   << "\t\t\"flags\" : " << b->second._place.flags << ", \n"
				   << "\t\t\"show\" : " << b->second._place.showCmd << ", \n"
				   << "\t\t\"min_position\" : { \"x\" : " << b->second._place.ptMinPosition.x << ", "
				   << "\"y\" : " << b->second._place.ptMinPosition.y << "}, \n"
				   << "\t\t\"max_position\" : { \"x\" : " << b->second._place.ptMaxPosition.x << ", "
				   << "\"y\" : " << b->second._place.ptMaxPosition.y << "}, \n"
				   << "\t\t\"placement\" : { \"top\" : " << b->second._place.rcNormalPosition.top << ", "
				   << "\"left\" : " << b->second._place.rcNormalPosition.left << ", "
				   << "\"bottom\" : " << b->second._place.rcNormalPosition.bottom << ", "
				   << "\"right\" : " << b->second._place.rcNormalPosition.right << " } } }";
		}
		output << "\n]" << std::endl;

		return true;
	}

	bool serializer::deserialize (std::string file_name)
	{
		deserializer_t des (file_name, _mapwin);

		for (auto & w : _mapwin) {
			logp (sys::e_debug, "Serializer window pre: " << w.second._deserialized
				  << ", '" << w.second._title << "'.");
		}

		logp (sys::e_debug, "Deserializer created.");
		if (! des) {
			logp (sys::e_debug, "Can't operate on file.");
			return false;
		}
		logp (sys::e_debug, "Calling deserializer to deserialize.");
		if (! des())
			return false;

		for (auto & w : _mapwin) {
			logp (sys::e_debug, "Serializer window after: " << w.second._deserialized
				  << ", '" << w.second._title << "'.");
		}

		return true;
	}

} // namespace sys
