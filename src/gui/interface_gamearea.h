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

#ifndef H2INTERFACE_GAMEAREA_H
#define H2INTERFACE_GAMEAREA_H

#include "gamedefs.h"
#include "cursor.h"

enum scroll_t
{
    SCROLL_NONE	  = 0x00,
    SCROLL_LEFT	  = 0x01,
    SCROLL_RIGHT  = 0x02,
    SCROLL_TOP	  = 0x04,
    SCROLL_BOTTOM = 0x08
};

enum level_t
{
    LEVEL_BOTTOM  = 0x01,
    LEVEL_TOP     = 0x02,
    LEVEL_HEROES  = 0x04,
    LEVEL_OBJECTS = 0x08,
    LEVEL_FOG     = 0x20,

    LEVEL_ALL     = 0xFF
};

namespace Maps { class Tiles; }

namespace Interface
{
    class GameArea
    {
    public:

	static GameArea & Get(void);
	void SetAreaSize(s16, s16, u16, u16);
	void Build(void);

	const Rect & GetArea(void) const;
	const Rect & GetRectMaps(void) const;

	Cursor::themes_t GetScrollCursor(void) const;
	bool NeedScroll(void) const;
	void Scroll(void);
	void SetScroll(scroll_t);

	void Center(s16, s16);
	void Center(const Point &pt);
	void Redraw(Surface & dst, u8 = LEVEL_ALL) const;

	void SetUpdateCursor(void);
        void QueueEventProcessing(void);
        
	s16 GetIndexFromMousePoint(const Point & pt) const;

	static void GenerateUltimateArtifactAreaSurface(const u16, Sprite &);
	static void SrcRectFixed(Rect & src, Point & dst, const u16 rw, const u16 rh);

    private:
	GameArea();

	Rect	rectArea;
	Rect	rectMaps;
	u16	oldIndexPos;
	u8	scrollDirection;
	bool    updateCursor;
    };
}

#endif
