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
#include "agg.h"
#include "dialog.h"
#include "cursor.h"
#include "settings.h"
#include "button.h"
#include "network.h"
#include "pocketpc.h"
#include "game.h"

u8 SelectCountPlayers(void);

Game::menu_t Game::NewStandard(void)
{
    Settings & conf = Settings::Get();
    conf.SetGameType(Game::STANDARD);
    conf.SetPreferablyCountPlayers(0);
    return Game::SELECTSCENARIO;
}

Game::menu_t Game::NewHotSeat(void)
{
    Settings & conf = Settings::Get();
    conf.SetGameType(Game::HOTSEAT);
    conf.SetPreferablyCountPlayers(conf.QVGA() ? 2 : SelectCountPlayers());
    return Game::SELECTSCENARIO;
}

Game::menu_t Game::NewCampain(void)
{
    Settings::Get().SetGameType(Game::CAMPAIGN);
    VERBOSE("New Campain Game: under construction.");
    return Game::NEWGAME;
}

Game::menu_t Game::NewNetwork(void)
{
    // cursor
    Cursor & cursor = Cursor::Get();
    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    Display & display = Display::Get();
    Settings & conf = Settings::Get();

    conf.SetGameType(Game::NETWORK);

    // image background
    const Sprite &back = AGG::GetICN(ICN::HEROES, 0);
    const Point top((display.w() - back.w()) / 2, (display.h() - back.h()) / 2);
    display.Blit(back, top);

    const Sprite &panel = AGG::GetICN(ICN::REDBACK, 0);
    display.Blit(panel, top.x + 405, top.y + 5);

    LocalEvent & le = LocalEvent::Get();

    Button buttonHost(top.x + 455, top.y + 45, ICN::BTNNET, 0, 1);
    Button buttonGuest(top.x + 455, top.y + 110, ICN::BTNNET, 2, 3);
    Button buttonCancelGame(top.x + 455, top.y + 375, ICN::BTNMP, 8, 9);

    buttonHost.Draw();
    buttonGuest.Draw();
    buttonCancelGame.Draw();

    cursor.Show();
    display.Flip();

    // newgame loop
    while(le.HandleEvents())
    {
	le.MousePressLeft(buttonHost) ? buttonHost.PressDraw() : buttonHost.ReleaseDraw();
	le.MousePressLeft(buttonGuest) ? buttonGuest.PressDraw() : buttonGuest.ReleaseDraw();
	le.MousePressLeft(buttonCancelGame) ? buttonCancelGame.PressDraw() : buttonCancelGame.ReleaseDraw();

	if(le.MouseClickLeft(buttonHost) || le.KeyPress(KEY_h)) return NetworkHost();
	if(le.MouseClickLeft(buttonGuest) || le.KeyPress(KEY_g)) return NetworkGuest();
	if(le.MouseClickLeft(buttonCancelGame) || le.KeyPress(KEY_ESCAPE)) return MAINMENU;

        // right info
	if(le.MousePressRight(buttonHost)) Dialog::Message(_("Host"), _("The host sets up the game options. There can only be one host per network game."), Font::BIG);
	if(le.MousePressRight(buttonGuest)) Dialog::Message(_("Guest"), _("The guest waits for the host to set up the game, then is automatically added in. There can be multiple guests for TCP/IP games."), Font::BIG);
	if(le.MousePressRight(buttonCancelGame)) Dialog::Message(_("Cancel"), _("Cancel back to the main menu."), Font::BIG);
    }

    return Game::MAINMENU;
}

