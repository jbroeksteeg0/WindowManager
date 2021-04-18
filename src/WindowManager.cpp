#include <iostream>
#include <algorithm>
#include <X11/keysym.h>
#include <X11/extensions/Xinerama.h>
#include <cassert>
#include "WindowManager.h"
#include "AppWindow.h"

int err(Display *d, XErrorEvent *e) {
	// oh god oh fuck

	return 0;
}

WindowManager::WindowManager(int n_workspaces): num_workspaces(n_workspaces) {
	display = XOpenDisplay(nullptr);
	root_window = XDefaultRootWindow(display);
	keypress_handler = new KeypressHandler(this, display,root_window);
	default_floating_sizes = std::map<Window,Rect>();

	for (int i = 0; i < n_workspaces; i++) {
		workspaces.push_back(new Workspace(this));
	}

	current_workspace = workspaces[0];
	if (!XineramaIsActive(display)) {
		std::cout << "no xinerama support" << std::endl;
		exit(1);
	}
	XineramaScreenInfo *info = XineramaQueryScreens(display, &num_monitors);
	monitor_workspace.resize(num_monitors);
	assert(num_monitors <= num_workspaces);

	for (int i = 0; i < num_monitors; i++) {
		monitor_workspace[i] = workspaces[i];
		monitors.push_back({ info[i].x_org, info[i].y_org, info[i].width, info[i].height });
	}

	XSetErrorHandler(err);
}

void WindowManager::run() {
	XSelectInput(display, root_window, KeyPressMask | SubstructureNotifyMask);

	bool do_reset=false;
	while (true) {
		XSynchronize(display, true);

		// TODO handle key presses after all other events are done?
		if (XPending(display) > 0) {
			XEvent e;
			XNextEvent(display, &e);
			do_reset = true;

			if (e.type == KeyPress) {
				reset_window_list();
				keypress_handler->on_key_down(e.xkey.state, e.xkey.keycode);
			} else if (e.type == UnmapNotify) {
				mapped.erase(e.xmap.window);
			} else if (e.type == MapNotify) {
				if (mapped.find(e.xmap.window) == mapped.end()) {
					Window root_return;
					int x_return,y_return;
					unsigned int width_return,height_return,temp;

					XGetGeometry(
						display,
						e.xmap.window,
						&root_return,
						&x_return,
						&y_return,
						&width_return,
						&height_return,
						&temp,
						&temp
					);

					default_floating_sizes[e.xmap.window] = {x_return,y_return,(int)width_return,(int)height_return};
					std::cout << "New window: default size is " << width_return << "x" << height_return << std::endl;
					std::cout << "At position " << x_return << " " << y_return << std::endl;
					std::cout << "flags are ";

					auto types = AppWindow(display,e.xmap.window).get_atom("_NET_WM_WINDOW_TYPE");

					if (types.is_ok()) {
						for (std::string s: types.ok()) {
							std::cout << s << " ";
						}
						std::cout << std::endl;					
					}

				}
				mapped.insert(e.xmap.window);
			}
		} else if (do_reset) {
			reset_window_list();
			tile();
			do_reset=false;
		} 

		XSynchronize(display, false);
	}
}

std::vector<Window> WindowManager::get_all_windows(Window current, int depth) {
	std::vector<Window> ret;
	ret.push_back(current);

	Window root, parent, *children=nullptr;
	unsigned int n_children;
	if (XQueryTree(display, current, &root, &parent, &children, &n_children) != 0) {
		for (int i = 0; i < n_children; i++) {
			for (Window w: get_all_windows(children[i],depth+1)) {
				ret.push_back(w);
			}
		}
	}
	XFree(children);

	return ret;
}

