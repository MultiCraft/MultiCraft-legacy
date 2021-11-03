/*
Copyright (C) 2014 sapier
Copyright (C) 2018 srifqi, Muhammad Rifqi Priyo Susanto
		<muhammadrifqipriyosusanto@gmail.com>
Copyright (C) 2014-2020 Maksim Gamarnik [MoNTE48] MoNTE48@mail.ua

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

#include "touchscreengui.h"
#include "irrlichttypes.h"
#include "irr_v2d.h"
#include "log.h"
#include "keycode.h"
#include "settings.h"
#include "gettime.h"
#include "util/numeric.h"
#include "porting.h"
#include "guiscalingfilter.h"

#include <iostream>
#include <algorithm>

using namespace irr::core;

const char **button_imagenames = (const char *[]) {
	"inventory_btn.png",
	"drop_btn.png",
	"jump_btn.png",
	"down_btn.png",
	"escape_btn.png",
	"minimap_btn.png",
	"rangeview_btn.png",
	"camera_btn.png",
	"chat_btn.png"
//	"noclip_btn.png",
//	"fast_btn.png"
};

const char **joystick_imagenames = (const char *[]) {
	"joystick_off.png",
	"joystick_bg.png",
	"joystick_center.png"
};

static irr::EKEY_CODE id2keycode(touch_gui_button_id id)
{
	std::string key = "";
	switch (id) {
		case forward_id:
			key = "forward";
			break;
		case left_id:
			key = "left";
			break;
		case right_id:
			key = "right";
			break;
		case backward_id:
			key = "backward";
			break;
		case inventory_id:
			key = "inventory";
			break;
		case drop_id:
			key = "drop";
			break;
		case jump_id:
			key = "jump";
			break;
		case crunch_id:
			key = "sneak";
			break;
		case escape_id:
			return irr::KEY_ESCAPE;
		case minimap_id:
			key = "minimap";
			break;
		case range_id:
			key = "rangeselect";
			break;
		case camera_id:
			key = "camera_mode";
			break;
		case chat_id:
			key = "chat";
			break;
	/*	case noclip_id:
			key = "noclip";
			break;
		case fast_id:
			key = "fast";
			break;	*/
		default:
			break;
	}
	assert(!key.empty());
	return keyname_to_keycode(g_settings->get("keymap_" + key).c_str());
}

TouchScreenGUI *g_touchscreengui;

static void load_button_texture(button_info *btn, const char *path,
		const rect<s32> &button_rect, ISimpleTextureSource *tsrc, video::IVideoDriver *driver)
{
	unsigned int tid;
	video::ITexture *texture = guiScalingImageButton(driver,
			tsrc->getTexture(path, &tid), button_rect.getWidth(),
			button_rect.getHeight());
	if (texture) {
		btn->guibutton->setUseAlphaChannel(true);
		if (g_settings->getBool("gui_scaling_filter")) {
			rect<s32> txr_rect = rect<s32>(0, 0, button_rect.getWidth(), button_rect.getHeight());
			btn->guibutton->setImage(texture, txr_rect);
			btn->guibutton->setPressedImage(texture, txr_rect);
			btn->guibutton->setScaleImage(false);
		} else {
			btn->guibutton->setImage(texture);
			btn->guibutton->setPressedImage(texture);
			btn->guibutton->setScaleImage(true);
		}
		btn->guibutton->setDrawBorder(false);
		btn->guibutton->setText(L"");
	}
}

TouchScreenGUI::TouchScreenGUI(IrrlichtDevice *device, IEventReceiver *receiver):
	m_device(device),
	m_guienv(device->getGUIEnvironment()),
	m_visible(false),
	m_receiver(receiver)
{
	for (auto &button : m_buttons) {
		button.guibutton     = nullptr;
		button.repeatcounter = -1;
		button.repeatdelay   = BUTTON_REPEAT_DELAY;
	}

	m_touchscreen_threshold = g_settings->getU16("touchscreen_threshold");
	m_screensize = m_device->getVideoDriver()->getScreenSize();
	button_size = MYMIN(m_screensize.Y / 4.5f,
			porting::getDisplayDensity() *
			g_settings->getFloat("hud_scaling") * 65.0f);
}

