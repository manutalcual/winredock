//-*- mode: c++; indent-tabs-mode: t; -*-
//
// Class: deserializer Copyright (c) 2018 Manuel Cano
// Author: manutalcual@gmail.com
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

namespace mcm {

	deserializer_t::deserializer_t (std::string file_name, mapwin_t & windows)
		: _good {false},
		  _in {file_name},
		  _i {0},
		  _win {windows},
		  _count{0}
	{
		_good = _in;
	}

	bool deserializer_t::match (char c)
	{
		skip_blanks ();
		if (_in[_i] != c)
			return false;

		++_i;
		return true;
	}

	bool deserializer_t::get_windows_config ()
	{
		logf ();
		return get_windows_vector();
	}

	bool deserializer_t::get_windows_vector ()
	{
		logf ();
		if (!match('[')) {
			logp (sys::e_debug, "Syntax error, there should be an open "
				  "quare bracket and there is a '"
				  << _in[_i] << "'.");
			_errors.push_back (std::string("Bad file format. Config file "
										   "must begin with an array square "
										   "braket and it begins with '")
							   + _in[_i] + "'..");
			return false;
		}

		if (!match(']')) {
			do {
				logp (sys::e_debug, "...");
				if (!get_class_entity()) {
					return false;
				}
				logp (sys::e_debug, "char at pos: " << _in[_i]);
			} while (match(','));
		}
		if (!match(']')) {
			logp (sys::e_debug, "Syntax error, there should be a close "
				  "square bracket and there is a '" << _in[_i] << "'.");
			_errors.push_back (std::string("Config file must be ended by a "
										   "close square bracket and no '")
							   + _in[_i] + "'.");
			return false;
		}

		return true;
	}

	bool deserializer_t::get_class_entity ()
	{
		logf ();
		if (! match('{')) {
			logp (sys::e_debug, "Syntax error, there should be an open brace.");
			_errors.push_back (std::string("Syntax error, there shoul be an "
										   "open brace and there is '")
							   + _in[_i] + "'.");
			return false;
		}

		if (!get_class_token()) {
			return false;
		}
		if (!match(':')) {
			logp (sys::e_debug, "Syntax error, there should be a colon an "
				  " there is '" << _in[_i] << "'.");
			_errors.push_back (std::string("Class keyword and class name must"
										   " be separated by a space and "
										   " there is '")
							   + _in[_i] + "'.");
			return false;
		}
		if (!get_class_name()) {
			return false;
		}
		if (match(',')) {
			if (!get_class_data()) {
				return false;
			}
		}

		skip_blanks ();
		if (!match('}')) {
			logp (sys::e_debug, "Bad class entity termination. (No '}' at end.)");
			_errors.push_back (std::string("Syntax error, there should be a "
										   "close bracket '}' and there is a '")
							   + _in[_i] + "'.");
			return false;
		}

		return true;
	}

	bool deserializer_t::get_class_token ()
	{
		logf ();
		skip_blanks ();
		std::string name = get_string();
		if (name != c_class) {
			logp (sys::e_debug, "Schema error, expected 'class', '"
				  << name << "' received.");
			return false;
		}
		return true;
	}

	bool deserializer_t::get_class_name ()
	{
		logf ();
		skip_blanks ();
		std::string value = get_value(); // class name
		_win[++_count]._class_name = value;
		_win[_count]._deserialized = true;
		return true;
	}

	bool deserializer_t::get_class_data ()
	{
		logf ();
		std::string data = get_string();

		if (data != c_data) {
			logp (sys::e_debug, "There should be 'data' and there is '"
				  << data << "'.");
			_errors.push_back (std::string("Data section must begin "
										   "with 'data' name, currently "
										   "begins with '")
							   + data + "'.");
			return false;
		}
		if (!match(':')) {
			logp (sys::e_debug, "There should be a colon and there is '"
				  << _in[_i] << "'.");
			_errors.push_back (std::string("Data section name and value must "
										   "be separated by a colon and "
										   "there is '")
							   + _in[_i] + "'.");
			return false;
		}
		if (!match('{')) {
			logp (sys::e_debug, "There should be an open brace and there is '"
				  << _in[_i] << "'.");
			_errors.push_back (std::string("Data section must begin with an open brace "
										   "and there is '")
							   + _in[_i] + "'.");
			return false;
		}
		if (!match('}')) {
			do {
				if (!get_class_element()) {
					return false;
				}
			} while (match(','));
		}
		if (!match('}')) {
			logp (sys::e_debug, "There should be an close brace and there is '"
				  << _in[_i] << "'.");
			_errors.push_back (std::string("Data section must end with a close brace "
										   "and there is '")
							   + _in[_i] + "'.");
			return false;
		}
		return true;
	}