void WindowManager::reset_window_list() {
	all_application_windows.clear();

	for (Window w: get_all_windows(root_window,0)) {
		if (w != root_window && mapped.find(w) != mapped.end()) {
			Result<std::vector<std::string>> window_types = AppWindow(display,w).get_atom("_NET_WM_WINDOW_TYPE");
			Result<std::vector<std::string>> protocols = AppWindow(display,w).get_atom("WM_PROTOCOLS");

			auto temp = AppWindow(display,w).get_name();

			if (window_types.is_ok()) {
				std::vector<std::string> types = window_types.ok();

				// popup menu: do nothing
				if (std::find(types.begin(),types.end(), "_NET_WM_WINDOW_TYPE_POPUP_MENU") != types.end()) {
					continue;		
				} else if (std::find(types.begin(),types.end(), "_NET_WM_WINDOW_TYPE_DIALOG") != types.end()) { // floating by default
					AppWindow temp(display,w);
					temp.set_floating(true);				
					all_application_windows.push_back(temp);
				} else if (std::find(types.begin(),types.end(),"_NET_WM_WINDOW_TYPE_NORMAL") != types.end()) { // normal window
					all_application_windows.push_back(AppWindow(display,w));
				}

			} else if (protocols.is_ok()) {
				std::vector<std::string> prot = protocols.ok();
				if (std::find(prot.begin(),prot.end(),"WM_DELETE_WINDOW") != prot.end()) {
					all_application_windows.push_back(AppWindow(display,w));
				}
			}
		}
	}

	// remove old windows from the workspaces
	for (Workspace *w: workspaces) {
		w->remove_old_windows(all_application_windows);
	}
	std::vector<AppWindow> all_workspace_windows;
	for (Workspace *w: workspaces) {
		for (AppWindow aw: w->get_all_used_windows()) {
			all_workspace_windows.push_back(aw);
		}
	}

	for (AppWindow w: all_application_windows) {
		if (std::find(all_workspace_windows.begin(),all_workspace_windows.end(), w) == all_workspace_windows.end()) {
			// new workspace detected
			current_workspace->add_window(w);
		}
	}

}

void WindowManager::print_windows() {
	for (int i = 0; i < num_workspaces; i++) {
		std::cout << "Workspace " << i << ": ";
		for (AppWindow w: workspaces[i]->get_all_used_windows()) {
			Result<std::string> temp = w.get_name();
			if (temp.is_ok()) {
				std::cout << temp.ok() << " ";
			} else {
				std::cout << "\"\"";
			}
		}
		std::cout << std::endl;
	}
}

void WindowManager::tile() {
	for (int i = 0; i < num_workspaces; i++) {
		bool hide_window=true;
		for (Workspace *w: monitor_workspace) {
			if (w == workspaces[i]) {
				hide_window=false;
			}
		}

		// not one of the shown ones
		if (hide_window) {
			workspaces[i]->hide_all();
		}
	}

	for (int i = 0; i < num_monitors; i++) {
		monitor_workspace[i]->tile(i == focused_monitor, monitors[i]);
	}
}

void WindowManager::add_key_bind(std::string keybind, std::function<void (WindowManager *)> func) {
	keypress_handler->add_key(keybind, func);
}

void WindowManager::move_focus_down() {
	current_workspace->move_focus_down();
}
void WindowManager::move_focus_up() {
	current_workspace->move_focus_up();
}

void WindowManager::move_floating_left() {
	current_workspace->move_floating_left();	
}
void WindowManager::move_floating_right() {
	current_workspace->move_floating_right();
}
void WindowManager::move_floating_up() {
	current_workspace->move_floating_up();	
}
void WindowManager::move_floating_down() {
	current_workspace->move_floating_down();
}

void WindowManager::resize_floating_left() {
	current_workspace->resize_floating_left();	
}
void WindowManager::resize_floating_right() {
	current_workspace->resize_floating_right();
}
void WindowManager::resize_floating_up() {
	current_workspace->resize_floating_up();	
}
void WindowManager::resize_floating_down() {
	current_workspace->resize_floating_down();
}

void WindowManager::move_window_up() {
	current_workspace->move_window_up();
}

void WindowManager::move_window_down() {
	current_workspace->move_window_down();
}

void WindowManager::kill_window() {
	current_workspace->kill_window();
}

void WindowManager::move_window_to_workspace(int workspace) {
	if (workspaces[workspace] == current_workspace) {
		return;
	}

	Result<AppWindow> to_move = current_workspace->get_focused_window();

	if (to_move.is_ok()) {
		current_workspace->delete_window(to_move.ok());
		workspaces[workspace]->add_window(to_move.ok());
	}
}

void WindowManager::view_workspace(int workspace) {
	if (workspaces[workspace] == current_workspace) { // do nothing, already viewing
		return;
	}

	for (Workspace *w: monitor_workspace) { // on other monitor, change to that monitor
		if (workspaces[workspace] == w) {
			current_workspace = workspaces[workspace];
			return;
		}
	}

	// otherwise, change current monitor to this new workspace
	for (int i = 0; i < num_monitors; i++) {
		if (monitor_workspace[i] == current_workspace) {
			monitor_workspace[i] = workspaces[workspace];
			current_workspace = workspaces[workspace];
			return;
		}
	}
}

std::map<Window,Rect> WindowManager::get_default_floating_sizes() {
	return default_floating_sizes;
}

void WindowManager::remove_window(AppWindow w) {
	mapped.erase(w.window());
}

void WindowManager::toggle_fullscreen() {
	current_workspace->toggle_fullscreen();
}
void WindowManager::toggle_floating() {
	current_workspace->toggle_floating();
}
