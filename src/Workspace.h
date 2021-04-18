#pragma once

#include <X11/Xlib.h>
#include <JB/Result.h>
#include "AppWindow.h"
#include "WindowManager.h"

class WindowManager;
class AppWindow;
class Rect;


class Workspace {
	std::vector<AppWindow> windows;
	int focused = -1;
	WindowManager *parent;
public:
	Workspace(WindowManager *p);

	void remove_old_windows(std::vector<AppWindow> all_windows);
	void add_window(AppWindow w);
	void delete_window(AppWindow w);
	void move_focus_up();
	void move_focus_down();
	void move_window_up();
	void move_window_down();

	void move_floating_left();
	void move_floating_right();
	void move_floating_up();
	void move_floating_down();

	void resize_floating_left();
	void resize_floating_right();
	void resize_floating_up();
	void resize_floating_down();

	void toggle_fullscreen();
	void toggle_floating();
	void kill_window();

	void tile(bool grab_focus, Rect geom);
	void hide_all();
	Result<AppWindow> get_focused_window();
	std::vector<AppWindow> get_all_used_windows();
};
