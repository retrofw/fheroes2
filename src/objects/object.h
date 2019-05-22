/***************************************************************************
 *   Copyright (C) 2009 by Andrey Afletdinov <fheroes2@gmail.com>          *
 *                                                                         *
 *   Part of the Free Heroes2 Engine:                                      *
 *   http://sourceforge.net/projects/fheroes2                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef H2OBJECT_H
#define H2OBJECT_H

#include "mp2.h"
#include "gamedefs.h"

namespace Maps { class TilesAddon; }

class Object
{
    public:
	Object(const MP2::object_t obj = MP2::OBJ_ZERO, const u16 sicn = ICN::UNKNOWN, const u32 uid = 0);
	~Object();

	static bool		isPassable(const std::list<Maps::TilesAddon> & bottoms, const u16 maps_index);
	static bool		AllowDirect(const u8 general, const u16 direct);

	const MP2::object_t	object;
	const u16		icn;
	const u32		id;

    private:
};

#endif
