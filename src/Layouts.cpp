#include "Layouts.h"
#include "AppWindow.h"
#include "WindowManager.h"
#include <vector>

void Layouts::verticalMaster(std::vector<AppWindow> windows, int gap_size, int border_size, Rect monitor_geometry) {
	float ratio = 0.6;

	int x = monitor_geometry.offset_x;
	int y = monitor_geometry.offset_y;
	int w = monitor_geometry.width;
	int h = monitor_geometry.height;
	
	// if there's a fullscreen window
	for (AppWindow win: windows) {
		if (win.get_fullscreen()) {
			win.move_resize(x,y,w,h);

			// hide others
			for (AppWindow win2: windows) {
				if (win.window() != win2.window()) win2.hide();
			}
			return;
		}
	}
	
	if (windows.size() == 1) {
		windows[0].move_resize(gap_size, gap_size, w - gap_size*2, h-gap_size*2, windows[0].get_border_size());
	} else if (windows.size() > 1) {
		windows[0].move_resize(gap_size, gap_size, (w * ratio)-gap_size*1.5, h-gap_size*2, windows[0].get_border_size());

		int nx = w*ratio + gap_size*0.5;
		int nw = w-nx;
		int ny = gap_size;
		int per_y = (h - windows.size()*gap_size) / (windows.size()-1);
		for (int i = 1; i < windows.size(); i++) {
			if (i+1 < windows.size()) { // do normally
				windows[i].move_resize(nx, ny, nw-gap_size, per_y, windows[i].get_border_size());
			} else { // rounding dies so compensate
				windows[i].move_resize(nx, ny, nw-gap_size, monitor_geometry.height-gap_size-ny, windows[i].get_border_size());
			}
			ny += per_y+gap_size;
		}
	}
}

void Layouts::arrangeFloatingWindows(std::vector<AppWindow> windows, Rect monitor_geometry) {
	for (AppWindow w: windows) {
		w.move_resize(
			w.floating_x+monitor_geometry.offset_x,
			w.floating_y+monitor_geometry.offset_y,
			w.floating_w,
			w.floating_h
		);
	}
}
