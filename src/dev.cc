//
#include "dev.hh"


bool get_enum_monitors (HMONITOR mon, HDC hdc, LPRECT rect, LPARAM data)
{
	logf ();
	dev * d = (dev*)data;
	d->add_monitor_count ();
	return true;
}
