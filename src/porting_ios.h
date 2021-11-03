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

#pragma once

#ifndef __IOS__
#error This file should only be included on iOS
#endif

#include "ioswrap.h"

namespace porting {
	void initializePathsiOS();

	void copyAssets();

	void setViewController(void *v);

	void showInputDialog(const std::string &acceptButton, const std::string &hint,
	                     const std::string &current, int editType);

	int getInputDialogState();

	std::string getInputDialogValue();

	void notifyServerConnect(bool is_multiplayer);

	void notifyExitGame();
}
