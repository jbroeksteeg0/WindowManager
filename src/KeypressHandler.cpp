#include "KeypressHandler.h"
#include <vector>
#include <cassert>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

// assume key is in the form S M A
// everything but the first one are modifiers

KeypressHandler::KeypressHandler(WindowManager *w,Display *d, Window r) {
	wm=w;
	display=d;
	root=r;

}

void KeypressHandler::on_key_down(unsigned int modifiers, int key) {
	for (auto pair: key_binds) {
		std::pair<unsigned int, int> parsed = parse_key(pair.first);
		if (modifiers == parsed.first && key == parsed.second) {
			pair.second(wm);
			return;
		}
	}
}

void KeypressHandler::add_key(std::string s, std::function<void (WindowManager *)> f) {
	std::pair<unsigned int, int> parsed = parse_key(s);
	key_binds.push_back({s,f});
	XGrabKey(
		display,
		parsed.second,
		parsed.first,
		root,
		false,
		GrabModeAsync,
		GrabModeAsync
	);
}

std::pair<unsigned int, int> KeypressHandler::parse_key(std::string key) {
	unsigned int modifiers = 0;

	std::vector<std::string> parts;
	std::string passed = "";
	for (char ch: key) {
		if (ch == ' ') {
			assert(passed != "");
			parts.push_back(passed);
			passed="";
		} else {
			passed+=ch;
		}
	}

	if (passed.empty()) {
		printf("Failed to parse key %s\n",key.c_str());
		exit(1);
	}
	parts.push_back(passed);

	for (int i = 0; i < parts.size()-1; i++) {
		if (parts[i] == "S") {
			modifiers |= ShiftMask;
		} else if (parts[i] == "C") {
			modifiers |= ControlMask;
		} else if (parts[i] == "M") {
			modifiers |= Mod4Mask;
		} else if (parts[i] == "A") {
			modifiers |= Mod1Mask;
		} else {
			exit(1);
		}
	}

	KeySym ks = XStringToKeysym(parts.back().c_str());

	if (ks == NoSymbol) {
		printf("Could not find key symbol: %s\n",parts.back().c_str());
		exit(1);
	}

	return {modifiers, XKeysymToKeycode(display, ks)};
}
