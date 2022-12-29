/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// menu.h

#ifndef __MENU_H__
#define __MENU_H__

extern	char	demodir[MAX_QPATH];

enum {
	m_none,
	m_main,
	m_singleplayer,
	m_load,
	m_save,
	m_multiplayer,
	m_setup,
	m_net,
	m_options,
	m_videomodes,
#ifdef SUPPORTS_QMB
	m_videoeffects,
	m_particles,
#endif
	m_keys,
#ifdef SUPPORTS_NEHAHRA
	m_nehdemos,
#endif
#ifdef SUPPORTS_EXTENDED_MENUS
	m_maps,
	m_demos,
#endif
	m_help,
	m_quit,
	m_serialconfig,
	m_modemconfig,
	m_lanconfig,
	m_gameoptions,
	m_search,
	m_slist,
	m_preferences,
	m_namemaker,
//#ifdef GAMEATRON
//	m_gameatron
//#endif
} m_state;

// menus
void M_Init (void);
void M_Keydown (int key, int ascii, qbool down);
void M_Draw (void);
void M_ToggleMenu_f (void);
void M_Menu_Main_f (void);
void M_Menu_Quit_f (void);

#endif // __MENU_H__