void TouchScreenGUI::initButton(touch_gui_button_id id, const rect<s32> &button_rect,
		const std::wstring &caption, bool immediate_release, float repeat_delay)
{
	button_info *btn       = &m_buttons[id];
	btn->guibutton         = m_guienv->addButton(button_rect, nullptr, id, caption.c_str());
	btn->guibutton->grab();
	btn->repeatcounter     = -1;
	btn->repeatdelay       = repeat_delay;
	btn->keycode           = id2keycode(id);
	btn->immediate_release = immediate_release;
	btn->ids.clear();

	load_button_texture(btn, button_imagenames[id], button_rect,
			m_texturesource, m_device->getVideoDriver());
}

button_info *TouchScreenGUI::initJoystickButton(touch_gui_button_id id,
		const rect<s32> &button_rect, int texture_id, bool visible)
{
	auto *btn = new button_info();
	btn->guibutton = m_guienv->addButton(button_rect, nullptr, id, L"O");
	btn->guibutton->setVisible(visible);
	btn->guibutton->grab();
	btn->ids.clear();

	load_button_texture(btn, joystick_imagenames[texture_id],
			button_rect, m_texturesource, m_device->getVideoDriver());

	return btn;
}

void TouchScreenGUI::init(ISimpleTextureSource *tsrc)
{
	assert(tsrc);

	m_visible       = true;
	m_texturesource = tsrc;

	/* Init joystick display "button"
	 * Joystick is placed on bottom left of screen.
	 */
	if (m_fixed_joystick) {
		m_joystick_btn_off = initJoystickButton(joystick_off_id,
				rect<s32>(button_size / 2,
						m_screensize.Y - button_size * 4.5,
						button_size * 4.5,
						m_screensize.Y - button_size / 2), 0);
	} else {
		m_joystick_btn_off = initJoystickButton(joystick_off_id,
				rect<s32>(button_size / 2,
						m_screensize.Y - button_size * 3.5,
						button_size * 3.5,
						m_screensize.Y - button_size / 2), 0);
	}

	m_joystick_btn_bg = initJoystickButton(joystick_bg_id,
			rect<s32>(button_size / 2,
					m_screensize.Y - button_size * 4.5,
					button_size * 4.5,
					m_screensize.Y - button_size / 2),
			1, false);

	m_joystick_btn_center = initJoystickButton(joystick_center_id,
			rect<s32>(0, 0, button_size * 1.5, button_size * 1.5), 2, false);


	// init inventory button
	initButton(inventory_id,
			rect<s32>(m_screensize.X - button_size * 1.5,
					  m_screensize.Y - button_size * 1.5,
					  m_screensize.X,
					  m_screensize.Y),
			L"inv", false);

	// init crunch button
	initButton(crunch_id,
			rect<s32>(m_screensize.X - button_size * 3,
					  m_screensize.Y - button_size / 1.5,
					  m_screensize.X - button_size * 1.5,
					  m_screensize.Y),
			L"H", false);

	// init jump button
	initButton(jump_id,
			rect<s32>(m_screensize.X - button_size * 3,
					  m_screensize.Y - button_size * 3,
					  m_screensize.X - button_size * 1.5,
					  m_screensize.Y - button_size * 1.5),
			L"x", false);

	// init drop button
	initButton(drop_id,
			rect<s32>(m_screensize.X - button_size,
					  m_screensize.Y / 2 - button_size * 1.5,
					  m_screensize.X,
					  m_screensize.Y / 2 - button_size / 2),
			L"drop", false);

	const bool minimap = g_settings->getBool("enable_minimap");

	double button_075 = 1;
	s32 button_05 = 1;
	double button_05b = 0;
	if (!minimap) {
		button_075 = 0.75;
		button_05 = 2;
		button_05b = button_size * 0.5;
	}

	// init pause button [1]
	initButton(escape_id,
			rect<s32>(m_screensize.X / 2 - button_size * 2 * button_075,
					  0,
					  m_screensize.X / 2 - button_size / button_05,
					  button_size),
			L"Exit", false);

	// init minimap button [2]
	if (minimap) {
		initButton(minimap_id,
				rect<s32>(m_screensize.X / 2 - button_size,
						  0,
						  m_screensize.X / 2,
						  button_size),
				L"minimap", false);
	}

	// init rangeselect button [3]
	initButton(range_id,
			rect<s32>(m_screensize.X / 2 - button_05b,
					  0,
					  m_screensize.X / 2 + button_size / button_05,
					  button_size),
			L"rangeview", false);

	// init camera button [4]
	initButton(camera_id,
			rect<s32>(m_screensize.X / 2 + button_size / button_05,
					  0,
					  m_screensize.X / 2 + button_size * 2 * button_075,
					  button_size),
			L"camera", false);

	// init chat button
	initButton(chat_id,
			rect<s32>(m_screensize.X - button_size * 1.25,
					  0,
					  m_screensize.X,
					  button_size),
			L"Chat", false);

	// init noclip button
/*	initButton(noclip_id,
			rect<s32>(m_screensize.X - button_size * 0.75,
					  m_screensize.Y - button_size * 4.75,
					  m_screensize.X,
					  m_screensize.Y - button_size * 4),
			   L"clip", false);
	// init fast button
	initButton(fast_id,
			rect<s32>(m_screensize.X - button_size * 0.75,
					  m_screensize.Y - button_size * 4,
					  m_screensize.X,
					  m_screensize.Y - button_size * 3.25),
			   L"fast", false); */

	initialized = true;
}

