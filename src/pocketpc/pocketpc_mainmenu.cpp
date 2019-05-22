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

#include "agg.h"
#include "cursor.h"
#include "settings.h"
#include "text.h"
#include "pocketpc.h"

Game::menu_t PocketPC::LoadGame(void)
{
    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();
    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    const Sprite &sprite = AGG::GetICN(ICN::HEROES, 0);
    Rect src_rt((sprite.w() - display.w()) / 2, 0, display.w(), display.h());
    display.Blit(sprite, src_rt, 0, 0);

    cursor.Show();
    display.Flip();

    std::string file;
    if(!Dialog::SelectFileLoad(file) || file.empty() || !Game::Load(file)) return Game::MAINMENU;

    return Game::STARTGAME;
}

Game::menu_t PocketPC::MainMenu(void)
{
    Game::SetFixVideoMode();

    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();

    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    const Sprite &sprite = AGG::GetICN(ICN::HEROES, 0);
    Rect src_rt((sprite.w() - display.w()) / 2, 0, display.w(), display.h());
    display.Blit(sprite, src_rt, 0, 0);

    const Sprite &board = AGG::GetICN(ICN::QWIKTOWN, 0);
    src_rt = Rect(13, 0, board.w() - 13, board.h() - 13);
    Point dst_pt((display.w() - src_rt.w) / 2, (display.h() - src_rt.h) / 2);
    display.Blit(board, src_rt, dst_pt.x , dst_pt.y);

    Text text;
    
    text.Set("Free Heroes II", Font::YELLOW_BIG);
    text.Blit(dst_pt.x + (src_rt.w - text.w()) / 2, dst_pt.y + 12);

    text.Set(_("New Game"), Font::BIG);
    const Rect rectNewGame(dst_pt.x + (src_rt.w - text.w()) / 2 - 5, dst_pt.y + 35, text.w() + 10, text.h() + 10);
    text.Blit(rectNewGame);

    text.Set(_("Load Game"));
    const Rect rectLoadGame(dst_pt.x + (src_rt.w - text.w()) / 2 - 5, dst_pt.y + 55, text.w() + 10, text.h() + 10);
    text.Blit(rectLoadGame);

    text.Set(_("Settings"));
    const Rect rectSettings(dst_pt.x + (src_rt.w - text.w()) / 2 - 5, dst_pt.y + 75, text.w() + 10, text.h() + 10);
    text.Blit(rectSettings);

    text.Set(_("High Scores"));
    const Rect rectHighScores(dst_pt.x + (src_rt.w - text.w()) / 2 - 5, dst_pt.y + 95, text.w() + 10, text.h() + 10);
    text.Blit(rectHighScores);

    text.Set(_("Credits"));
    const Rect rectCredits(dst_pt.x + (src_rt.w - text.w()) / 2 - 5, dst_pt.y + 115, text.w() + 10, text.h() + 10);
    text.Blit(rectCredits);

    text.Set(_("Quit"));
    const Rect rectQuitGame(dst_pt.x + (src_rt.w - text.w()) / 2 - 5, dst_pt.y + 135, text.w() + 10, text.h() + 10);
    text.Blit(rectQuitGame);

    cursor.Show();
    display.Flip();

    std::string test;

    // mainmenu loop
    while(le.HandleEvents())
    {
	if(le.KeyPress(KEY_n) || le.MouseClickLeft(rectNewGame)) return Game::NEWGAME;
	else
	if(le.KeyPress(KEY_l) || le.MouseClickLeft(rectLoadGame)) return Game::LOADGAME;
	else
	if(le.KeyPress(KEY_s) || le.MouseClickLeft(rectSettings)){ Dialog::ExtSettings(); cursor.Show(); display.Flip(); }
	else
	if(le.KeyPress(KEY_c) || le.MouseClickLeft(rectCredits)) return Game::CREDITS;
	else
	if(le.KeyPress(KEY_h) || le.MouseClickLeft(rectHighScores)) return Game::HIGHSCORES;
	else
	if(le.KeyPress(KEY_q) || le.MouseClickLeft(rectQuitGame) || le.KeyPress(KEY_ESCAPE)) return Game::QUITGAME;
    }

    return Game::QUITGAME;
}

