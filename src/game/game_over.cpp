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

#include "gamedefs.h"
#include "dialog.h"
#include "world.h"
#include "kingdom.h"
#include "castle.h"
#include "settings.h"
#include "game_over.h"

const char* GameOver::GetString(conditions_t cond)
{
    const char* cond_str[] = { "None", 
	_("Defeat all enemy heroes and towns."), _("Capture a specific town."), _("Defeat a specific hero."), _("Find a specific artifact."), _("Your side defeats the opposing side."), _("Accumulate a large amount of gold."),
	_("Lose all your heroes and towns."), _("Lose a specific town."), _("Lose a specific hero."), _("Run out of time. (Fail to win by a certain point.)") };

    switch(cond)
    {
	case WINS_ALL:		return cond_str[1];
	case WINS_TOWN:		return cond_str[2];
	case WINS_HERO:		return cond_str[3];
	case WINS_ARTIFACT:	return cond_str[4];
        case WINS_SIDE:		return cond_str[5];
        case WINS_GOLD:		return cond_str[6];

	case LOSS_ALL:		return cond_str[7];
	case LOSS_TOWN:		return cond_str[8];
	case LOSS_HERO:		return cond_str[9];
	case LOSS_TIME:		return cond_str[10];

        default: break;
    }

    return cond_str[0];
}

void GameOver::GetActualDescription(u16 cond, std::string & msg)
{
    const Settings & conf = Settings::Get();

    if(WINS_ALL == cond || WINS_SIDE == cond)
	msg = GetString(WINS_ALL);
    else
    if(WINS_TOWN & cond)
    {
	const Castle *town = world.GetCastle(conf.WinsMapsIndexObject());
	if(town)
	{
    	    msg = town->isCastle() ? _("Capture the castle '%{name}'") : _("Capture the town '%{name}'");;
	    String::Replace(msg, "%{name}", town->GetName());
	}
    }
    else
    if(WINS_HERO & cond)
    {
	const Heroes *hero = world.GetHeroesCondWins();
	if(hero)
	{
	    msg = _("Defeat the hero '%{name}'");
	    String::Replace(msg, "%{name}", hero->GetName());
	}
    }
    else
    if(WINS_ARTIFACT & cond)
    {
	if(conf.WinsFindUltimateArtifact())
	    msg = _("Find the ultimate artifact");
	else
	{
	    msg = _("Find the '%{name}' artifact");
	    String::Replace(msg, "%{name}", Artifact::GetName(conf.WinsFindArtifact()));
	}
    }
    else
    if(WINS_GOLD & cond)
    {
	msg = _("Accumulate %{count} gold");
	String::Replace(msg, "%{count}", conf.WinsAccumulateGold());
    }

    if(WINS_ALL != cond && (WINS_ALL & cond)) msg.append(_(", or you may win by defeating all enemy heroes and capturing all enemy towns and castles."));

    if(LOSS_ALL == cond)
	msg = GetString(WINS_ALL);
    else
    if(LOSS_TOWN & cond)
    {
	const Castle *town = world.GetCastle(conf.WinsMapsIndexObject());
	if(town)
	{
    	    msg = town->isCastle() ? _("Lose the castle '%{name}'.") : _("Lose the town '%{name}'.");
	    String::Replace(msg, "%{name}", town->GetName());
	}
    }
    else
    if(LOSS_HERO & cond)
    {
    	const Heroes *hero = world.GetHeroesCondWins();
	if(hero)
	{
	    msg = _("Lose the hero '%{name}'.");
	    String::Replace(msg, "%{name}", hero->GetName());
	}
    }
    else
    if(LOSS_TIME & cond)
    {
	msg = _("Fail to win by the end of month %{month}, week %{week}, day %{day}.");
        String::Replace(msg, "%{day}", (conf.LossCountDays() % DAYOFWEEK));
        String::Replace(msg, "%{week}", (conf.LossCountDays() / DAYOFWEEK) + 1);
        String::Replace(msg, "%{month}", (conf.LossCountDays() / (DAYOFWEEK * WEEKOFMONTH)) + 1);
    }
}

