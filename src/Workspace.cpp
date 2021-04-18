#include <algorithm>
#include "Workspace.h"
#include "Layouts.h"

// this workspace may contain some old windows that were removed previously
void Workspace::remove_old_windows(std::vector<AppWindow> all_windows) {
	std::vector<AppWindow> temp;

	for (AppWindow w: windows) {
		// if w is still in the window tree
		if (std::find(all_windows.begin(),all_windows.end(), w) != all_windows.end()) {
			temp.push_back(w);
		}
	}

	windows = temp;
	focused = std::min(focused, ((int)windows.size())-1);
}

void Workspace::add_window(AppWindow w) {
	if (focused == -1) focused = 0;

	auto temp = parent->get_default_floating_sizes();

	if (temp.find(w.window()) != temp.end()) { // pog
		Rect g = temp[w.window()];
		w.floating_x=g.offset_x;
		w.floating_y=g.offset_y;
		w.floating_w=g.width;
		w.floating_h=g.height;
	}
	windows.insert(windows.begin(), w);
}

void Workspace::delete_window(AppWindow w) {
	if (std::find(windows.begin(),windows.end(),w) != windows.end()) {
		windows.erase(std::find(windows.begin(),windows.end(),w));
		if (focused >= windows.size()) focused--;
	}
}

Result<AppWindow> Workspace::get_focused_window() {
	if (focused == -1) {
		return Result<AppWindow>::Err("");
	}
	return Result<AppWindow>::Ok(windows[focused]);
}

void Workspace::tile(bool grab_focus, Rect geom) {
	std::vector<AppWindow> to_tile, to_float;

	for (AppWindow w: windows) {
		(w.get_floating() ? to_float : to_tile).push_back(w);
	}

	bool has_fullscreen = false;

	for (AppWindow &w: to_tile) {
		if (w.get_fullscreen()) {
			has_fullscreen=true;
			w.set_border_size(0);
			break;
		}
	}
	
	if (!has_fullscreen) {
		for (int i = 0; i < windows.size(); i++) {
			windows[i].set_border_size((i == focused) * 4);
			if (i == focused && grab_focus) {
				windows[i].grab_focus();
			}
		}
	}

	Layouts::verticalMaster(to_tile, parent->gap_size, parent->border_size, geom);
	Layouts::arrangeFloatingWindows(to_float, geom);

	// get stacking order
	Window *all_windows = new Window[windows.size()];
	int cnt=0;
	for (AppWindow w: to_float) { all_windows[cnt++] = w.window();}
	for (AppWindow w: to_tile) { all_windows[cnt++] = w.window();}
	XRestackWindows(parent->display, all_windows, cnt);
	delete[] all_windows;
}

void Workspace::hide_all() {
	for (AppWindow &w: windows) w.hide();
}

void Workspace::move_focus_down() {
	if (focused+1<windows.size()) {
		focused++;
	}
}
void Workspace::move_focus_up() {
	if (focused>0) {
		focused--;
	}
}

void Workspace::move_floating_left() {
	if (focused>=0 && windows[focused].get_floating()) {
		windows[focused].floating_x -= parent->floating_increment_size;		
	}
}
void Workspace::move_floating_right() {
	if (focused>=0 && windows[focused].get_floating()) {
		windows[focused].floating_x += parent->floating_increment_size;		
	}
}
void Workspace::move_floating_up() {
	if (focused>=0 && windows[focused].get_floating()) {
		windows[focused].floating_y -= parent->floating_increment_size;		
	}
}
void Workspace::move_floating_down() {
	if (focused>=0 && windows[focused].get_floating()) {
		windows[focused].floating_y += parent->floating_increment_size;		
	}
}

void Workspace::resize_floating_left() {
	if (focused>=0 && windows[focused].get_floating() && windows[focused].floating_w > parent->floating_increment_size) {
		windows[focused].floating_w -= parent->floating_increment_size;		
	}
}
void Workspace::resize_floating_right() {
	if (focused>=0 && windows[focused].get_floating()) {
		windows[focused].floating_w += parent->floating_increment_size;		
	}
}
void Workspace::resize_floating_up() {
	if (focused>=0 && windows[focused].get_floating() && windows[focused].floating_h > parent->floating_increment_size) {
		windows[focused].floating_h -= parent->floating_increment_size;		
	}
}
void Workspace::resize_floating_down() {
	if (focused>=0 && windows[focused].get_floating()) {
		windows[focused].floating_h += parent->floating_increment_size;		
	}
}

void Workspace::move_window_down() {
	if (focused+1<windows.size()) {
		std::swap(windows[focused],windows[focused+1]);
		focused++;
	}
}
void Workspace::move_window_up() {
	if (focused > 0) {
		std::swap(windows[focused-1],windows[focused]);
		focused--;
	}
}

void Workspace::kill_window() {
	if (focused != -1) {
		windows[focused].kill();
		parent->remove_window(windows[focused]);
		windows.erase(windows.begin()+focused);
		if (focused >= windows.size()) {
			focused--;
		}
	}
}

void Workspace::toggle_floating() {
	if (focused != -1) {
		windows[focused].set_floating(!windows[focused].get_floating());
	}
}

void Workspace::toggle_fullscreen() {
	if (focused != -1) {
		bool set=false;
		if (!windows[focused].get_fullscreen()) {
			set=true;
		}

		for (AppWindow &w: windows) {
			windows[focused].set_fullscreen(false);
		}

		if (set) {
			windows[focused].set_fullscreen(true);
		}
	}
}
std::vector<AppWindow> Workspace::get_all_used_windows() {
	return windows;
}

Workspace::Workspace(WindowManager *p): parent(p) {

}
