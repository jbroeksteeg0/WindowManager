#include <iostream>
#include "WindowManager.h"

int main() {
	WindowManager wm = WindowManager(8);

	wm.add_key_bind("M P", [](WindowManager *wm) {system("dmenu_run &");});
	wm.add_key_bind("M T", [](WindowManager *wm) {system("konsole &");});

	wm.add_key_bind("M J", [](WindowManager *wm) { wm->move_focus_down(); });
	wm.add_key_bind("M K", [](WindowManager *wm) { wm->move_focus_up(); });
	wm.add_key_bind("M S J", [](WindowManager *wm) { wm->move_window_down(); });
	wm.add_key_bind("M S K", [](WindowManager *wm) { wm->move_window_up(); });

	wm.add_key_bind("M A H", [](WindowManager *wm) { wm->move_floating_left(); });
	wm.add_key_bind("M A L", [](WindowManager *wm) { wm->move_floating_right(); });
	wm.add_key_bind("M A K", [](WindowManager *wm) { wm->move_floating_up(); });
	wm.add_key_bind("M A J", [](WindowManager *wm) { wm->move_floating_down(); });

	wm.add_key_bind("M S A H", [](WindowManager *wm) { wm->resize_floating_left(); });
	wm.add_key_bind("M S A L", [](WindowManager *wm) { wm->resize_floating_right(); });
	wm.add_key_bind("M S A K", [](WindowManager *wm) { wm->resize_floating_up(); });
	wm.add_key_bind("M S A J", [](WindowManager *wm) { wm->resize_floating_down(); });

	wm.add_key_bind("M S F", [](WindowManager *wm) { wm->toggle_fullscreen(); });
	wm.add_key_bind("M F", [](WindowManager *wm) { wm->toggle_floating(); });
	wm.add_key_bind("M S Q", [](WindowManager *wm) { wm->kill_window(); });
	
	for (int i = 0; i < 8; i++) {
		wm.add_key_bind("M S " + std::to_string(i+1), [i](WindowManager *wm) { wm->move_window_to_workspace(i);});
		wm.add_key_bind("M " + std::to_string(i+1), [i](WindowManager *wm) { wm->view_workspace(i); });
	}
	wm.run();
	
	return 0;
}
