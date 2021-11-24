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

	const CLSID CLSID_ImmersiveShell = { 0xC2F03A33, 0x21F5, 0x47FA, 0xB4, 0xBB, 0x15, 0x63, 0x62, 0xA2, 0xF2, 0x39 };

	IServiceProvider* GetServiceProvider()
	{
		IServiceProvider* provider{ nullptr };
		if (FAILED(CoCreateInstance(CLSID_ImmersiveShell, nullptr, CLSCTX_LOCAL_SERVER, __uuidof(provider), (PVOID*)&provider)))
		{
			logp(sys::e_error, "Failed to get ServiceProvider for VirtualDesktopManager");
			return nullptr;
		}
		return provider;
	}

	IVirtualDesktopManager* GetVirtualDesktopManager()
	{
		IVirtualDesktopManager* manager{ nullptr };
		IServiceProvider* serviceProvider = GetServiceProvider();
		if (serviceProvider == nullptr || FAILED(serviceProvider->QueryService(__uuidof(manager), &manager)))
		{
			logp(sys::e_error, "Failed to get VirtualDesktopManager");
			return nullptr;
		}
		return manager;
	}

	virt_desktop_t::virt_desktop_t ()
		: _manager{ GetVirtualDesktopManager() }
	{
	}

	virt_desktop_t::~virt_desktop_t ()
	{
	}

	virt_desktop_t::opt_guid virt_desktop_t::get_current_desktop_id()
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
			logp(sys::e_trace, "Can't get registry value.");
		}
		logp(sys::e_trace, "Can't get registry key.");
		return std::nullopt;

	}

	virt_desktop_t::opt_guid virt_desktop_t::get_desktop_id_from_current_session()
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
				logp(sys::e_trace, "Got current desktop: '" << win::guid_to_string(&value) << "'");
				return value;
			}
			logp(sys::e_trace, "Can't get registry value.");
		}
		logp(sys::e_trace, "Can't get registry key.");

		return std::nullopt;
	}

	virt_desktop_t::opt_guid_vect virt_desktop_t::get_virtual_desktop_ids_from_registry(HKEY hKey) const
	{
		if (!hKey)
		{
			return std::nullopt;
		}

		DWORD bufferCapacity;
		// request regkey binary buffer capacity only
		if (RegQueryValueExW(hKey, strings::RegVirtualDesktopIds, 0, nullptr, nullptr, &bufferCapacity) != ERROR_SUCCESS)
		{
			return std::nullopt;
		}

		std::unique_ptr<BYTE[]> buffer = std::make_unique<BYTE[]>(bufferCapacity);
		// request regkey binary content
		if (RegQueryValueExW(hKey, strings::RegVirtualDesktopIds, 0, nullptr, buffer.get(), &bufferCapacity) != ERROR_SUCCESS)
		{
			return std::nullopt;
		}

		const size_t guidSize = sizeof(GUID);
		std::vector<GUID> temp;
		temp.reserve(bufferCapacity / guidSize);
		for (size_t i = 0; i < bufferCapacity; i += guidSize)
		{
			GUID* guid = reinterpret_cast<GUID*>(buffer.get() + i);
			temp.push_back(*guid);
		}

		return temp;
	}

	virt_desktop_t::opt_guid virt_desktop_t::get_desktop_id_for_window(HWND window) const
	{
		GUID id;
		BOOL isWindowOnCurrentDesktop = false;
		if (_manager->IsWindowOnCurrentVirtualDesktop(window, &isWindowOnCurrentDesktop) == S_OK && isWindowOnCurrentDesktop)
		{
			LRESULT ok{ 0 };
			// Filter windows such as Windows Start Menu, Task Switcher, etc.
			if ((ok = _manager->GetWindowDesktopId(window, &id)) == S_OK && id != GUID_NULL)
			{
				return id;
			}
			error err{ok, "Getting window destop id"};
			logp(sys::e_trace, "Can't get window desktop id.");
		}
		logp(sys::e_trace, "Can't know if window is in current desktop. ("
			<< isWindowOnCurrentDesktop << ")");

		return std::nullopt;
	}


} // end namespace win

