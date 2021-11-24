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
#include <optional>
#include "common.hh"

namespace win {

	class virt_desktop_t
	{
	public:
		virt_desktop_t ();
		~virt_desktop_t ();

		std::optional<GUID> get_current_desktop_id();
		std::optional<GUID> GetDesktopIdFromCurrentSession();
	protected:
	private:
	};

} // end namespace win