touch_gui_button_id TouchScreenGUI::getButtonID(s32 x, s32 y)
{
	IGUIElement *rootguielement = m_guienv->getRootGUIElement();

	if (rootguielement != nullptr) {
		gui::IGUIElement *element =
				rootguielement->getElementFromPoint(core::position2d<s32>(x, y));

		if (element)
			for (unsigned int i = 0; i < after_last_element_id; i++)
				if (element == m_buttons[i].guibutton)
					return (touch_gui_button_id) i;
	}

	return after_last_element_id;
}

touch_gui_button_id TouchScreenGUI::getButtonID(size_t eventID)
{
	for (unsigned int i = 0; i < after_last_element_id; i++) {
		button_info *btn = &m_buttons[i];

		auto id = std::find(btn->ids.begin(), btn->ids.end(), eventID);

		if (id != btn->ids.end())
			return (touch_gui_button_id) i;
	}

	return after_last_element_id;
}

bool TouchScreenGUI::isHUDButton(const SEvent &event)
{
	// check if hud item is pressed
	for (auto &hud_rect : m_hud_rects) {
		if (hud_rect.second.isPointInside(v2s32(event.TouchInput.X,
				event.TouchInput.Y))) {
			auto *translated = new SEvent();
			memset(translated, 0, sizeof(SEvent));
			translated->EventType = irr::EET_KEY_INPUT_EVENT;
			translated->KeyInput.Key         = (irr::EKEY_CODE) (KEY_KEY_1 + hud_rect.first);
			translated->KeyInput.Control     = false;
			translated->KeyInput.Shift       = false;
			translated->KeyInput.PressedDown = true;
			m_receiver->OnEvent(*translated);
			m_hud_ids[event.TouchInput.ID]   = translated->KeyInput.Key;
			delete translated;
			return true;
		}
	}
	return false;
}

