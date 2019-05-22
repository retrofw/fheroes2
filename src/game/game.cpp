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

#include <map>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <algorithm>

#include "gamedefs.h"
#include "settings.h"
#include "game_focus.h"
#include "maps_tiles.h"
#include "ground.h"
#include "world.h"
#include "kingdom.h"
#include "castle.h"
#include "mp2.h"
#include "agg.h"
#include "test.h"
#include "game.h"

#ifdef WITH_XML
#include "xmlccwrap.h"
#endif

namespace Game
{
    u8 GetMixerChannelFromObject(const Maps::Tiles &);

    static u8 lost_town_days(7);
    static u16 reserved_vols[LOOPXX_COUNT];

    // town, castle, heroes, artifact_telescope, object_observation_tower, object_magi_eyes
    static u8 view_distance[] = { 4, 5, 4, 1, 10, 9 };
    static u8 whirlpool_percent = 50;
    static u8 heroes_restore_spell_points_day = 1;
}

Game::menu_t Game::Testing(u8 t)
{
#ifndef BUILD_RELEASE
    Test::Run(t);
    return Game::QUITGAME;
#else
    return Game::MAINMENU;
#endif
}

Game::menu_t Game::Credits(void)
{
    const Settings & conf = Settings::Get();

    std::string str = conf.BuildVersion();

    str.append("\n \n");
    str.append("Site project: https://sf.net/projects/fheroes2");

    Dialog::Message("Free Heroes II Engine", str, Font::SMALL, Dialog::OK);

    //VERBOSE("Credits: under construction.");

    return Game::MAINMENU;
}

void Game::SetFixVideoMode(void)
{
    const Settings & conf = Settings::Get();

    Size fixsize(conf.VideoMode());
    u32 max = 0;

    switch(conf.MapsWidth())
    {
	default:
	case Maps::SMALL:
	    max = (6 + 36) * TILEWIDTH;
	    break;

	case Maps::MEDIUM:
	    max = (6 + 72) * TILEWIDTH;
	    break;

	case Maps::LARGE:
	    max = (6 + 108) * TILEWIDTH;
	    break;

	case Maps::XLARGE:
	    max = (6 + 144) * TILEWIDTH;
	    break;
    }

    if(conf.VideoMode().w > max) fixsize.w = max;
    if(conf.VideoMode().h > max) fixsize.h = max;

//	fixsize.w = 320;
//	fixsize.h = 240;

    Display::Get().SetVideoMode(fixsize, conf.FullScreen());
}

/* play all sound from focus area game */
void Game::EnvironmentSoundMixer(void)
{
    const Focus & focus = Focus::Get();
    const Point & abs_pt = focus.Center();
    const Settings & conf = Settings::Get();

    if(conf.Sound())
    {
	std::fill(reserved_vols, reserved_vols + LOOPXX_COUNT, 0);

        // scan 4x4 square from focus
        for(s16 yy = abs_pt.y - 3; yy <= abs_pt.y + 3; ++yy)
    	{
    	    for(s16 xx = abs_pt.x - 3; xx <= abs_pt.x + 3; ++xx)
	    {
		if(Maps::isValidAbsPoint(xx, yy))
		{
		    const u8 channel = GetMixerChannelFromObject(world.GetTiles(xx, yy));
    		    if(channel < LOOPXX_COUNT)
		    {
			// calculation volume
    			const u8 length = std::max(std::abs(xx - abs_pt.x), std::abs(yy - abs_pt.y));
			const u16 volume = (2 < length ? 4 : (1 < length ? 8 : (0 < length ? 12 : 16))) * Mixer::MaxVolume() / 16;

			if(volume > reserved_vols[channel]) reserved_vols[channel] = volume;
		    }
		}
	    }
	}

	AGG::Cache::Get().LoadLOOPXXSounds(reserved_vols);
    }
}

u8 Game::GetMixerChannelFromObject(const Maps::Tiles & tile)
{
    // force: check stream
    if(tile.isStream()) return 13;

    MP2::object_t object = tile.GetObject();
    if(MP2::OBJ_HEROES == object)
    {
	const Heroes* hero = world.GetHeroes(tile.GetIndex());
	object = hero->GetUnderObject();
    }

    return M82::GetIndexLOOP00XXFromObject(object);
}

Game::control_t Game::GetControl(u8 index)
{
    switch(index)
    {
	case LOCAL:	return LOCAL;
	case REMOTE:	return REMOTE;
	case AI:	return AI;
	default: break;
    }
    return NONE;
}

Game::type_t Game::GetType(u8 index)
{
    switch(index)
    {
        case STANDARD:  return STANDARD;
        case CAMPAIGN:  return CAMPAIGN;
        case HOTSEAT:   return HOTSEAT;
        case NETWORK:   return NETWORK;
	default: break;
    }
    return UNKNOWN;
}

u8 Game::GetRating(void)
{
    Settings & conf = Settings::Get();
    u8 rating = 50;

    switch(conf.MapsDifficulty())
    {
        case Difficulty::NORMAL:     rating += 20; break;
        case Difficulty::HARD:       rating += 40; break;
        case Difficulty::EXPERT:
        case Difficulty::IMPOSSIBLE:	rating += 80; break;
	default: break;
    }

    switch(conf.GameDifficulty())
    {
        case Difficulty::NORMAL:     rating += 30; break;
        case Difficulty::HARD:       rating += 50; break;
        case Difficulty::EXPERT:	rating += 70; break;
        case Difficulty::IMPOSSIBLE:	rating += 90; break;
	default: break;
    }

    return rating;
}