Game::menu_t PocketPC::NewGame(void)
{
    Game::SetFixVideoMode();

    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();
    Settings & conf = Settings::Get();
    LocalEvent & le = LocalEvent::Get();

    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    const Sprite &sprite = AGG::GetICN(ICN::HEROES, 0);
    Rect src_rt((sprite.w() - display.w()) / 2, 0, display.w(), display.h());
    display.Blit(sprite, src_rt, 0, 0);

    const Sprite &board = AGG::GetICN(ICN::QWIKTOWN, 0);
    src_rt = Rect(13, 0, board.w() - 13, board.h() - 13);
    Point dst_pt((display.w() - src_rt.w) / 2, (display.h() - src_rt.h) / 2);
    display.Blit(board, src_rt, dst_pt.x , dst_pt.y);

    Text text;
    
    text.Set("Free Heroes II", Font::YELLOW_BIG);
    text.Blit(dst_pt.x + (src_rt.w - text.w()) / 2, dst_pt.y + 12);

    text.Set(conf.BuildVersion(), Font::YELLOW_SMALL);
    text.Blit(dst_pt.x + (src_rt.w - text.w()) / 2, dst_pt.y + 148);

    text.Set(_("Standard Game"), Font::BIG);
    const Rect rectStandardGame(dst_pt.x + (src_rt.w - text.w()) / 2 - 5, dst_pt.y + 40 + 5, text.w() + 10, text.h() + 10);
    text.Blit(rectStandardGame);

    text.Set(_("Campaign Game"));
    const Rect rectCampaignGame(dst_pt.x + (src_rt.w - text.w()) / 2 - 5, dst_pt.y + 65 + 5, text.w() + 10, text.h() + 10);
    text.Blit(rectCampaignGame);

    text.Set(_("Multi-Player Game"));
    const Rect rectMultiGame(dst_pt.x + (src_rt.w - text.w()) / 2 - 5, dst_pt.y + 90 + 5, text.w() + 10, text.h() + 10);
    text.Blit(rectMultiGame);

    text.Set(_("Cancel"));
    const Rect rectCancel(dst_pt.x + (src_rt.w - text.w()) / 2 - 5, dst_pt.y + 115 + 5, text.w() + 10, text.h() + 10);
    text.Blit(rectCancel);

    cursor.Show();
    display.Flip();

    // mainmenu loop
    while(le.HandleEvents())
    {
	if(le.KeyPress(KEY_s) || le.MouseClickLeft(rectStandardGame)) return Game::NEWSTANDARD;
	else
	if(le.KeyPress(KEY_c) || le.MouseClickLeft(rectCampaignGame)) return Game::MAINMENU;
	else
	if(le.KeyPress(KEY_m) || le.MouseClickLeft(rectMultiGame)) return Game::NEWMULTI;
	else
	if(le.MouseClickLeft(rectCancel) || le.KeyPress(KEY_ESCAPE)) return Game::MAINMENU;
    }

    return Game::QUITGAME;
}

Game::menu_t PocketPC::NewMulti(void)
{
    Game::SetFixVideoMode();

    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();
    Settings & conf = Settings::Get();
    LocalEvent & le = LocalEvent::Get();

    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    const Sprite &sprite = AGG::GetICN(ICN::HEROES, 0);
    Rect src_rt((sprite.w() - display.w()) / 2, 0, display.w(), display.h());
    display.Blit(sprite, src_rt, 0, 0);

    const Sprite &board = AGG::GetICN(ICN::QWIKTOWN, 0);
    src_rt = Rect(13, 0, board.w() - 13, board.h() - 13);
    Point dst_pt((display.w() - src_rt.w) / 2, (display.h() - src_rt.h) / 2);
    display.Blit(board, src_rt, dst_pt.x , dst_pt.y);

    Text text;
    
    text.Set("Free Heroes II", Font::YELLOW_BIG);
    text.Blit(dst_pt.x + (src_rt.w - text.w()) / 2, dst_pt.y + 12);

    text.Set(conf.BuildVersion(), Font::YELLOW_SMALL);
    text.Blit(dst_pt.x + (src_rt.w - text.w()) / 2, dst_pt.y + 148);

    text.Set(_("Hot Seat"), Font::BIG);
    const Rect rectHotSeat(dst_pt.x + (src_rt.w - text.w()) / 2 - 5, dst_pt.y + 40 + 5, text.w() + 10, text.h() + 10);
    text.Blit(rectHotSeat);

    text.Set(_("Network"));
    const Rect rectNetwork(dst_pt.x + (src_rt.w - text.w()) / 2 - 5, dst_pt.y + 65 + 5, text.w() + 10, text.h() + 10);
    text.Blit(rectNetwork);

    text.Set(_("Cancel"));
    const Rect rectCancel(dst_pt.x + (src_rt.w - text.w()) / 2 - 5, dst_pt.y + 115 + 5, text.w() + 10, text.h() + 10);
    text.Blit(rectCancel);

    cursor.Show();
    display.Flip();

    // mainmenu loop
    while(le.HandleEvents())
    {
	if(le.KeyPress(KEY_h) || le.MouseClickLeft(rectHotSeat)) return Game::NEWHOTSEAT;
	else
	if(le.KeyPress(KEY_n) || le.MouseClickLeft(rectNetwork))
	{
	    Dialog::Message(_("Error"), _("This release is compiled without network support."), Font::BIG, Dialog::OK);
	    return Game::MAINMENU;
	}
	else
	if(le.MouseClickLeft(rectCancel) || le.KeyPress(KEY_ESCAPE)) return Game::MAINMENU;
    }

    return Game::QUITGAME;
}