void TouchScreenGUI::handleButtonEvent(touch_gui_button_id button,
		size_t eventID, bool action)
{
	button_info *btn = &m_buttons[button];
	auto *translated = new SEvent();
	memset(translated, 0, sizeof(SEvent));
	translated->EventType            = irr::EET_KEY_INPUT_EVENT;
	translated->KeyInput.Key         = btn->keycode;
	translated->KeyInput.Control     = false;
	translated->KeyInput.Shift       = false;
	translated->KeyInput.Char        = 0;

	// add this event
	if (action) {
		assert(std::find(btn->ids.begin(), btn->ids.end(), eventID) == btn->ids.end());

		btn->ids.push_back(eventID);

		if (btn->ids.size() > 1) return;

		btn->repeatcounter = 0;
		translated->KeyInput.PressedDown = true;
		translated->KeyInput.Key = btn->keycode;
		m_receiver->OnEvent(*translated);
	}

	// remove event
	if ((!action) || (btn->immediate_release)) {
		auto pos = std::find(btn->ids.begin(), btn->ids.end(), eventID);
		// has to be in touch list
		assert(pos != btn->ids.end());
		btn->ids.erase(pos);

		if (!btn->ids.empty())
			return;

		translated->KeyInput.PressedDown = false;
		btn->repeatcounter               = -1;
		m_receiver->OnEvent(*translated);
	}
	delete translated;
}

void TouchScreenGUI::handleReleaseEvent(size_t evt_id)
{
	touch_gui_button_id button = getButtonID(evt_id);

	if (button != after_last_element_id) {
		// handle button events
		handleButtonEvent(button, evt_id, false);
	} else if (evt_id == m_move_id) {
		// handle the point used for moving view
		m_move_id = -1;

		// if this pointer issued a mouse event issue symmetric release here
		if (m_move_sent_as_mouse_event) {
			auto *translated = new SEvent;
			memset(translated, 0, sizeof(SEvent));
			translated->EventType               = EET_MOUSE_INPUT_EVENT;
			translated->MouseInput.X            = m_move_downlocation.X;
			translated->MouseInput.Y            = m_move_downlocation.Y;
			translated->MouseInput.Shift        = false;
			translated->MouseInput.Control      = false;
			translated->MouseInput.ButtonStates = 0;
			translated->MouseInput.Event        = EMIE_LMOUSE_LEFT_UP;
			m_receiver->OnEvent(*translated);
			delete translated;
		} else if (!m_move_has_really_moved) {
			auto *translated = new SEvent;
			memset(translated, 0, sizeof(SEvent));
			translated->EventType               = EET_MOUSE_INPUT_EVENT;
			translated->MouseInput.X            = m_move_downlocation.X;
			translated->MouseInput.Y            = m_move_downlocation.Y;
			translated->MouseInput.Shift        = false;
			translated->MouseInput.Control      = false;
			translated->MouseInput.ButtonStates = 0;
			translated->MouseInput.Event        = EMIE_LMOUSE_LEFT_UP;
			m_receiver->OnEvent(*translated);
			delete translated;
			quickTapDetection();
			m_shootline = m_device
						->getSceneManager()
						->getSceneCollisionManager()
						->getRayFromScreenCoordinates(
							v2s32(m_move_downlocation.X, m_move_downlocation.Y));
		}
	}

	// handle joystick
	else if (evt_id == m_joystick_id) {
		m_joystick_id = -1;

		// reset joystick
		for (unsigned int i = 0; i < 4; i++)
			m_joystick_status[i] = false;
		applyJoystickStatus();

		if (m_visible)
			m_joystick_btn_off->guibutton->setVisible(true);
		m_joystick_btn_bg->guibutton->setVisible(false);
		m_joystick_btn_center->guibutton->setVisible(false);
	} else {
		infostream
			<< "TouchScreenGUI::translateEvent released unknown button: "
			<< evt_id << std::endl;
	}

	for (auto iter = m_known_ids.begin();
			iter != m_known_ids.end(); ++iter) {
		if (iter->id == evt_id) {
			m_known_ids.erase(iter);
			break;
		}
	}
}

