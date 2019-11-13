//
#include "dev.hh"


bool get_enum_monitors (HMONITOR mon, HDC hdc, LPRECT rect, LPARAM data)
{
	dev * d = (dev*)data;
	d->add_monitor_count ();
	d->update_monitors (mon);
	return true;
}
