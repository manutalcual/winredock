//-*- mode: c++; indent-tabs-mode: t; -*-
//
// File: common Copyright (c) 2018 Manuel Cano
// Author: manutalcual@gmail.com
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

namespace mcm {
	namespace sys {

#ifdef WITH_LOG
		std::ofstream log ("wm.log");
#endif

		int atoi (std::string & str)
		{
			int num = ::strtol(str.c_str(), NULL, 10);
			return num;
		}

		std::string itoa (int i)
		{
			char buf[124];
			::sprintf (buf, "%d", i);
			return buf;
		}

		stat_t::stat_t (std::string file_name)
			: _good (::stat(file_name.c_str(), &_st) == 0)
		{
		}

		size_t stat_t::size ()
		{
			return _st.st_size;
		}

		file_t::file_t (std::string file_name)
			: _good (false),
			  _file (::fopen(file_name.c_str(), "r")),
			  _buf (nullptr)
		{
			nlogf ();

			if (_file == nullptr) {
				logp (sys::e_debug, "Ca't open file.");
				return;
			}

			stat_t st (file_name);
			if (! st) {
				logp (sys::e_debug, "Can't stat file.");
				return;
			}

			_buf = new char [st.size() + 1];
			size_t readed = ::fread (_buf, 1, st.size(), _file);
			if (readed != st.size()) {
				logp (sys::e_debug, "Can't read the whole file.");
				return;
			}
			_buf[st.size()] = '\0';
			_size = st.size();
			nlogp (sys::e_debug, "Data readed: "
				  << _size << ".");
			nlogp (sys::e_debug, "First char: '"
				  << _buf[0] << "'."); //
			_good = true;
		}

		char & file_t::operator [] (int i)
		{
			return _buf[i];
		}

		bool set_cwd::operator () (set_cwd::cwd type)
		{
			DWORD flag;
			switch (type) {
			case home:
				flag = CSIDL_PROFILE;
				break;
			case data:
				flag = CSIDL_APPDATA;
				break;
			default:
				return false;
				break;
			}

			HRESULT result = SHGetFolderPath(NULL, flag, NULL, 0, _path);
			if (SUCCEEDED(result)) {
				/*
				MessageBoxExW ((HWND)0, profilePath, L"Seting working dir",
							   MB_OK,
							   MAKELANGID(LANG_NEUTRAL,
							   SUBLANG_NEUTRAL));
				*/
				return SetCurrentDirectory(_path);
			}
			return false;
		}

	} // namespace sys

	std::string guid_to_string (GUID * guid)
	{
		char guid_string[37]; // 32 hex chars + 4 hyphens + null terminator
		snprintf(
			guid_string, sizeof(guid_string) / sizeof(guid_string[0]),
			"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			guid->Data1, guid->Data2, guid->Data3,
			guid->Data4[0], guid->Data4[1], guid->Data4[2],
			guid->Data4[3], guid->Data4[4], guid->Data4[5],
			guid->Data4[6], guid->Data4[7]);
		return guid_string;
	}


} // namespace mcm