void TouchScreenGUI::moveJoystick(const SEvent &event, float dx, float dy) {
	m_joystick_has_really_moved = true;
	double distance = sqrt(dx * dx + dy * dy);

	// angle in degrees
	double angle = acos(dx / distance) * 180 / M_PI;
	if (dy < 0)
		angle *= -1;
	// rotate to make comparing easier
	angle = fmod(angle + 180 + 22.5, 360);

	// reset state before applying
	for (bool & joystick_status : m_joystick_status)
		joystick_status = false;

	if (distance <= m_touchscreen_threshold) {
		// do nothing
	} else if (angle < 45)
		m_joystick_status[j_left] = true;
	else if (angle < 90) {
		m_joystick_status[j_forward] = true;
		m_joystick_status[j_left] = true;
	} else if (angle < 135)
		m_joystick_status[j_forward] = true;
	else if (angle < 180) {
		m_joystick_status[j_forward] = true;
		m_joystick_status[j_right] = true;
	} else if (angle < 225)
		m_joystick_status[j_right] = true;
	else if (angle < 270) {
		m_joystick_status[j_backward] = true;
		m_joystick_status[j_right] = true;
	} else if (angle < 315)
		m_joystick_status[j_backward] = true;
	else if (angle <= 360) {
		m_joystick_status[j_backward] = true;
		m_joystick_status[j_left] = true;
	}

	if (distance > button_size * 1.5) {
		m_joystick_status[j_special1] = true;
		// move joystick "button"
		s32 ndx = button_size * dx / distance * 1.5f - button_size / 2.0f * 1.5f;
		s32 ndy = button_size * dy / distance * 1.5f - button_size / 2.0f * 1.5f;
		if (m_fixed_joystick) {
			m_joystick_btn_center->guibutton->setRelativePosition(v2s32(
				button_size * 5 / 2 + ndx,
				m_screensize.Y - button_size * 5 / 2 + ndy));
		} else {
			m_joystick_btn_center->guibutton->setRelativePosition(v2s32(
				m_pointerpos[event.TouchInput.ID].X + ndx,
				m_pointerpos[event.TouchInput.ID].Y + ndy));
		}
	} else {
		m_joystick_btn_center->guibutton->setRelativePosition(v2s32(
				event.TouchInput.X - button_size / 2.0f * 1.5f,
				event.TouchInput.Y - button_size / 2.0f * 1.5f));
	}
}

