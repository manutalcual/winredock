//-*- mode: c++; indent-tabs-mode: t; -*-
//
// Class: deserializer Copyright (c) 2018 Manuel Cano
// Author: manuel.cano@amadeus.com
// Date: Tue Oct 09 10::22 2018
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
#include "deserializer.hh"

namespace mc {
	namespace ds {

		deserializer_t::deserializer_t (std::string file_name, mapwin_t & windows)
			: _good (false),
			  _in (file_name),
			  _win (windows)
		{
			_good = _in;
		}

		bool deserializer_t::match (char c)
		{
			if (_in[_i] != c)
				return false;

			++_i;
			return true;
		}


		bool deserializer_t::operator () ()
		{
			logf ();
			std::string name;
			std::string value;
			std::string data;

			logp (sys::e_debug, "This is the first char: '"
				  << _in[_i] << "'.");

			if (_in[_i] != '[') {
				logp (sys::e_debug,
					  "Serialization file must begin with a '[' character.");
				return false;
			}
			logp (sys::e_debug, "We begin with the havoc!");
			HWND count = 0;
			int st = 0; // status
			skip_blanks ();
			if (!match('[')) {
				logp (sys::e_debug, "Syntax error, there should be an open whatever.");
				return false;
			}
			skip_blanks ();
			while (_i < _in.size()) {
				logp (sys::e_debug, "Status: " << st);
				switch (st) {
				case 0: // class
					skip_blanks ();
					if (! match('{')) {
						logp (sys::e_debug, "Syntax error, there should be an open brace.");
						st = 99;
						return false;
					}
					skip_blanks ();
					name = get_string();
					if (name != "class") {
						logp (sys::e_debug, "Schema error, expected 'class', '"
							  << name << "' received.");
						st = 99;
					} else
						st = 1;
					break;
				case 1: // class name
					skip_blanks ();
					if (! match(':')) {
						logp (sys::e_debug, "Syntax error, expected ':' after a class begining, found '"
							  << _in[_i] << "'.");
						st = 99;
					} else {
						skip_blanks ();
						value = get_value(); // class name
						_win[++count]._class_name = value;
						_win[count]._deserialized = true;
						st = 2;
					}
					break;
				case 2: // data
					skip_blanks ();
					if (! match(',')) {
						logp (sys::e_debug, "Syntax error, there should be a comma.");
						st = 99;
					} else {
						skip_blanks ();
						data = get_string();
						if (data != "data") {
							logp (sys::e_debug, "Schema error, should be 'data' and there is "
								  << data << "'.");
							st = 99;
							return false;
						} else
							st = 3;
					}
					break;
				case 3: // data object
					skip_blanks ();
					if (! match(':')) {
						logp (sys::e_debug, "Syntax error, there should be a colon.");
						st = 99;
						return false;
					}
					skip_blanks ();
					if (! match('{')) {
						logp (sys::e_debug, "Syntax error, there should be an open brace.");
						st = 99;
						return false;
					}
					st = 4;
					break;
				case 4: // data values
					skip_blanks ();
					while (st == 4) {
						name = get_string();
						skip_blanks ();
						if (!match(':')) {
							logp (sys::e_debug, "Syntax error, there should be a colon.");
							st = 99;
							return false;
						}
						value = "";
						skip_blanks ();
						if (name == "title") {
							value = get_value();
							_win[count]._title = value;
						} else if (name == "name")
							value = get_value();
						else if (name == "show") {
							value = get_value();
							_win[count]._place.length = sizeof(WINDOWPLACEMENT);
							_win[count]._place.showCmd = sys::atoi(value);
						} else if (name == "flags") {
							value = get_value();
							_win[count]._place.flags = sys::atoi(value);
						} else if (name == "min_position") {
							st = 5;
						} else if (name == "max_position") {
							st = 9;
						} else if (name == "placement") {
							st = 6;
						} else {
							logp (sys::e_debug, "Schema error, unknown "
								  << data << "''field.");
							st = 99;
							return false;
						}
						if (value == "") {
							logp (sys::e_debug, "Captured: '"
								  << name << "' without value, or this is an old name.");
						} else {
							logp (sys::e_debug, "Captured '" << name
								  << "': '" << value << "'.");
						}
						skip_blanks ();
						if (!match(','))
							--_i;
					}
					skip_blanks ();
					break;
				case 5: // min position
					skip_blanks ();
					// GET the X
					if (! match('{')) {
						logp (sys::e_debug,
							  "Syntax error, there should be an open brace, there is '"
							  << _in[_i] << "'.");
						st = 99;
						return false;
					}
					skip_blanks ();
					name = get_string();
					skip_blanks ();
					if (! match(':')) {
						logp (sys::e_debug, "Syntax errot, there should be a colon.");
						st = 99;
						return false;
					}
					skip_blanks ();
					value = get_value();
					_win[count]._place.ptMinPosition.x = sys::atoi(value);
					skip_blanks ();
					if (! match(',')) {
						logp (sys::e_debug, "Syntax errot, there should be a comma.");
						st = 99;
						return false;
					}
					skip_blanks ();
					// GET the Y
					name = get_string();
					skip_blanks ();
					if (! match(':')) {
						logp (sys::e_debug, "Syntax errot, there should be a colon.");
						st = 99;
						return false;
					}
					skip_blanks ();
					value = get_value();
					_win[count]._place.ptMinPosition.y = sys::atoi(value);
					skip_blanks ();
					if (! match('}')) {
						logp (sys::e_debug, "Syntax errot, there should be a closing brace.");
						st = 99;
						return false;
					}
					skip_blanks ();
					if (match(',')) {
						logp (sys::e_debug, "Syntax errot, there should be a closing brace.");
						st = 4;
					} else {
						--_i; // retrieve the "match" eaten char
					}
					break;
				case 9: // max position
					skip_blanks ();
					// GET the X
					if (! match('{')) {
						logp (sys::e_debug,
							  "Syntax error, there should be an open brace, there is '"
							  << _in[_i] << "'.");
						st = 99;
						return false;
					}
					skip_blanks ();
					name = get_string();
					skip_blanks ();
					if (! match(':')) {
						logp (sys::e_debug, "Syntax errot, there should be a colon.");
						st = 99;
						return false;
					}
					skip_blanks ();
					value = get_value();
					_win[count]._place.ptMaxPosition.x = sys::atoi(value);
					skip_blanks ();
					if (! match(',')) {
						logp (sys::e_debug, "Syntax errot, there should be a comma.");
						st = 99;
						return false;
					}
					skip_blanks ();
					// GET the Y
					name = get_string();
					skip_blanks ();
					if (! match(':')) {
						logp (sys::e_debug, "Syntax errot, there should be a colon.");
						st = 99;
						return false;
					}
					skip_blanks ();
					value = get_value();
					_win[count]._place.ptMaxPosition.y = sys::atoi(value);
					skip_blanks ();
					if (! match('}')) {
						logp (sys::e_debug, "Syntax errot, there should be a closing brace.");
						st = 99;
						return false;
					}
					skip_blanks ();
					if (match(',')) {
						logp (sys::e_debug, "Syntax errot, there should be a closing brace.");
						st = 4;
					} else {
						--_i; // retrieve the "match" eaten char
					}
					break;
				case 6:
					skip_blanks ();
					// GET the TOP
					if (! match('{')) {
						logp (sys::e_debug, "Syntax error, there should be an open brace.");
						st = 99;
						return false;
					}
					skip_blanks ();
					name = get_string();
					skip_blanks ();
					if (! match(':')) {
						logp (sys::e_debug, "Syntax errot, there should be a colon.");
						st = 99;
						return false;
					}
					skip_blanks ();
					value = get_value();
					_win[count]._place.rcNormalPosition.top = sys::atoi(value);
					skip_blanks ();
					if (! match(',')) {
						logp (sys::e_debug, "Syntax errot, there should be a comma.");
						st = 99;
						return false;
					}
					skip_blanks ();
					// GET the LEFT
					name = get_string();
					skip_blanks ();
					if (! match(':')) {
						logp (sys::e_debug, "Syntax errot, there should be a colon.");
						st = 99;
						return false;
					}
					skip_blanks ();
					value = get_value();
					_win[count]._place.rcNormalPosition.left = sys::atoi(value);
					skip_blanks ();
					if (! match(',')) {
						logp (sys::e_debug, "Syntax error, there should be a comma, there is '"
							  << _in[_i] << "'.");
						st = 99;
						return false;
					}
					++_i; // skip ,
					skip_blanks ();
					// GET the BOTTOM
					name = get_string();
					skip_blanks ();
					if (! match(':')) {
						logp (sys::e_debug, "Syntax errot, there should be a colon.");
						st = 99;
						return false;
					}
					skip_blanks ();
					value = get_value();
					_win[count]._place.rcNormalPosition.bottom = sys::atoi(value);
					skip_blanks ();
					if (! match(',')) {
						logp (sys::e_debug, "Syntax errot, there should be a comma.");
						st = 99;
						return false;
					}
					skip_blanks ();
					// GET the RIGHT
					name = get_string();
					skip_blanks ();
					if (! match(':')) {
						logp (sys::e_debug, "Syntax errot, there should be a colon.");
						st = 99;
						return false;
					}
					skip_blanks ();
					value = get_value();
					_win[count]._place.rcNormalPosition.right = sys::atoi(value);
					skip_blanks ();
					if (! match('}')) {
						logp (sys::e_debug, "Syntax errot, there should be a closing brace.");
						st = 99;
						return false;
					}
					st = 7;
					break;
				case 7: // closing brace of data
					skip_blanks ();
					if (! match('}')) {
						logp (sys::e_debug, "Syntax errot, there should be a closing brace.");
						st = 99;
						return false;
					}
					st = 8;
					break;
				case 8: // closing brace of class
					skip_blanks ();
					if (! match('}')) {
						logp (sys::e_debug, "Syntax errot, there should be a closing brace.");
						st = 99;
						return false;
					}
					skip_blanks ();
					if (match(',')) {
						st = 0;
					} else {
						logp (sys::e_debug, "Parsing finished succesfully.");
						return true;
					}
					break;
				case 99:
					logp (sys::e_debug, "There has been an error and I can't continue.");
					return false;
					break;
				default:
					logp (sys::e_debug, "Unknown state when parsing.");
					break;
				}
			}

			logp (sys::e_debug, "Parsing finished succesfuly.");
			return true;
		}