u16 Game::GetGameOverScores(void)
{
    Settings & conf = Settings::Get();

    u8 k_size = 0;

    switch(conf.MapsWidth())
    {
	case Maps::SMALL:	k_size = 140; break;
	case Maps::MEDIUM:	k_size = 100; break;
	case Maps::LARGE:	k_size =  80; break;
	case Maps::XLARGE:	k_size =  60; break;
	default: break;
    }

    u8 flag = 0;
    u8 nk = 0;
    u16 end_days = world.CountDay();

    for(u16 ii = 1; ii <= end_days; ++ii)
    {
	nk = ii * k_size / 100;

	if(0 == flag && nk > 60){ end_days = ii + (world.CountDay() - ii) / 2; flag = 1; }
	else
	if(1 == flag && nk > 120) end_days = ii + (world.CountDay() - ii) / 2;
	else
	if(nk > 180) break;
    }

    return GetRating() * (200 - nk) / 100;
}

void Game::KeyboardGlobalFilter(u32 sym, u16 mod)
{
    Display & display = Display::Get();

    switch(sym)
    {
	case SDLK_F4:
    	    display.FullScreen();
            break;

	case SDLK_PRINT:
        {
            std::ostringstream stream;
            stream << Settings::Get().LocalPrefix() << SEPARATOR << "files" << SEPARATOR << "save" << SEPARATOR << "screenshot_" << std::time(0);
#ifndef WITH_IMAGE
                stream << ".bmp";
#else
                stream << ".png";
#endif
	    if(display.Save(stream.str().c_str())) DEBUG(DBG_GAME , DBG_INFO, "Game::KeyboardGlobalFilter: save: " << stream.str());
        }
	    break;

	default:
	    break;
    }
}

void Game::ShowLoadMapsText(void)
{
    Display & display = Display::Get();
    display.Fill(0, 0, 0);
    TextBox(_("Maps Loading..."), Font::BIG, Rect(0, display.h()/2, display.w(), display.h()/2));
    display.Flip();
}

u8 Game::GetLostTownDays(void)
{
    return lost_town_days;
}

u8 Game::GetViewDistance(distance_t d)
{
    return view_distance[d];
}

void Game::UpdateGlobalDefines(const std::string & spec)
{
#ifdef WITH_XML
    // parse profits.xml
    TiXmlDocument doc;
    const TiXmlElement* xml_globals = NULL;

    if(doc.LoadFile(spec.c_str()) &&
	NULL != (xml_globals = doc.FirstChildElement("globals")))
    {
	const TiXmlElement* xml_element;

	// starting_resource
	xml_element = xml_globals->FirstChildElement("starting_resource");
	if(xml_element)
	    Kingdom::UpdateStartingResource(xml_element);

	// view_distance
	xml_element = xml_globals->FirstChildElement("view_distance");
	if(xml_element)
	{
	    int value;
	    xml_element->Attribute("town", &value);
	    if(value) view_distance[0] = value;

	    xml_element->Attribute("castle", &value);
	    if(value) view_distance[1] = value;

	    xml_element->Attribute("heroes", &value);
	    if(value) view_distance[2] = value;

	    xml_element->Attribute("artifact_telescope", &value);
	    if(value) view_distance[3] = value;

	    xml_element->Attribute("object_observation_tower", &value);
	    if(value) view_distance[4] = value;

	    xml_element->Attribute("object_magi_eyes", &value);
	    if(value) view_distance[5] = value;
	}

	// kingdom
	xml_element = xml_globals->FirstChildElement("kingdom");
	if(xml_element)
	{
	    int value;
	    xml_element->Attribute("max_heroes", &value);
	    Kingdom::SetMaxHeroes(value);
	}


	// game_over
	xml_element = xml_globals->FirstChildElement("game_over");
	if(xml_element)
	{
	    int value;
	    xml_element->Attribute("lost_towns_days", &value);
	    lost_town_days = value;
	}

	// whirlpool
	xml_element = xml_globals->FirstChildElement("whirlpool");
	if(xml_element)
	{
	    int value;
	    xml_element->Attribute("percent", &value);
	    if(value && value < 90) whirlpool_percent = value;
	}

	// heroes
	xml_element = xml_globals->FirstChildElement("heroes");
	if(xml_element)
	{
	    int value;
	    xml_element->Attribute("spell_points_per_day", &value);
	    if(value < 11) heroes_restore_spell_points_day = value;
	}

	// castle_extra_growth 
	xml_element = xml_globals->FirstChildElement("castle_extra_growth ");
	if(xml_element) Castle::UpdateExtraGrowth(xml_element);
    }
    else
    VERBOSE(spec << ": " << doc.ErrorDesc());
#endif
}

u8 Game::GetWhirlpoolPercent(void)
{
    return whirlpool_percent;
}

u8 Game::GetHeroRestoreSpellPointsPerDay(void)
{
    return heroes_restore_spell_points_day;
}