void TouchScreenGUI::translateEvent(const SEvent &event)
{
	if (!m_visible) {
		infostream
			<< "TouchScreenGUI::translateEvent got event but not visible!"
			<< std::endl;
		return;
	}

	if (event.EventType != EET_TOUCH_INPUT_EVENT)
		return;

	if (event.TouchInput.Event == ETIE_PRESSED_DOWN) {
		/*
		 * Add to own copy of event list...
		 * android would provide this information but Irrlicht guys don't
		 * wanna design a efficient interface
		 */
		id_status toadd{};
		toadd.id = event.TouchInput.ID;
		toadd.X  = event.TouchInput.X;
		toadd.Y  = event.TouchInput.Y;
		m_known_ids.push_back(toadd);

		size_t eventID = event.TouchInput.ID;

		touch_gui_button_id button =
				getButtonID(event.TouchInput.X, event.TouchInput.Y);

		// handle button events
		if (button != after_last_element_id) {
			handleButtonEvent(button, eventID, true);
		} else if (isHUDButton(event)) {
			// already handled in isHUDButton()
		} else {
			// handle non button events
			s32 dxj = event.TouchInput.X - button_size * 5.0f / 2.0f;
			s32 dyj = event.TouchInput.Y - m_screensize.Y + button_size * 5.0f / 2.0f;

			/* Select joystick when left 1/3 of screen dragged or
			 * when joystick tapped (fixed joystick position)
			 */
			bool inside_joystick =
				m_fixed_joystick
					? dxj * dxj + dyj * dyj <= button_size * button_size * 1.5 * 1.5
					: event.TouchInput.X < m_screensize.X / 3.0f;
			if (inside_joystick) {
				// If we don't already have a starting point for joystick make this the one.
				if (m_joystick_id == -1) {
					m_joystick_id               = event.TouchInput.ID;
					m_joystick_has_really_moved = false;

					m_joystick_btn_off->guibutton->setVisible(false);
					m_joystick_btn_bg->guibutton->setVisible(true);
					m_joystick_btn_center->guibutton->setVisible(true);

					if (m_fixed_joystick) {
						moveJoystick(event, dxj, dyj);
					} else {
						m_joystick_btn_bg->guibutton->setRelativePosition(v2s32(
								event.TouchInput.X - button_size * 3.0f / 1.5f,
								event.TouchInput.Y - button_size * 3.0f / 1.5f));
						m_joystick_btn_center->guibutton->setRelativePosition(v2s32(
								event.TouchInput.X - button_size / 2.0f * 1.5f,
								event.TouchInput.Y - button_size / 2.0f * 1.5f));
					}
				}
			} else {
				// If we don't already have a moving point make this the moving one.
				if (m_move_id == -1) {
					m_move_id                  = event.TouchInput.ID;
					m_move_has_really_moved    = false;
					m_move_downtime            = porting::getTimeMs();
					m_move_downlocation        = v2s32(event.TouchInput.X, event.TouchInput.Y);
					m_move_sent_as_mouse_event = false;
				}
			}
		}

		m_pointerpos[event.TouchInput.ID] = v2s32(event.TouchInput.X, event.TouchInput.Y);
	}
	else if (event.TouchInput.Event == ETIE_LEFT_UP) {
		verbosestream
			<< "Up event for pointerid: " << event.TouchInput.ID << std::endl;
		handleReleaseEvent(event.TouchInput.ID);
	} else {
		assert(event.TouchInput.Event == ETIE_MOVED);

		if (m_pointerpos[event.TouchInput.ID] ==
				v2s32(event.TouchInput.X, event.TouchInput.Y))
			return;

		if (m_move_id != -1) {
			if ((event.TouchInput.ID == m_move_id) &&
				(!m_move_sent_as_mouse_event || !g_settings->getBool("touchtarget"))) {

				double distance = sqrt(
						(m_pointerpos[event.TouchInput.ID].X - event.TouchInput.X) *
						(m_pointerpos[event.TouchInput.ID].X - event.TouchInput.X) +
						(m_pointerpos[event.TouchInput.ID].Y - event.TouchInput.Y) *
						(m_pointerpos[event.TouchInput.ID].Y - event.TouchInput.Y));

				if ((distance > m_touchscreen_threshold) ||
						(m_move_has_really_moved)) {
					m_move_has_really_moved = true;
					s32 X = event.TouchInput.X;
					s32 Y = event.TouchInput.Y;

					// update camera_yaw and camera_pitch
					s32 dx = X - m_pointerpos[event.TouchInput.ID].X;
					s32 dy = Y - m_pointerpos[event.TouchInput.ID].Y;

					// adapt to similar behaviour as pc screen
					double d = g_settings->getFloat("mouse_sensitivity");

					m_camera_yaw_change -= dx * d;
					m_camera_pitch = MYMIN(MYMAX(m_camera_pitch + (dy * d), -180), 180);

					// update shootline
					m_shootline = m_device
							->getSceneManager()
							->getSceneCollisionManager()
							->getRayFromScreenCoordinates(v2s32(X, Y));
					m_pointerpos[event.TouchInput.ID] = v2s32(X, Y);
				}
			} else if ((event.TouchInput.ID == m_move_id) &&
					(m_move_sent_as_mouse_event)) {
				m_shootline = m_device
						->getSceneManager()
						->getSceneCollisionManager()
						->getRayFromScreenCoordinates(
								v2s32(event.TouchInput.X, event.TouchInput.Y));
			}
		}

		if (m_joystick_id != -1 && event.TouchInput.ID == m_joystick_id) {
			s32 X = event.TouchInput.X;
			s32 Y = event.TouchInput.Y;

			s32 dx = X - m_pointerpos[event.TouchInput.ID].X;
			s32 dy = Y - m_pointerpos[event.TouchInput.ID].Y;
			if (m_fixed_joystick) {
				dx = X - button_size * 5 / 2;
				dy = Y - m_screensize.Y + button_size * 5 / 2;
			}

			s32 dxj = event.TouchInput.X - button_size * 5.0f / 2.0f;
			s32 dyj = event.TouchInput.Y - m_screensize.Y + button_size * 5.0f / 2.0f;
			bool inside_joystick = (dxj * dxj + dyj * dyj <= button_size * button_size * 1.5 * 1.5);

			if (m_joystick_has_really_moved ||
					(!m_joystick_has_really_moved && inside_joystick) ||
					(!m_fixed_joystick &&
					dx * dx + dy * dy > m_touchscreen_threshold * m_touchscreen_threshold)) {
				moveJoystick(event, dx, dy);
			}
		}

		if (m_move_id == -1 && m_joystick_id == -1)
			handleChangedButton(event);
	}
}

