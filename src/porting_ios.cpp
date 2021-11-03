/*
MultiCraft

Copyright (C) 2014-2021 Maksim Gamarnik [MoNTE48] MoNTE48@mail.ua
Copyright (C) 2016-2019 sfan5

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 3.0 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <string>

#include "porting.h"
#include "config.h"

static void *uiviewcontroller;

namespace porting {
	void initializePathsiOS() {
		char buf[128];

		ioswrap_paths(PATH_DOCUMENTS, buf, sizeof(buf));
		path_user = std::string(buf);
		ioswrap_paths(PATH_LIBRARY_SUPPORT, buf, sizeof(buf));
		path_share = std::string(buf);
		path_locale = std::string(buf) + "/locale";
		ioswrap_paths(PATH_LIBRARY_CACHE, buf, sizeof(buf));
		path_cache = std::string(buf);
	}

	void copyAssets() {
		ioswrap_assets();
	}

	float getDisplayDensity() {
		static bool firstrun = true;
		static float scale;

		if (firstrun) {
			scale = ioswrap_scale();
			firstrun = false;
		}

		return scale;
	}

	v2u32 getDisplaySize() {
		return porting::getWindowSize();
	}

	void setViewController(void *v) {
		uiviewcontroller = v;
		ioswrap_init_viewc(v);
	}

	void showInputDialog(const std::string &acceptButton, const std::string &hint,
						 const std::string &current, int editType) {
		ioswrap_show_dialog(uiviewcontroller, acceptButton.c_str(), hint.c_str(), current.c_str(), editType);
	}

	int getInputDialogState() {
		return ioswrap_get_dialog(NULL);
	}

	std::string getInputDialogValue() {
		const char *str;
		ioswrap_get_dialog(&str);
		return std::string(str);
	}

	void notifyServerConnect(bool is_multiplayer) {
		ioswrap_server_connect(is_multiplayer);
	}

	void notifyExitGame() {
		ioswrap_exit_game();
	}
}

extern int real_main(int argc, char *argv[]);

void irrlicht_main() {
	ioswrap_init();
	static const char *args[] = {
			PROJECT_NAME,
	};
	real_main(1, (char **) args);
}
