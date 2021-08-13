--Minetest
--Copyright (C) 2013 sapier
--
--This program is free software; you can redistribute it and/or modify
--it under the terms of the GNU Lesser General Public License as published by
--the Free Software Foundation; either version 3.0 of the License, or
--(at your option) any later version.
--
--This program is distributed in the hope that it will be useful,
--but WITHOUT ANY WARRANTY; without even the implied warranty of
--MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--GNU Lesser General Public License for more details.
--
--You should have received a copy of the GNU Lesser General Public License along
--with this program; if not, write to the Free Software Foundation, Inc.,
--51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

--------------------------------------------------------------------------------

local multicraft_developers = {
	"Maksim Gamarnik (MoNTE48) <MoNTE48@mail.ua>",
	"Bektur Mambetov (ubulem) <berkut87@gmail.com>",
	"Alexander Zavrin (Ransom.00)",
	"luk3yx",
	"Nathan Salapat (NathanS21) <nathan@nathansalapat.com>",
	"Vitaliy Lobachevskiy (numberZero) <numzer0@yandex.ru>",
	"Jean-Patrick Guerrero (kilbith) <jeanpatrick.guerrero@gmail.com>",
	"An0n3m0us",
	"sfan5 <sfan5@live.de>",
	"Stuart Jones (stujones11) <stujones111@gmail.com>",
	"And other people who helped make the world better!"
}

local core_developers = {
	"Perttu Ahola (celeron55) <celeron55@gmail.com>",
	"sfan5 <sfan5@live.de>",
	"ShadowNinja <shadowninja@minetest.net>",
	"Nathanaël Courant (Nore/Ekdohibs) <nore@mesecons.net>",
	"Loic Blot (nerzhul/nrz) <loic.blot@unix-experience.fr>",
	"paramat",
	"Craig Robbins (Zeno) <craig.d.robbins@gmail.com>",
	"Auke Kok (sofar) <sofar@foo-projects.org>",
	"rubenwardy <rw@rubenwardy.com>",
	"Krock/SmallJoker <mk939@ymail.com>"
}

local active_contributors = {
	"red-001 <red-001@outlook.ie> [CSM & Menu fixes]",
	"Dániel Juhász (juhdanad) <juhdanad@gmail.com> [Audiovisuals: lighting]",
	"numberZero [Audiovisuals: meshgen]",
	"Lars Hofhansl <larsh@apache.org> [Occulusion culling, fixes]",
	"Jean-Patrick G (kilbith) <jeanpatrick.guerrero@gmail.com> [Audiovisuals]",
	"Vincent Glize (Dumbeldor) <vincent.glize@live.fr> [CSM]",
	"bigfoot547 [CSM]",
	"Rogier <rogier777@gmail.com> [Fixes]",
	"Wuzzy [Audiovisuals]",
	"Shara/Ezhh [Settings]"
}

local previous_core_developers = {
	"BlockMen",
	"Maciej Kasatkin (RealBadAngel) [RIP]",
	"Lisa Milne (darkrose) <lisa@ltmnet.com>",
	"proller",
	"Ilya Zhuravlev (xyz) <xyz@minetest.net>",
	"PilzAdam <pilzadam@minetest.net>",
	"est31 <MTest31@outlook.com>",
	"kahrl <kahrl@gmx.net>",
	"Ryan Kwolek (kwolekr) <kwolekr@minetest.net>",
	"sapier"
}

local previous_contributors = {
	"Gregory Currie (gregorycu) [optimisation]",
	"Diego Martínez (kaeza) <kaeza@users.sf.net>",
	"T4im [Profiler]",
	"TeTpaAka [Hand overriding, nametag colors]",
	"HybridDog [Fixes]",
	"Duane Robertson <duane@duanerobertson.com> [MGValleys]",
	"neoascetic [OS X Fixes]",
	"TriBlade9 <triblade9@mail.com> [Audiovisuals]",
	"Jurgen Doser (doserj) <jurgen.doser@gmail.com> [Fixes]",
	"MirceaKitsune <mirceakitsune@gmail.com> [Audiovisuals]",
	"Guiseppe Bilotta (Oblomov) <guiseppe.bilotta@gmail.com> [Fixes]",
	"matttpt <matttpt@gmail.com> [Fixes]",
	"Nils Dagsson Moskopp (erlehmann) <nils@dieweltistgarnichtso.net> [Minetest Logo]",
	"Jeija <jeija@mesecons.net> [HTTP, particles]"
}

local function buildCreditList(source)
	local ret = {}
	for i = 1, #source do
		ret[i] = core.formspec_escape(source[i])
	end
	return table.concat(ret, ",,")
end

return {
	name = "credits",
	caption = fgettext("Credits"),
	cbf_formspec = function()
	local version = core.get_version()
		return "label[0.1,-0.1;" ..
			"MultiCraft Open Source, ver. " .. version.string .. "\n" ..
			"Copyright (C) 2014-2021 MultiCraft Development Team\n" ..
			"Licence: LGPLv3.0+ and CC-BY-SA 4.0\n" ..
			"Home page: http://multicraft.world]" ..
			"button[4,1;2,0.5;homepage;Privacy Policy]" ..
			"tablecolumns[color;text]" ..
			"tableoptions[background=#999999;highlight=#00000000;border=true]" ..
			"table[0,1.6;11.8,3.8;list_credits;" ..
			"#FFFF00," .. fgettext("MultiCraft Developers") .. ",," ..
			buildCreditList(multicraft_developers) .. ",,," ..
			"#FFFF00," .. fgettext("Minetest Developers") .. ",," ..
			buildCreditList(core_developers) .. ",,," ..
			"#FFFF00," .. fgettext("Active Contributors") .. ",," ..
			buildCreditList(active_contributors) .. ",,," ..
			"#FFFF00," .. fgettext("Previous Core Developers") ..",," ..
			buildCreditList(previous_core_developers) .. ",,," ..
			"#FFFF00," .. fgettext("Previous Contributors") .. ",," ..
			buildCreditList(previous_contributors) .. "," ..
			";1]"
	end,
	cbf_button_handler = function(this, fields, name, tabdata)
		if fields.homepage then
			core.open_url("http://multicraft.world/privacy")
		end
	end,
}