void TouchScreenGUI::handleChangedButton(const SEvent &event)
{
	for (unsigned int i = 0; i < after_last_element_id; i++) {
		if (m_buttons[i].ids.empty())
			continue;

		for (auto iter = m_buttons[i].ids.begin();
				iter != m_buttons[i].ids.end(); ++iter) {
			if (event.TouchInput.ID == *iter) {
				int current_button_id =
						getButtonID(event.TouchInput.X, event.TouchInput.Y);

				if (current_button_id == i)
					continue;

				// remove old button
				handleButtonEvent((touch_gui_button_id) i, *iter, false);

				if (current_button_id == after_last_element_id)
					return;

				handleButtonEvent((touch_gui_button_id) current_button_id, *iter, true);
				return;
			}
		}
	}

	int current_button_id = getButtonID(event.TouchInput.X, event.TouchInput.Y);

	if (current_button_id == after_last_element_id)
		return;

	button_info *btn = &m_buttons[current_button_id];
	if (std::find(btn->ids.begin(), btn->ids.end(), event.TouchInput.ID)
			== btn->ids.end())
		handleButtonEvent((touch_gui_button_id) current_button_id,
				event.TouchInput.ID, true);
}

// Punch or left click
bool TouchScreenGUI::quickTapDetection()
{
	m_key_events[0].down_time = m_key_events[1].down_time;
	m_key_events[0].x         = m_key_events[1].x;
	m_key_events[0].y         = m_key_events[1].y;

	// ignore the occasional touch
	u64 delta = porting::getDeltaMs(m_move_downtime, porting::getTimeMs());
	if (delta < 50)
		return false;

	auto *translated = new SEvent();
	memset(translated, 0, sizeof(SEvent));
	translated->EventType               = EET_MOUSE_INPUT_EVENT;
	translated->MouseInput.X            = m_key_events[0].x;
	translated->MouseInput.Y            = m_key_events[0].y;
	translated->MouseInput.Shift        = false;
	translated->MouseInput.Control      = false;
	translated->MouseInput.ButtonStates = EMBSM_RIGHT;

	// update shootline
	m_shootline = m_device
			->getSceneManager()
			->getSceneCollisionManager()
			->getRayFromScreenCoordinates(v2s32(m_key_events[0].x, m_key_events[0].y));

	translated->MouseInput.Event = EMIE_RMOUSE_PRESSED_DOWN;
	verbosestream << "TouchScreenGUI::translateEvent right click press" << std::endl;
	m_receiver->OnEvent(*translated);

	translated->MouseInput.ButtonStates = 0;
	translated->MouseInput.Event = EMIE_RMOUSE_LEFT_UP;
	verbosestream << "TouchScreenGUI::translateEvent right click release" << std::endl;
	m_receiver->OnEvent(*translated);
	delete translated;
	return true;
}

void TouchScreenGUI::applyJoystickStatus()
{
	for (unsigned int i = 0; i < 5; i++) {
		if (i == 4)
			continue;

		SEvent translated{};
		translated.EventType            = irr::EET_KEY_INPUT_EVENT;
		translated.KeyInput.Key         = id2keycode(m_joystick_names[i]);
		translated.KeyInput.PressedDown = false;
		m_receiver->OnEvent(translated);

		if (m_joystick_status[i]) {
			translated.KeyInput.PressedDown = true;
			m_receiver->OnEvent(translated);
		}
	}
}

