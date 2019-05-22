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

#include <algorithm>
#include "dir.h"
#include "game.h"
#include "dialog.h"
#include "settings.h"
#include "network.h"
#include "remoteclient.h"
#include "localclient.h"
#include "cursor.h"
#include "button.h"
#include "world.h"
#include "agg.h"
#include "zzlib.h"
#include "dialog_selectscenario.h"
#include "server.h"

#ifdef WITH_NET

FH2LocalClient & FH2LocalClient::Get(void)
{
    static FH2LocalClient fh2localclient;

    return fh2localclient;
}

FH2LocalClient::FH2LocalClient()
{
    players.reserve(6);
}

bool FH2LocalClient::Connect(const std::string & srv, u16 port)
{
    server = srv;
    IPaddress ip;
    if(Network::ResolveHost(ip, srv.c_str(), port) && Open(ip))
    {
	Settings::Get().SetNetworkLocalClient(true);
	return true;
    }
    return false;
}

u8 FH2LocalClient::GetPlayersColors(void) const
{
    u8 res = 0;
    std::vector<Player>::const_iterator it1 = players.begin();
    std::vector<Player>::const_iterator it2 = players.end();
    for(; it1 != it2; ++it1) if((*it1).player_id && (*it1).player_color) res |= (*it1).player_color;

    return res;
}

void FH2LocalClient::PopPlayersInfo(QueueMessage & msg)
{
    Player cur;
    u8 size;
    players.clear();
    msg.Pop(size);
    for(u8 ii = 0; ii < size; ++ii)
    {
        msg.Pop(cur.player_color);
        msg.Pop(cur.player_race);
        msg.Pop(cur.player_name);
        msg.Pop(cur.player_id);
        if(cur.player_id) players.push_back(cur);
    }
}

int FH2LocalClient::Main(void)
{
    if(ConnectionChat())
    {
	if(ScenarioInfoDialog())
	{
    	    if(StartGame())
    	    {
        	// may be also
    	    }
	}
    }

    Logout();

    return 1;
}

void FH2LocalClient::Logout(void)
{
    packet.Reset();
    packet.SetID(MSG_LOGOUT);
    packet.Push(player_name);
    Send(packet);
    DELAY(100);
    Close();
    modes = 0;

    if(Modes(ST_LOCALSERVER))
    {
        FH2Server & server = FH2Server::Get();
        if(server.IsRun())
	{
	    server.SetExit();
    	    DELAY(100);
	}
    }
}

bool FH2LocalClient::ConnectionChat(void)
{
    Settings & conf = Settings::Get();

    player_color = 0;
    player_race = 0;
    player_name.clear();
    player_id = 0;

    // recv ready, banner
    DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::ConnectionChat: wait ready");
    if(!Wait(packet, MSG_READY)) return false;

    // get banner
    std::string str;
    packet.Pop(str);

    // dialog: input name
    if(!Dialog::InputString("Connected to " + server + "\n \n" + str + "\n \nEnter player name:", player_name))
	return false;

    // send hello
    packet.Reset();
    packet.SetID(MSG_HELLO);
    packet.Push(player_name);
    DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::ConnectionChat send hello");
    if(!Send(packet)) return false;

    // recv hello, modes, player_id
    DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::ConnectionChat: wait hello");
    if(!Wait(packet, MSG_HELLO)) return false;
    packet.Pop(modes);
    packet.Pop(player_id);
    packet.Pop(player_color);
    if(0 == player_id || 0 == player_color) DEBUG(DBG_NETWORK , DBG_WARN, "FH2LocalClient::ConnectionChat: player zero values");
    DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::ConnectionChat: " << (Modes(ST_ADMIN) ? "admin" : "client") << " mode");
    
    // send get maps info
    packet.Reset();
    packet.SetID(MSG_MAPS_INFO_GET);
    DEBUG(DBG_NETWORK , DBG_INFO,  "FH2LocalClient::ConnectionChat send get_maps_info");
    if(!Send(packet)) return false;

    // recv maps
    DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::ConnectionChat: wait maps_info");
    if(!Wait(packet, MSG_MAPS_INFO)) return false;

    Network::PacketPopMapsFileInfo(packet, conf.CurrentFileInfo());

    // send get players
    packet.Reset();
    packet.SetID(MSG_PLAYERS_GET);
    DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::ConnectionChat send get_players");
    if(!Send(packet)) return false;

    // recv players
    DEBUG(DBG_NETWORK , DBG_INFO,  "FH2LocalClient::ConnectionChat: wait players");
    if(!Wait(packet, MSG_PLAYERS)) return false;
    PopPlayersInfo(packet);

    return true;
}

