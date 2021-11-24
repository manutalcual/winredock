//
// Clase: virt_desktop_t Copyright (c) 2021 Manuel Cano
// Autor: Manuel Cano Muñoz
// Fecha: Tue Nov 23 14:19:04 2021
//
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; version 2 of the License.
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
#include <strsafe.h>

#include "virt_desktop.hh"

namespace strings
{
	const wchar_t RegCurrentVirtualDesktop[] = L"CurrentVirtualDesktop";
	const wchar_t RegVirtualDesktopIds[] = L"VirtualDesktopIDs";
	const wchar_t RegKeyVirtualDesktops[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VirtualDesktops";
	const wchar_t RegKeyVirtualDesktopsFromSession[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SessionInfo\\%d\\VirtualDesktops";
}


namespace win {

	virt_desktop_t::virt_desktop_t ()
	{
	}

	virt_desktop_t::~virt_desktop_t ()
	{
	}

	std::optional<GUID> virt_desktop_t::get_current_desktop_id()
	{
		HKEY key{};
		LSTATUS ok{ 0 };
		if ((ok = RegOpenKeyExW(HKEY_CURRENT_USER, strings::RegKeyVirtualDesktops, 0, KEY_ALL_ACCESS, &key)) == ERROR_SUCCESS)
		{
			GUID value{};
			DWORD size = sizeof(GUID);
			if ((ok = RegQueryValueExW(key, strings::RegCurrentVirtualDesktop, 0, nullptr, reinterpret_cast<BYTE*>(&value), &size)) == ERROR_SUCCESS)
			{
				logp(sys::e_trace, "Got current desktop: '" << win::guid_to_string(&value) << "'");
				return value;
			}
			win::error err{ ok, "Can't open registry value." };
			logp(sys::e_trace, "Can't get registry value.");
		}
		logp(sys::e_trace, "Can't get registry key.");
		win::error err{ ok, "Can't open registry key." };
		return std::nullopt;

	}

	std::optional<GUID> virt_desktop_t::GetDesktopIdFromCurrentSession()
	{
		DWORD sessionId;
		if (!ProcessIdToSessionId(GetCurrentProcessId(), &sessionId))
		{
			return std::nullopt;
		}

		wchar_t sessionKeyPath[256]{};
		if (FAILED(StringCchPrintfW(sessionKeyPath, ARRAYSIZE(sessionKeyPath),
			strings::RegKeyVirtualDesktopsFromSession,
			sessionId)))
		{
			logp(sys::e_error, "Error performing string substitution on RegKeyVirtualDesktopsFromSession.");
			return std::nullopt;
		}

		HKEY key{};
		if (RegOpenKeyExW(HKEY_CURRENT_USER, sessionKeyPath, 0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS)
		{
			GUID value{};
			DWORD size = sizeof(GUID);
			if (RegQueryValueExW(key, strings::RegCurrentVirtualDesktop, 0, nullptr,
				reinterpret_cast<BYTE*>(&value), &size) == ERROR_SUCCESS)
			{
				return value;
			}
		}

		return std::nullopt;
	}


} // end namespace win