Game::menu_t Game::NewGame(void)
{
    Mixer::Pause();
    AGG::PlayMusic(MUS::MAINMENU);
    Settings & conf = Settings::Get();

    Game::IO::last_name.clear();
    conf.SetLoadedGameVersion(false);

    if(Settings::Get().QVGA()) return PocketPC::NewGame();
  
    // preload
    AGG::PreloadObject(ICN::HEROES);
    AGG::PreloadObject(ICN::BTNNEWGM);
    AGG::PreloadObject(ICN::REDBACK);

    // cursor
    Cursor & cursor = Cursor::Get();
    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    Display & display = Display::Get();

    // load game settings
    conf.BinaryLoad();

    // image background
    const Sprite &back = AGG::GetICN(ICN::HEROES, 0);
    const Point top((display.w() - back.w()) / 2, (display.h() - back.h()) / 2);
    display.Blit(back, top);

    const Sprite &panel = AGG::GetICN(ICN::REDBACK, 0);
    display.Blit(panel, top.x + 405, top.y + 5);

    LocalEvent & le = LocalEvent::Get();

    Button buttonStandartGame(top.x + 455, top.y + 45, ICN::BTNNEWGM, 0, 1);
    Button buttonCampainGame(top.x + 455, top.y + 110, ICN::BTNNEWGM, 2, 3);
    Button buttonMultiGame(top.x + 455, top.y + 175, ICN::BTNNEWGM, 4, 5);
    Button buttonCancelGame(top.x + 455, top.y + 375, ICN::BTNNEWGM, 6, 7);
    Button buttonSettings(top.x + 455, top.y + 240, ICN::BTNDCCFG, 4, 5);

    buttonStandartGame.Draw();
    buttonCampainGame.Draw();
    buttonMultiGame.Draw();
    buttonCancelGame.Draw();
    buttonSettings.Draw();

    cursor.Show();
    display.Flip();

    // newgame loop
    while(le.HandleEvents())
    {
	le.MousePressLeft(buttonStandartGame) ? buttonStandartGame.PressDraw() : buttonStandartGame.ReleaseDraw();
	le.MousePressLeft(buttonCampainGame) ? buttonCampainGame.PressDraw() : buttonCampainGame.ReleaseDraw();
	le.MousePressLeft(buttonMultiGame) ? buttonMultiGame.PressDraw() : buttonMultiGame.ReleaseDraw();
	le.MousePressLeft(buttonCancelGame) ? buttonCancelGame.PressDraw() : buttonCancelGame.ReleaseDraw();
	le.MousePressLeft(buttonSettings) ? buttonSettings.PressDraw() : buttonSettings.ReleaseDraw();

	if(le.KeyPress(KEY_s) || le.MouseClickLeft(buttonStandartGame)) return NEWSTANDARD;
	if(le.KeyPress(KEY_c) || le.MouseClickLeft(buttonCampainGame)) return NEWCAMPAIN;
	if(le.KeyPress(KEY_m) || le.MouseClickLeft(buttonMultiGame)) return NEWMULTI;
	if(le.MouseClickLeft(buttonCancelGame) || le.KeyPress(KEY_ESCAPE)) return MAINMENU;
	if(le.MouseClickLeft(buttonSettings)){ Dialog::ExtSettings(); cursor.Show(); display.Flip(); }

        // right info
	if(le.MousePressRight(buttonStandartGame)) Dialog::Message(_("Standard Game"), _("A single player game playing out a single map."), Font::BIG);
	if(le.MousePressRight(buttonCampainGame)) Dialog::Message(_("Campaign Game"), _("A single player game playing through a series of maps."), Font::BIG);
	if(le.MousePressRight(buttonMultiGame)) Dialog::Message(_("Multi-Player Game"), _("A multi-player game, with several human players completing against each other on a single map."), Font::BIG);
	if(le.MousePressRight(buttonSettings)) Dialog::Message(_("Settings"), _("FHeroes2 game settings."), Font::BIG);
	if(le.MousePressRight(buttonCancelGame)) Dialog::Message(_("Cancel"), _("Cancel back to the main menu."), Font::BIG);
    }

    return QUITGAME;
}

Game::menu_t Game::NewMulti(void)
{
    // reset prev. scenario info
    Settings::Get().SetMyColor(Color::GRAY);

    if(Settings::Get().QVGA()) return PocketPC::NewMulti();

    // preload
    AGG::PreloadObject(ICN::HEROES);
    AGG::PreloadObject(ICN::BTNHOTST);
    AGG::PreloadObject(ICN::REDBACK);

    // cursor
    Cursor & cursor = Cursor::Get();
    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    Display & display = Display::Get();

    // image background
    const Sprite &back = AGG::GetICN(ICN::HEROES, 0);
    const Point top((display.w() - back.w()) / 2, (display.h() - back.h()) / 2);
    display.Blit(back, top);

    const Sprite &panel = AGG::GetICN(ICN::REDBACK, 0);
    display.Blit(panel, top.x + 405, top.y + 5);

    LocalEvent & le = LocalEvent::Get();

    Button buttonHotSeat(top.x + 455, top.y + 45, ICN::BTNMP, 0, 1);
    Button buttonNetwork(top.x + 455, top.y + 110, ICN::BTNMP, 2, 3);
    Button buttonCancelGame(top.x + 455, top.y + 375, ICN::BTNMP, 8, 9);

    buttonHotSeat.Draw();
    buttonNetwork.Draw();
    buttonCancelGame.Draw();

    cursor.Show();
    display.Flip();

    // newgame loop
    while(le.HandleEvents())
    {
	le.MousePressLeft(buttonHotSeat) ? buttonHotSeat.PressDraw() : buttonHotSeat.ReleaseDraw();
	le.MousePressLeft(buttonNetwork) ? buttonNetwork.PressDraw() : buttonNetwork.ReleaseDraw();
	le.MousePressLeft(buttonCancelGame) ? buttonCancelGame.PressDraw() : buttonCancelGame.ReleaseDraw();

	if(le.MouseClickLeft(buttonHotSeat) || le.KeyPress(KEY_h)) return NEWHOTSEAT;
	if(le.MouseClickLeft(buttonNetwork) || le.KeyPress(KEY_n)) return NEWNETWORK;
	if(le.MouseClickLeft(buttonCancelGame) || le.KeyPress(KEY_ESCAPE)) return MAINMENU;

        // right info
	if(le.MousePressRight(buttonHotSeat)) Dialog::Message(_("Hot Seat"), _("Play a Hot Seat game, where 2 to 4 players play around the same computer, switching into the 'Hot Seat' when it is their turn."), Font::BIG);
	if(le.MousePressRight(buttonNetwork)) Dialog::Message(_("Network"), _("Play a network game, where 2 players use their own computers connected through a LAN (Local Area Network)."), Font::BIG);
	if(le.MousePressRight(buttonCancelGame)) Dialog::Message(_("Cancel"), _("Cancel back to the main menu."), Font::BIG);
		 
    }

    return QUITGAME;
}

