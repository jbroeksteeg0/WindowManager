#pragma once
#include "WindowManager.h"

namespace Layouts {
	void arrangeFloatingWindows(std::vector<AppWindow> windows, Rect monitor_geometry); 
	void verticalMaster(std::vector<AppWindow> windows, int gap_size, int border_size, Rect monitor_geometry);
}