TouchScreenGUI::~TouchScreenGUI()
{
	for (auto &button : m_buttons) {
		if (button.guibutton) {
			button.guibutton->drop();
			button.guibutton = nullptr;
		}
	}

	if (!initialized)
		return;

	if (m_joystick_btn_off->guibutton) {
		m_joystick_btn_off->guibutton->drop();
		m_joystick_btn_off->guibutton = nullptr;
	}

	if (m_joystick_btn_bg->guibutton) {
		m_joystick_btn_bg->guibutton->drop();
		m_joystick_btn_bg->guibutton = nullptr;
	}

	if (m_joystick_btn_center->guibutton) {
		m_joystick_btn_center->guibutton->drop();
		m_joystick_btn_center->guibutton = nullptr;
	}
}

void TouchScreenGUI::step(float dtime)
{
	// simulate keyboard repeats
	for (auto &button : m_buttons) {
		if (!button.ids.empty()) {
			button.repeatcounter += dtime;

			if (button.repeatcounter < button.repeatdelay)
				continue;

			button.repeatcounter            = 0;
			SEvent translated;
			memset(&translated, 0, sizeof(SEvent));
			translated.EventType            = irr::EET_KEY_INPUT_EVENT;
			translated.KeyInput.Key         = button.keycode;
			translated.KeyInput.PressedDown = false;
			m_receiver->OnEvent(translated);

			translated.KeyInput.PressedDown = true;
			m_receiver->OnEvent(translated);
		}
	}

	// joystick
	for (unsigned int i = 0; i < 4; i++) {
		if (m_joystick_status[i]) {
			applyJoystickStatus();
			break;
		}
	}

	// if a new placed pointer isn't moved for some time start digging
	if ((m_move_id != -1) &&
			(!m_move_has_really_moved) &&
			(!m_move_sent_as_mouse_event)) {
		u64 delta = porting::getDeltaMs(m_move_downtime, porting::getTimeMs());

		if (delta > MIN_DIG_TIME_MS) {
			m_shootline = m_device
					->getSceneManager()
					->getSceneCollisionManager()
					->getRayFromScreenCoordinates(
							v2s32(m_move_downlocation.X, m_move_downlocation.Y));

			SEvent translated;
			memset(&translated, 0, sizeof(SEvent));
			translated.EventType               = EET_MOUSE_INPUT_EVENT;
			translated.MouseInput.X            = m_move_downlocation.X;
			translated.MouseInput.Y            = m_move_downlocation.Y;
			translated.MouseInput.Shift        = false;
			translated.MouseInput.Control      = false;
			translated.MouseInput.ButtonStates = EMBSM_LEFT;
			translated.MouseInput.Event        = EMIE_LMOUSE_PRESSED_DOWN;
			verbosestream << "TouchScreenGUI::step left click press" << std::endl;
			m_receiver->OnEvent(translated);
			m_move_sent_as_mouse_event         = true;
		}
	}
}

void TouchScreenGUI::resetHud()
{
	m_hud_rects.clear();
}

void TouchScreenGUI::registerHudItem(int index, const rect<s32> &rect)
{
	m_hud_rects[index] = rect;
}

void TouchScreenGUI::Toggle(bool visible)
{
	m_visible = visible;
	for (auto &button : m_buttons) {
		if (button.guibutton)
			button.guibutton->setVisible(visible);
	}

	if (m_joystick_btn_off != nullptr && m_joystick_btn_off->guibutton)
		m_joystick_btn_off->guibutton->setVisible(visible);

	// clear all active buttons
	if (!visible) {
		while (!m_known_ids.empty())
			handleReleaseEvent(m_known_ids.begin()->id);
	}
}

void TouchScreenGUI::hide()
{
	if (!m_visible)
		return;

	Toggle(false);
}

void TouchScreenGUI::show()
{
	if (m_visible)
		return;

	Toggle(true);
}

void TouchScreenGUI::handleReleaseAll()
{
	while (!m_known_ids.empty())
		handleReleaseEvent(m_known_ids.back().id);
	for (auto &button : m_buttons)
		button.ids.clear(); // should do nothing
}