u8 SelectCountPlayers(void)
{
    // cursor
    Cursor & cursor = Cursor::Get();
    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    Display & display = Display::Get();

    // image background
    const Sprite &back = AGG::GetICN(ICN::HEROES, 0);
    const Point top((display.w() - back.w()) / 2, (display.h() - back.h()) / 2);
    display.Blit(back, top);

    const Sprite &panel = AGG::GetICN(ICN::REDBACK, 0);
    display.Blit(panel, top.x + 405, top.y + 5);

    LocalEvent & le = LocalEvent::Get();

    Button button2Players(top.x + 455, top.y + 45, ICN::BTNHOTST, 0, 1);
    Button button3Players(top.x + 455, top.y + 110, ICN::BTNHOTST, 2, 3);
    Button button4Players(top.x + 455, top.y + 175, ICN::BTNHOTST, 4, 5);
    Button button5Players(top.x + 455, top.y + 240, ICN::BTNHOTST, 6, 7);
    Button button6Players(top.x + 455, top.y + 305, ICN::BTNHOTST, 8, 9);
    Button buttonCancel(top.x + 455, top.y + 375, ICN::BTNNEWGM, 6, 7);

    button2Players.Draw();
    button3Players.Draw();
    button4Players.Draw();
    button5Players.Draw();
    button6Players.Draw();
    buttonCancel.Draw();

    cursor.Show();
    display.Flip();

    // newgame loop
    while(le.HandleEvents())
    {
	le.MousePressLeft(button2Players) ? button2Players.PressDraw() : button2Players.ReleaseDraw();
	le.MousePressLeft(button3Players) ? button3Players.PressDraw() : button3Players.ReleaseDraw();
	le.MousePressLeft(button4Players) ? button4Players.PressDraw() : button4Players.ReleaseDraw();
	le.MousePressLeft(button5Players) ? button5Players.PressDraw() : button5Players.ReleaseDraw();
	le.MousePressLeft(button6Players) ? button6Players.PressDraw() : button6Players.ReleaseDraw();

	le.MousePressLeft(buttonCancel) ? buttonCancel.PressDraw() : buttonCancel.ReleaseDraw();

	if(le.MouseClickLeft(button2Players) || le.KeyPress(KEY_2)) return 2;
	if(le.MouseClickLeft(button3Players) || le.KeyPress(KEY_3)) return 3;
	if(le.MouseClickLeft(button4Players) || le.KeyPress(KEY_4)) return 4;
	if(le.MouseClickLeft(button5Players) || le.KeyPress(KEY_5)) return 5;
	if(le.MouseClickLeft(button6Players) || le.KeyPress(KEY_6)) return 6;

	if(le.MouseClickLeft(buttonCancel) || le.KeyPress(KEY_ESCAPE)) return 0;

        // right info
	if(le.MousePressRight(button2Players)) Dialog::Message(_("2 Players"), _("Play with 2 human players, and optionally, up, to 4 additional computer players."), Font::BIG);
	if(le.MousePressRight(button3Players)) Dialog::Message(_("3 Players"), _("Play with 3 human players, and optionally, up, to 3 additional computer players."), Font::BIG);
	if(le.MousePressRight(button4Players)) Dialog::Message(_("4 Players"), _("Play with 4 human players, and optionally, up, to 2 additional computer players."), Font::BIG);
	if(le.MousePressRight(button5Players)) Dialog::Message(_("5 Players"), _("Play with 5 human players, and optionally, up, to 1 additional computer players."), Font::BIG);
	if(le.MousePressRight(button6Players)) Dialog::Message(_("6 Players"), _("Play with 6 human players."), Font::BIG);
	if(le.MousePressRight(buttonCancel)) Dialog::Message(_("Cancel"), _("Cancel back to the main menu."), Font::BIG);
    }

    return 0;
}