		std::string deserializer_t::get_value ()
		{
			logf ();
			skip_blanks ();
			if (_in[_i] == '"') {
				logp (sys::e_debug, "Selected 'string' to parse.");
				return get_string();
			} else {
				logp (sys::e_debug, "Selected 'number' to parse.");
				return get_number();
			}
			return "";
		}

		std::string deserializer_t::get_number ()
		{
			logf ();
			std::string value;
			skip_blanks ();
			if (_in[_i] == '-') {
				value += '-';
				++_i; // skip - (minus sign)
			}
			while (::isdigit(_in[_i]))
				value += _in[_i++];
			logp (sys::e_debug, "  captured '"
				  << value << "'.");
			return value;
		}

		std::string deserializer_t::get_string ()
		{
			logf ();
			skip_blanks ();
			logp (sys::e_debug, "  get string begins with: '"
				  << _in[_i] << "'.");
			if (_in[_i] != '"') {
				logp (sys::e_debug, "Not getting anything.");
				return "";
			}
			++_i;
			std::string str;
			while (_in[_i] != '"')
				str += _in[_i++];
			++_i; // skip closing "
			logp (sys::e_debug, "  captured '"
				  << str << "'. (remaining char '"
				  << _in[_i] << "')");
			return str;
		}

		void deserializer_t::skip_blanks ()
		{
			while (::isspace(_in[_i]))
				++_i;
		}


	} // namespace ds
} // nammespace m
