#include <algorithm>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include "AppWindow.h"

AppWindow::AppWindow(Display *d, Window w, bool tile): display(d), raw_window(w) {
	do_tile=tile;
}


void AppWindow::move_resize(int x, int y, int w, int h, int border_size) {
	XMoveResizeWindow(display, raw_window, x,y,w-border_size*2,h-border_size*2);
}

void AppWindow::set_fullscreen(bool b) {
	is_fullscreen=b;
}
void AppWindow::set_floating(bool b) {
	is_floating=b;
}

void AppWindow::set_border_size(int size) {
	border_size=size;
	Colormap c = XDefaultColormapOfScreen(DefaultScreenOfDisplay(display));
	XColor temp;
	temp.blue=255*256;
	temp.red=255*256;
	temp.green=255*256;
	XAllocColor(display, c, &temp);
	XSetWindowBorder(display, raw_window, temp.pixel);
	XSetWindowBorderWidth(display, raw_window, size);
}

void AppWindow::grab_focus() {
	XSetInputFocus(display, raw_window, RevertToPointerRoot, CurrentTime);
}

void AppWindow::hide() {
	move_resize(-1000, 0,100,100);
}

void AppWindow::kill() {
	std::vector<std::string> protocols = get_atom("WM_PROTOCOLS").ok();
	if (std::find(protocols.begin(),protocols.end(), "WM_DELETE_WINDOW") != protocols.end()) {
		XEvent ev;
		ev.xclient.type = ClientMessage;
		ev.xclient.window = raw_window;
		ev.xclient.message_type = XInternAtom(display, "WM_PROTOCOLS",true);
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = XInternAtom(display, "WM_DELETE_WINDOW", false);
		ev.xclient.data.l[1] = CurrentTime;
		XSendEvent(display,raw_window,false,NoEventMask,&ev);
	} else {
		XKillClient(display, raw_window);
	}
	XFlush(display);
}

Result<unsigned long> AppWindow::get_cardinal(std::string name) {
	Atom type = XInternAtom(display, name.c_str(), true);
	Atom actual_type;
	int format;
	unsigned long n_items, bytes;
	unsigned char *values;
	int status = XGetWindowProperty(
		display,
		raw_window,
		type,
		0,
		sizeof(unsigned long),
		false,
		XA_CARDINAL,
		&actual_type,
	    &format,
		&n_items,
		&bytes,
		&values
	);
	if (actual_type == None) {
		return Result<unsigned long>::Err("Window did not have atom type");
	}
	if (status == Success) {
		unsigned long ret = ((unsigned long*)values)[0];
		return Result<unsigned long>::Ok(ret);
	} else {
		return Result<unsigned long>::Err("GetWindowProperty did not return success");
	}
}

Result<std::vector<std::string>> AppWindow::get_atom(std::string name) {
	Atom type = XInternAtom(display, name.c_str(), true);
	Atom actual_type;
	int format;
	unsigned long n_items, bytes;
	unsigned char *values;
	int status = XGetWindowProperty(
		display,
		raw_window,
		type,
		0,
		sizeof(Atom),
		false,
		XA_ATOM,
		&actual_type,
	    &format,
		&n_items,
		&bytes,
		&values
	);

	std::vector<std::string> ret;
	if (status == Success && actual_type != None && format != 0) {
		for (int i = 0; i < n_items; i++) {
			Atom temp = ((Atom*)values)[i];
			char *actual_name = XGetAtomName(display, temp);
			ret.push_back(std::string(actual_name));
			XFree(actual_name);
		}
		return Result<std::vector<std::string>>::Ok(ret);
	}
	return Result<std::vector<std::string>>::Err("XGetProperty did not return success");
}

Result<std::string> AppWindow::get_name() const {
	XTextProperty t;
	if (!XGetWMName(display,raw_window,&t) || t.value == nullptr) {
		return Result<std::string>::Err("");
	} else {
		return Result<std::string>::Ok((char*)t.value);
	}
}
bool AppWindow::get_fullscreen() const {return is_fullscreen;}
bool AppWindow::get_floating() const {return is_floating;}
int AppWindow::get_border_size() const {return border_size;}
Window AppWindow::window() const {return raw_window;}

bool AppWindow::operator==(const AppWindow &rhs) const {
	return window() == rhs.window();
}
