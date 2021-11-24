//
// Clase: virt_desktop_t Copyright (c) 2021 Manuel Cano
// Autor: Manuel Cano Muñoz
// Fecha: Tue Nov 23 14:19:04 2021
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
#pragma once
//
// Includes
//
#include <vector>
#include <optional>

#include "common.hh"

namespace strings
{
	extern const wchar_t RegCurrentVirtualDesktop[];
	extern const wchar_t RegVirtualDesktopIds[];
	extern const wchar_t RegKeyVirtualDesktops[];
	extern const wchar_t RegKeyVirtualDesktopsFromSession[];
}

namespace win {

	class virt_desktop_t
	{
	public:
		using opt_guid_vect = std::optional<std::vector<GUID>>;
		using opt_guid = std::optional<GUID>;

		static HKEY open_virtual_desktops_reg_key()
		{
			HKEY hKey{ nullptr };
			if (RegOpenKeyExW(HKEY_CURRENT_USER, strings::RegKeyVirtualDesktops, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
			{
				return hKey;
			}
			return nullptr;
		}
		static HKEY get_virtual_desktops_reg_key()
		{
			static HKEY virtualDesktopsKey{ open_virtual_desktops_reg_key() };
			return virtualDesktopsKey;
		}


		virt_desktop_t ();
		~virt_desktop_t ();

		opt_guid get_current_desktop_id();
		opt_guid get_desktop_id_from_current_session();
		opt_guid_vect get_virtual_desktop_ids_from_registry(HKEY hKey) const;

		opt_guid get_desktop_id_for_window(HWND window) const;




	protected:
	private:
		IVirtualDesktopManager* _manager{ nullptr };
	};

} // end namespace win


