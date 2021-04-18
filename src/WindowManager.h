#pragma once

#include <vector>
#include <map>
#include <functional>
#include <X11/Xlib.h>
#include <set>
#include "AppWindow.h"
#include "Workspace.h"
#include "KeypressHandler.h"

class KeypressHandler;
class Workspace;
class AppWindow;

struct Rect {
	int offset_x, offset_y, width, height;
};


class WindowManager {
	Window root_window;
	KeypressHandler *keypress_handler;
	
	std::set<Window> mapped;
	std::map<Window, Rect> default_floating_sizes;
	
	std::vector<Workspace*> workspaces;
	int num_workspaces;
	Workspace *current_workspace;

	std::vector<Rect> monitors;
	std::vector<Workspace*> monitor_workspace;
	int num_monitors;
	int focused_monitor=0;

	std::vector<Window> get_all_windows(Window current, int depth=0);
	std::vector<AppWindow> all_application_windows;

	void reset_window_list();
	void print_windows();
	void tile();
public:
	Display *display;
	int gap_size = 16;
	int border_size = 4;
	int floating_increment_size = 32;

	WindowManager(int n_workspaces);
	void add_key_bind(std::string keybind, std::function<void(WindowManager*)> func);
	void remove_window(AppWindow w);

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

	void move_window_to_workspace(int workspace);
	void view_workspace(int workspace);
	
	void kill_window();
	void toggle_fullscreen();
	void toggle_floating();
	void run();

	std::map<Window, Rect> get_default_floating_sizes();
};
