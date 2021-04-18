#pragma once

#include <iostream>
#include <functional>
#include <vector>
#include <X11/Xlib.h>

class WindowManager;
#include "WindowManager.h"

class KeypressHandler {
	Display *display;
	Window root;
	WindowManager *wm;
	std::vector<std::pair<std::string,std::function<void(WindowManager*)>>> key_binds;
	std::pair<unsigned int, int> parse_key(std::string key);

	unsigned int current_mod_mask = 0;
public:
	KeypressHandler(WindowManager*,Display*,Window);

	void on_key_down(unsigned int modifiers, int key); // key bindings AND modifier keys
	
	void add_key(std::string s, std::function<void(WindowManager*)> f);
};
