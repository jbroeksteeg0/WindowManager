#pragma once

#include <iostream>
#include <vector>
#include <X11/Xlib.h>
#include <JB/Result.h>
#include "WindowManager.h"

class AppWindow {
	Window raw_window;
	Display *display;
	bool is_fullscreen=false;
	bool do_tile; // TODO implement this
	int border_size=0;
	bool is_floating=false;
public:
	int floating_x,floating_y,floating_w,floating_h;
	AppWindow(Display *d, Window w, bool tile=true);

	void set_floating(bool b);

	void move_resize(int x, int y, int w, int h, int border_size=0);
	void set_fullscreen(bool f);
	void set_border_size(int size);
	void grab_focus();
	void hide();
	void kill();
	
	Result<unsigned long> get_cardinal(std::string name);
	Result<std::vector<std::string>> get_atom(std::string name);
	Window window() const;
	bool get_fullscreen() const;
	int get_border_size() const;
	bool get_floating() const;
	Result<std::string> get_name() const;
	bool operator==(const AppWindow &rhs) const;
};