bool FH2LocalClient::ScenarioInfoDialog(void)
{
    Settings & conf = Settings::Get();
    bool change_players = false;
    bool update_info    = false;

    // draw info dialog
    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();

    const Point pointPanel(204, 32);
    const Point pointOpponentInfo(pointPanel.x + 24, pointPanel.y + 202);
    const Point pointClassInfo(pointPanel.x + 24, pointPanel.y + 282);

    Game::Scenario::RedrawStaticInfo(pointPanel);
    Game::Scenario::RedrawOpponentsInfo(pointOpponentInfo, &players);
    Game::Scenario::RedrawClassInfo(pointClassInfo);

    Button buttonSelectMaps(pointPanel.x + 309, pointPanel.y + 45, ICN::NGEXTRA, 64, 65);
    Button buttonOk(pointPanel.x + 31, pointPanel.y + 380, ICN::NGEXTRA, 66, 67);
    Button buttonCancel(pointPanel.x + 287, pointPanel.y + 380, ICN::NGEXTRA, 68, 69);

    if(! Modes(ST_ADMIN & modes))
    {
	buttonOk.Press();
	buttonOk.SetDisable(true);
	buttonSelectMaps.Press();
	buttonSelectMaps.SetDisable(true);
    }

    buttonSelectMaps.Draw();
    buttonOk.Draw();
    buttonCancel.Draw();
    cursor.Show();
    display.Flip();

    bool exit = false;
    DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::ScenarioInfoDialog: start queue");

    while(!exit && le.HandleEvents())
    {
        if(Ready())
	{
	    if(!Recv(packet)) return false;
	    DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::ScenarioInfoDialog: recv: " << Network::GetMsgString(packet.GetID()));

	    switch(packet.GetID())
    	    {
		case MSG_READY:
		    break;

		case MSG_PLAYERS:
		{
		    PopPlayersInfo(packet);
		    conf.SetPlayersColors(GetPlayersColors());
		    std::vector<Player>::iterator itp = std::find_if(players.begin(), players.end(), std::bind2nd(std::mem_fun_ref(&Player::isID), player_id));
		    if(itp != players.end())
		    {
			player_color = (*itp).player_color;
			player_race = (*itp).player_race;
		    }
		    update_info = true;
		    break;
		}

		case MSG_MAPS_INFO:
		    Network::PacketPopMapsFileInfo(packet, conf.CurrentFileInfo());
		    update_info = true;
		    break;

		case MSG_MESSAGE:
		    break;

		case MSG_SHUTDOWN:
		    exit = true;
		    break;

		case MSG_MAPS_LOAD:
		    cursor.Hide();
        	    display.Fill(0, 0, 0);
            	    TextBox(_("Maps Loading..."), Font::BIG, Rect(0, display.h()/2, display.w(), display.h()/2));
                    display.Flip();

		    if(Game::IO::LoadBIN(packet))
		    {
			conf.SetMyColor(Color::Get(player_color));
			cursor.Hide();
			return true;
		    }
		    else
		    {
			return false;
		    }
		    break;

		default: break;
	    }
	}

	if(update_info)
	{
	    cursor.Hide();
	    Game::Scenario::RedrawStaticInfo(pointPanel);
	    Game::Scenario::RedrawOpponentsInfo(pointOpponentInfo, &players);
	    Game::Scenario::RedrawClassInfo(pointClassInfo);
	    buttonSelectMaps.Draw();
	    buttonOk.Draw();
	    buttonCancel.Draw();
	    cursor.Show();
	    display.Flip();
	    update_info = false;
	}

	if(change_players)
	{
	/*
	    packet.Reset();
	    packet.SetID(MSG_PLAYERS);
	    Network::PacketPushPlayersInfo(packet, players);

	    DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::ScenarioInfoDialog: send players";
	    if(!Send(packet)) return Dialog::CANCEL;

	    conf.SetPlayersColors(Network::GetPlayersColors(players));
	*/
	    change_players = false;
	    update_info = true;
	}

	// press button
        if(buttonSelectMaps.isEnable()) le.MousePressLeft(buttonSelectMaps) ? buttonSelectMaps.PressDraw() : buttonSelectMaps.ReleaseDraw();
        if(buttonOk.isEnable()) le.MousePressLeft(buttonOk) ? buttonOk.PressDraw() : buttonOk.ReleaseDraw();
        le.MousePressLeft(buttonCancel) ? buttonCancel.PressDraw() : buttonCancel.ReleaseDraw();

	// click select
	if(le.KeyPress(KEY_s) || (buttonSelectMaps.isEnable() && le.MouseClickLeft(buttonSelectMaps)))
	{
	    // recv maps_info_list
	    packet.Reset();
	    packet.SetID(MSG_MAPS_LIST_GET);
	    DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::ScenarioInfoDialog: send get_maps_list");
	    if(!Send(packet)) return false;

	    DEBUG(DBG_NETWORK , DBG_INFO,  "FH2LocalClient::ScenarioInfoDialog: wait maps_list");
	    if(!Wait(packet, MSG_MAPS_LIST)) return false;

	    MapsFileInfoList lists;
	    Network::PacketPopMapsFileInfoList(packet, lists);

            std::string filemaps;
            if(Dialog::SelectScenario(lists, filemaps) && filemaps.size())
            {
		// send set_maps_info
	        packet.Reset();
		packet.SetID(MSG_MAPS_INFO_SET);
	        packet.Push(filemaps);
		DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::ScenarioInfoDialog: send maps_info");
	        if(!Send(packet)) return false;
	    }
	    update_info = true;
	}
	else
	// click cancel
        if(le.MouseClickLeft(buttonCancel) || le.KeyPress(KEY_ESCAPE))
    	    return false;
        else
        // click ok
        if(le.KeyPress(KEY_RETURN) || (buttonOk.isEnable() && le.MouseClickLeft(buttonOk)))
    	{
           packet.Reset();
           packet.SetID(MSG_MAPS_LOAD);
           DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::ScenarioInfoDialog: send maps_load");
           if(!Send(packet)) return false;
	}

        DELAY(10);
    }

    return false;
}

Game::menu_t Game::NetworkGuest(void)
{
    Settings & conf = Settings::Get();
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    
    // clear background
    const Sprite &back = AGG::GetICN(ICN::HEROES, 0);
    cursor.Hide();
    display.Blit(back);
    cursor.Show();
    display.Flip();

    std::string server;
    if(!Dialog::InputString("Server Name", server)) return MAINMENU;

    FH2LocalClient & client = FH2LocalClient::Get();

    if(! client.Connect(server, conf.GetPort()))
    {
        Dialog::Message(_("Error"), Network::GetError(), Font::BIG, Dialog::OK);
	return Game::MAINMENU;
    }
    conf.SetGameType(Game::NETWORK);

    // main procedure
    client.Main();

    return QUITGAME;
}

#else
Game::menu_t Game::NetworkGuest(void)
{
    Dialog::Message(_("Error"), _("This release is compiled without network support."), Font::BIG, Dialog::OK);
    return MAINMENU;
}
#endif
