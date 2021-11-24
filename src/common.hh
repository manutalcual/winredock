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
#ifndef common_h
#define common_h

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN64
#include <io.h>
#define __PRETTY_FUNCTION__ __FUNCTION__
#else
#include <unistd.h>
#endif
#include <string>
#include <iostream>
#include <map>
#include <fstream>

#include <windows.h>
#include <shlobj.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "resource.h"

#ifdef UNICODE
#define stringcopy wcscpy
#else
#define stringcopy strcpy
#endif


#ifdef WITH_LOG
#define logp(p, str) do { if (p <= sys::common::LOG_LEVEL) { \
		sys::log \
		<< sys::get_printable_time()   \
		<< "[" << __FILE__	\
		<< ":" << __LINE__ << "] " \
		<< sys::common::log_names[p]                    \
		<< ": "                                         \
		<< sys::log_tabs::tabs << str << std::endl; \
	} } while (0)
#define logf() logp(sys::e_trace, __PRETTY_FUNCTION__)
#define nlogp(p, str)
#define nlogf()
#else
#define logp(p, str)
#define logf()
#define nlogp(p, str)
#define nlogf()
#endif

const std::string FILE_NAME("window_list.json");

class win_t
{
public:
	class place_t
	{
	public:
		WINDOWPLACEMENT _place;
		HMONITOR _hmon;
	};
	typedef std::map<std::string, win_t::place_t> places_t;
	HWND _hwnd;
	HDC _hdc;
	WINDOWPLACEMENT _place;
	places_t _places;
	bool _deserialized;
	std::string _title;
	std::string _class_name;
	bool _erase;
	bool _off_screen;

	win_t()
		: _hwnd{},
		_deserialized{},
		_erase{},
		_off_screen{}
	{}
	~win_t()
	{
		ReleaseDC(_hwnd, _hdc);
	}
};
typedef std::map<HWND, win_t> mapwin_t;

namespace sys {
	enum e_log_level {
		e_failure,
		e_alert,
		e_critical,
		e_notice,
		e_error,
		e_warning,
		e_info,
		e_debug,
		e_trace
	};

	class common
	{
	public:
		static std::ofstream cerr;
		static int LOG_LEVEL; /**< log level used for log macros */
		static const char* log_names[]; /** log priority names */
	};


#ifdef WITH_LOG
	extern std::ofstream log;
#endif

	class log_tabs
	{
	public:
		static std::string tabs;
		log_tabs()
		{
			tabs += "  ";
		}
		~log_tabs()
		{
			if (tabs.size())
				tabs = tabs.substr(0, tabs.size() - 2);
		}
	};

	std::ostream& operator << (std::ostream& o, log_tabs& l);

	template<typename Type>
	Type amin(Type a, Type b)
	{
		return a < b ? a : b;
	}

	int atoi(std::string& str);
	std::string itoa(int str);
	std::string get_printable_time();

	class stat_t
	{
	public:
		stat_t(std::string file_name);
		operator bool() { return _good; }
		size_t size();
	private:
		bool _good;
		struct stat _st;
	};

	class file_t
	{
	public:
		file_t(std::string file_name);
		~file_t() { if (_file) ::fclose(_file); }
		operator bool() { return _good; }
		size_t size() { return _size; }
		char& operator [] (int i);
	private:
		bool _good;
		FILE* _file;
		size_t _size;
		char* _buf;
	};

	class set_cwd
	{
	public:
		enum cwd
		{
			home,
			data
		};
		bool operator () (cwd type);
		std::string path() { return _path; }
	private:
		char _path[MAX_PATH];
		/*
		  CHAR path[MAX_PATH];
		  GetCurrentDirectory (MAX_PATH, path);
		*/
	};

} // namespace sys


namespace win {

	std::string guid_to_string(GUID* guid);

	class error
	{
	public:
		error(const char* msg)
			: _msg(msg)
		{
			_error = GetLastError();
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				_error,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&_lpMsgBuf,
				0, NULL);
			logp(sys::e_debug, "Error: " << (char*)_lpMsgBuf);
		}
		error(LRESULT error, const char* msg)
			: _msg{ msg },
			_error{ static_cast<DWORD>(error) }
		{
			//_error = GetLastError();
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				_error,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&_lpMsgBuf,
				0, NULL);
			logp(sys::e_debug, "Error: " << (char*)_lpMsgBuf);
		}
		error& operator () ()
		{
			logp(sys::e_debug, _msg);
			std::string msg{ _msg };
			msg += (char*)_lpMsgBuf;
			FatalAppExit(0, TEXT(msg.c_str()));
			return *this; // this will never be reached!
		}
	private:
		const char* _msg;
		DWORD _error;
		LPVOID _lpMsgBuf;
	};



	bool if_nok(auto res)
	{
		decltype(auto) r = res;
		if (res != 0) {
			error err{ r, "Error executing Windows function" };
			return false;
		}
		return true;
	}

} // end namespace win

#endif // common_h