void GameOver::DialogWins(u16 cond)
{
    const Settings & conf = Settings::Get();
    std::string body;

    switch(cond)
    {
	case WINS_ALL:
	    break;

	case WINS_TOWN:
	{
	    body = _("You captured %{name}!\nYou are victorious.");
	    const Castle *town = world.GetCastle(conf.WinsMapsIndexObject());
	    if(town) String::Replace(body, "%{name}", town->GetName());
	}
	break;

	case WINS_HERO:
	{
	    body = _("You have captured the enemy hero %{name}!\nYour quest is complete.");
	    const Heroes *hero = world.GetHeroesCondWins();
	    if(hero) String::Replace(body, "%{name}", hero->GetName());
	    break;
	}

	case WINS_ARTIFACT:
	{
	    body = _("You have found the %{name}.\nYour quest is complete.");
	    const Artifact::artifact_t art = conf.WinsFindUltimateArtifact() ? world.GetUltimateArtifact() : conf.WinsFindArtifact();
	    String::Replace(body, "%{name}", (conf.WinsFindUltimateArtifact() ? "Ultimate Artifact" : Artifact::GetName(art)));
	    break;
        }

        case WINS_SIDE:
    	    body = _("The enemy is beaten.\nYour side has triumphed!");
    	    break;

        case WINS_GOLD:
        {
    	    body = _("You have built up over %{count} gold in your treasury.\nAll enemies bow before your wealth and power.");
	    String::Replace(body, "%{count}", conf.WinsAccumulateGold());
    	    break;
	}

    	default: break;
    }

    if(body.size()) Dialog::Message("", body, Font::BIG, Dialog::OK);
}

void GameOver::DialogLoss(u16 cond)
{
    const Settings & conf = Settings::Get();
    std::string body;

    switch(cond)
    {
	case WINS_ARTIFACT:
	{
	    body = _("The enemy has found the %{name}.\nYour quest is a failure.");
	    const Artifact::artifact_t art = conf.WinsFindArtifact();
	    String::Replace(body, "%{name}", Artifact::GetName(art));
	    break;
        }

        case WINS_SIDE:
        {
    	    body = _("%{color} has fallen!\nAll is lost.");
    	    break;
    	}

        case WINS_GOLD:
        {
    	    body = _("The enemy has built up over %{count} gold in his treasury.\nYou must bow done in defeat before his wealth and power.");
	    String::Replace(body, "%{count}", conf.WinsAccumulateGold());
    	    break;
	}

	case LOSS_ALL:
	    body = _("You have been eliminated from the game!!!");
	    break;

	case LOSS_TOWN:
	{
	    body = _("The enemy has captured %{name}!\nThey are triumphant.");
	    const Castle *town = world.GetCastle(conf.WinsMapsIndexObject());
	    if(town) String::Replace(body, "%{name}", town->GetName());
	}

	case LOSS_HERO:
	{
	    body = _("You have lost the hero %{name}.\nYour quest is over.");
	    const Heroes *hero = world.GetHeroesCondLoss();
	    if(hero) String::Replace(body, "%{name}", hero->GetName());
	    break;
	}

	case LOSS_TIME:
	    body = _("You have failed to complete your quest in time.\nAll is lost.");
	    break;

    	default: break;
    }

    if(body.size()) Dialog::Message("", body, Font::BIG, Dialog::OK);
}

GameOver::Result & GameOver::Result::Get(void)
{
    static Result gresult;
    return gresult;
}

GameOver::Result::Result() : colors(0), result(0)
{
}

void GameOver::Result::Reset(void)
{
    colors = Settings::Get().KingdomColors();
    result = GameOver::COND_NONE;
}

void GameOver::Result::SetResult(u16 r)
{
    result = r;
}

u16  GameOver::Result::GetResult(void) const
{
    return result;
}

namespace Game
{
    void DialogPlayers(const Color::color_t, const std::string &);
}

bool GameOver::Result::CheckGameOver(Game::menu_t & res)
{
    for(Color::color_t c = Color::BLUE; c != Color::GRAY; ++c)
        if(!world.GetKingdom(c).isPlay() && (colors & c))
    {
        std::string message(_("%{color} has been vanquished!"));
        String::Replace(message, "%{color}", Color::String(c));
        Game::DialogPlayers(c, message);
        colors &= (~c);
    }

    const Kingdom & myKingdom = world.GetMyKingdom();

    if(GameOver::COND_NONE != (result = world.CheckKingdomLoss(myKingdom)))
    {
        GameOver::DialogLoss(result);
        res = Game::MAINMENU;
        return true;
    }
    else
    if(GameOver::COND_NONE != (result = world.CheckKingdomWins(myKingdom)))
    {
        GameOver::DialogWins(result);
        res = Game::HIGHSCORES;
        return true;
    }

    return false;
}