	bool deserializer_t::get_class_element ()
	{
		logf ();
		std::string name = get_string();
		std::string value;

		skip_blanks ();
		logp (sys::e_debug, "What is there? '"
			  << _in[_i] << "'.");
		if (!match(':')) {
			logp (sys::e_debug, "There should be a colon and there is '"
				  << _in[_i] << "'.");
			_errors.push_back (std::string("Element and values must "
										   "be separated by a colon and "
										   "there is '")
							   + _in[_i] + "'.");
			return false;
		}
		if (!match('{')) {
			logp (sys::e_debug, "Get class element: value");
			skip_blanks ();
			value = get_value();
			if (name == c_title)
				_win[_count]._title = value;
			if (name == c_flags)
				_win[_count]._place.flags = mcm::sys::atoi(value);
			if (name == c_show)
				_win[_count]._place.showCmd = mcm::sys::atoi(value);

		} else if (--_i, match('{')) {
			logp (sys::e_debug, "Get class element: subelement");
			if (!get_sub_element(name)) {
				return false;
			}
			if (!match('}')) {
				logp (sys::e_debug, "There should be a close brace"
					  " and there is '" << _in[_i] << "'.");
				_errors.push_back (std::string("Element values must "
											   "end with close brace and "
											   "there is '")
								   + _in[_i] + "'.");
				return false;
			}
		} else {
			logp (sys::e_debug, "There should be a colon or an open brace"
				  " and there is '" << _in[_i] << "'.");
			_errors.push_back (std::string("Element values must "
										   "begin with color on open brace and "
										   "there is '")
							   + _in[_i] + "'.");
			return false;
		}
		return true;
	}

	bool deserializer_t::get_sub_element (std::string element)
	{
		logf ();
		bool ret = false;

		if (element == c_min_position) {
			ret = get_min_position();
		} else if (element == c_max_position) {
			ret = get_max_position();
		} else if (element == c_placement) {
			ret = get_placement();
		} else {
			logp (sys::e_debug, "Unknown element: '"
				  << element << "'.");
			_errors.push_back (std::string("Unknown element: '")
							   + element + "'.");
			return false;
		}
		return ret;
	}

	bool deserializer_t::get_min_position ()
	{
		logf ();
		std::string name1 = get_string();
		match (':');
		std::string value1 = get_value();
		match (',');
		std::string name2 = get_string();
		match (':');
		std::string value2 = get_value();
		return true;
	}

	bool deserializer_t::get_max_position ()
	{
		logf ();
		std::string name1 = get_string();
		match (':');
		std::string value1 = get_value();
		match (',');
		std::string name2 = get_string();
		match (':');
		std::string value2 = get_value();
		return true;
	}

	bool deserializer_t::get_placement ()
	{
		logf ();
		std::string name1 = get_string();
		match (':');
		std::string value1 = get_value();
		match (',');
		std::string name2 = get_string();
		match (':');
		std::string value2 = get_value();
		match (',');
		std::string name3 = get_string();
		match (':');
		std::string value3 = get_value();
		match (',');
		std::string name4 = get_string();
		match (':');
		std::string value4 = get_value();
		return true;
	}

	bool deserializer_t::operator () ()
	{
		logf ();

		return get_windows_config();
	}

	std::string deserializer_t::get_value ()
	{
		logf ();
		skip_blanks ();
		if (_in[_i] == '"') {
			logp (sys::e_debug, "Selected 'string' to parse." << _in[_i]);
			return get_string();
		} else {
			logp (sys::e_debug, "Selected 'number' to parse. " << _in[_i]);
			return get_number();
		}
		return "";
	}

	std::string deserializer_t::get_number ()
	{
		logf ();
		std::string value;
		skip_blanks ();
		logp (sys::e_debug, "  begin with '"
			  << _in[_i] << "'.");
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
		logp (sys::e_debug, "  get string begins with: '"
			  << _in[_i] << "'.");
		if (!match('"')) {
			logp (sys::e_debug, "Not getting anything.");
			return "";
		}
		std::string str;
		while (_in[_i] != '"')
			str += _in[_i++];
		if (!match ('"')) {
			logp (sys::e_debug, "This can't happen!");
		}
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


} // nammespace mcm
