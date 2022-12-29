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
// menu.c
#include "quakedef.h"


#ifdef _WIN32 // Headers
#include "winquake.h"
#endif


#define MYPICT mpic_t
qbool vid_windowedmouse = true;
#if SUPPORTS_GLVIDEO_MODESWITCH
void (*vid_menucmdfn)(void); //johnfitz
#endif
void (*vid_menudrawfn)(void);
void (*vid_menukeyfn)(int key);

/*
// Settings
#define NumberOfNehahraDemos 34
typedef struct menux_s
{
	int  row;
	int  col;
	char *caption;
	int  option_type;
} menux_t;
static menux_t NehahraDemos[NumberOfNehahraDemos] =
{
	{"intro", "Prologue"},
	{"genf", "The Beginning"},
	{"genlab", "A Doomed Project"},
	{"nehcre", "The New Recruits"},
	{"maxneh", "Breakthrough"},
	{"maxchar", "Renewal and Duty"},
	{"crisis", "Worlds Collide"},
	{"postcris", "Darkening Skies"},
	{"hearing", "The Hearing"},
	{"getjack", "On a Mexican Radio"},
	{"prelude", "Honor and Justice"},
	{"abase", "A Message Sent"},
	{"effect", "The Other Side"},
	{"uhoh", "Missing in Action"},
	{"prepare", "The Response"},
	{"vision", "Farsighted Eyes"},
	{"maxturns", "Enter the Immortal"},
	{"backlot", "Separate Ways"},
	{"maxside", "The Ancient Runes"},
	{"counter", "The New Initiative"},
	{"warprep", "Ghosts to the World"},
	{"counter1", "A Fate Worse Than Death"},
	{"counter2", "Friendly Fire"},
	{"counter3", "Minor Setback"},
	{"madmax", "Scores to Settle"},
	{"quake", "One Man"},
	{"cthmm", "Shattered Masks"},
	{"shades", "Deal with the Dead"},
	{"gophil", "An Unlikely Hero"},
	{"cstrike", "War in Hell"},
	{"shubset", "The Conspiracy"},
	{"shubdie", "Even Death May Die"},
	{"newranks", "An Empty Throne"},
	{"seal", "The Seal is Broken"}
};*/
qbool menu_fastexit=false;   // Support for instant exit from menu used by namemaker and gametron, instead of having to go 3 layers out
extern int key_special_dest;	// Used to receive mouse screen coordinates
void M_Menu_Main_f (void);
	void M_Menu_SinglePlayer_f (void);
		void M_Menu_Load_f (void);
		void M_Menu_Save_f (void);
	void M_Menu_MultiPlayer_f (void);
		void M_Menu_Setup_f (void);
void M_Menu_NameMaker_f (void);//JQ1.5dev
		void M_Menu_Net_f (void);
	void M_Menu_Options_f (void);
		void M_Menu_Keys_f (void);
		void M_Menu_Preferences_f (void);
		void M_Menu_VideoModes_f (void);
#ifdef SUPPORTS_QMB
		void M_Menu_VideoEffects_f (void);
			void M_Menu_Particles_f (void);
#endif
#ifdef SUPPORTS_NEHAHRA
	void M_Menu_NehDemos_f (void);
#endif
#ifdef SUPPORTS_EXTENDED_MENUS
	void M_Menu_Maps_f (void);
	void M_Menu_Demos_f (void);
#endif
	void M_Menu_Quit_f (void);
void M_Menu_SerialConfig_f (void);
	void M_Menu_ModemConfig_f (void);
void M_Menu_LanConfig_f (void);
void M_Menu_GameOptions_f (void);
void M_Menu_Search_f (void);
void M_Menu_ServerList_f (void);
void M_Main_Draw (void);
	void M_SinglePlayer_Draw (void);
		void M_Load_Draw (void);
		void M_Save_Draw (void);
	void M_MultiPlayer_Draw (void);
		void M_Setup_Draw (void);
void M_NameMaker_Draw (void);//JQ1.5Dev
		void M_Net_Draw (void);
	void M_Options_Draw (void);
		void M_Keys_Draw (void);
		void M_VideoModes_Draw (void);
#ifdef SUPPORTS_QMB
		void M_VideoEffects_Draw (void);
			void M_Particles_Draw (void);
#endif
#ifdef SUPPORTS_NEHAHRA
	void M_NehDemos_Draw (void);
#endif
#ifdef SUPPORTS_EXTENDED_MENUS
	void M_Maps_Draw (void);
	void M_Demos_Draw (void);
#endif
	void M_Quit_Draw (void);
void M_SerialConfig_Draw (void);
	void M_ModemConfig_Draw (void);
void M_LanConfig_Draw (void);
void M_GameOptions_Draw (void);
void M_Search_Draw (void);
void M_ServerList_Draw (void);
void M_Main_Key (int key, int ascii);
	void M_SinglePlayer_Key (int key, int ascii);
		void M_Load_Key (int key, int ascii);
		void M_Save_Key (int key, int ascii);
	void M_MultiPlayer_Key (int key, int ascii);
		void M_Setup_Key (int key, int ascii);
		void M_Net_Key (int key, int ascii);
	void M_Options_Key (int key, int ascii);
		void M_Keys_Key (int key, int ascii, qbool down);
		void M_VideoModes_Key (int key, int ascii);
#ifdef SUPPORTS_QMB
		void M_VideoEffects_Key (int key, int ascii);
			void M_Particles_Key (int key, int ascii);
#endif
#ifdef SUPPORTS_NEHAHRA
	void M_NehDemos_Key (int key, int ascii);
#endif
#ifdef SUPPORTS_EXTENDED_MENUS
	void M_Maps_Key (int key, int ascii);
	void M_Demos_Key (int key, int ascii);
#endif
	void M_Quit_Key (int key, int ascii);
void M_SerialConfig_Key (int key, int ascii);
	void M_ModemConfig_Key (int key, int ascii);
	void M_NameMaker_Key (int key, int ascii);
void M_LanConfig_Key (int key, int ascii);
void M_GameOptions_Key (int key, int ascii);
void M_Search_Key (int key, int ascii);
void M_ServerList_Key (int key, int ascii);
qbool	m_entersound;		// play after drawing a frame, so caching
					// won't disrupt the sound
qbool	m_recursiveDraw;
int			m_return_state;
qbool	m_return_onerror;
char		m_return_reason[32];
#define StartingGame	(m_multiplayer_cursor == 1)
#define JoiningGame	(m_multiplayer_cursor == 0)
#define SerialConfig	(m_net_cursor == 0)
#define DirectConfig	(m_net_cursor == 1)
#define	IPXConfig	(m_net_cursor == 2)
#define	TCPIPConfig	(m_net_cursor == 3)
void M_ConfigureNetSubsystem(void);
#ifdef SUPPORTS_CENTERED_MENUS
int	menuwidth = 320;
int	menuheight = 240;
#else
#define	menuwidth	vid.width
#define	menuheight	vid.height
#endif
int	m_yofs = 0;
/*
================
M_DrawCharacter
Draws one solid graphics character
================
*/
void M_DrawCharacter (int cx, int line, int num)
{
	Draw_Character (cx + ((menuwidth - 320) >> 1), line + m_yofs, num);
}
void M_Print (int cx, int cy, char *str)
{
	Draw_Alt_String (cx + ((menuwidth - 320) >> 1), cy + m_yofs, str);
}
void M_PrintWhite (int cx, int cy, char *str)
{
	Draw_String (cx + ((menuwidth - 320) >> 1), cy + m_yofs, str);
}
void M_DrawTransPic (int x, int y, MYPICT *pic)
{
	Draw_TransPic (x + ((menuwidth - 320) >> 1), y + m_yofs, pic);
}
void M_DrawPic (int x, int y, MYPICT *pic)
{
	Draw_Pic (x + ((menuwidth - 320) >> 1), y + m_yofs, pic);
}
byte	identityTable[256];
byte	translationTable[256];
void M_BuildTranslationTable (int top, int bottom)
{
	int	j;
	byte	*dest, *source;
	for (j=0 ; j<256 ; j++)
		identityTable[j] = j;
	dest = translationTable;
	source = identityTable;
	memcpy (dest, source, 256);
	if (top < 128)	// the artists made some backwards ranges. sigh.
		memcpy (dest + TOP_RANGE, source + top, 16);
	else
		for (j=0 ; j<16 ; j++)
			dest[TOP_RANGE+j] = source[top+15-j];
	if (bottom < 128) // the artists made some backwards ranges. sigh.
		memcpy (dest + BOTTOM_RANGE, source + bottom, 16);
	else
		for (j=0 ; j<16 ; j++)
			dest[BOTTOM_RANGE+j] = source[bottom+15-j];
}

void M_DrawTransPicTranslate (int x, int y, MYPICT *pic)
{
	Draw_TransPicTranslate (x + ((menuwidth - 320) >> 1), y + m_yofs, pic, translationTable);
}
void M_DrawTextBox (int x, int y, int width, int lines)
{
	Draw_TextBox (x + ((menuwidth - 320) >> 1), y + m_yofs, width, lines);
}
void M_DrawTextBox2 (int x, int y, int width, int lines)
{
	Draw_Fill (x + ((menuwidth - 320) >> 1), y + m_yofs, width * 8, lines * 8, 0);
	Draw_TextBox (x + ((menuwidth - 320) >> 1), y + m_yofs, width, lines);
}

//=============================================================================
//int	m_save_demonum;
/*
================
M_ToggleMenu_f
================
*/
void M_ToggleMenu_f (void)
{
	m_entersound = true;
	if (key_dest == key_menu)
	{
		if (m_state != m_main)
		{
			M_Menu_Main_f ();
			return;
		}
		key_dest = key_game;
		m_state = m_none;
		return;
	}
	if (key_dest == key_console)
	{
		Con_ToggleConsole_f ();
	}
	else
	{
		M_Menu_Main_f ();
	}
}

//=============================================================================
/* MAIN MENU */
int	m_main_cursor;
int	MAIN_ITEMS = 6;
void M_Menu_Main_f (void)
{
#ifdef SUPPORTS_NEHAHRA
	if (nehahra)
	{
		if (NehGameType == TYPE_DEMO)
			MAIN_ITEMS = 4;
		else if (NehGameType == TYPE_GAME)
			MAIN_ITEMS = 5;
		else
			MAIN_ITEMS = 6;
	}
#endif
//	if (key_dest != key_menu)
//	{
//		m_save_demonum = cls.demonum;
//		cls.demonum = -1;
//	}
	key_dest = key_menu;
	m_state = m_main;
	m_entersound = true;
}

void M_Main_Draw (void)
{
	int	f;
	MYPICT	*p;
	M_DrawTransPic (16, 4, Draw_CachePic("gfx/qplaque.lmp"));
	p = Draw_CachePic ("gfx/ttl_main.lmp");
	M_DrawPic ((320-p->width)/2, 4, p);
#ifdef SUPPORTS_NEHAHRA
	if (nehahra)
	{
		if (NehGameType == TYPE_BOTH)
			M_DrawTransPic (72, 32, Draw_CachePic("gfx/mainmenu.lmp"));
		else if (NehGameType == TYPE_GAME)
			M_DrawTransPic (72, 32, Draw_CachePic("gfx/gamemenu.lmp"));
		else
			M_DrawTransPic (72, 32, Draw_CachePic("gfx/demomenu.lmp"));
	}
	else
#endif
		M_DrawTransPic (72, 32, Draw_CachePic("gfx/mainmenu.lmp"));
	f = (int)(realtime * 10)%6;  //johnfitz -- was host_time
	M_DrawTransPic (54, 32 + m_main_cursor*20, Draw_CachePic(va("gfx/menudot%i.lmp", f+1)));
}

void M_Main_Key (int key, int ascii)
{
	switch (key)
	{
	case K_ESCAPE:
#ifdef FLASH_FILE_SYSTEM
		//For FLASH, we write config.cfg every time we leave the main menu.
		//This is because we cant do it when we quit (as is done originally),
		//as you cant quit a flash app
		Host_WriteConfiguration();
#endif
		key_dest = key_game;
		m_state = m_none;
//		cls.demonum = m_save_demonum;
//		if (cls.demonum != -1 && !cls.demoplayback && cls.state != ca_connected)
//			CL_NextDemo ();
		break;
	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (++m_main_cursor >= MAIN_ITEMS)
			m_main_cursor = 0;
		break;
	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (--m_main_cursor < 0)
			m_main_cursor = MAIN_ITEMS - 1;
		break;
#ifdef KEYS_NOVEAU
	case K_HOME:
	case K_PGUP:
		S_LocalSound ("misc/menu1.wav");
		m_main_cursor = 0;
		break;
	case K_END:
	case K_PGDN:
		S_LocalSound ("misc/menu1.wav");
		m_main_cursor = MAIN_ITEMS - 1;
		break;
	case K_SPACE:
#endif
	case K_ENTER:
		m_entersound = true;
#ifdef SUPPORTS_NEHAHRA
		if (nehahra)	// nehahra menus
		{
			if (NehGameType == TYPE_GAME)
			{
				switch (m_main_cursor)
				{
				case 0:
					M_Menu_SinglePlayer_f ();
					break;
				case 1:
					M_Menu_MultiPlayer_f ();
					break;
				case 2:
					M_Menu_Options_f ();
					break;
				case 3:
					key_dest = key_game;
					if (sv.active)
						Cbuf_AddText ("disconnect\n");
					Cbuf_AddText ("playdemo ENDCRED\n");
					break;
				case 4:
					M_Menu_Quit_f ();
					break;
				}
			}
			else if (NehGameType == TYPE_DEMO)
			{
				switch (m_main_cursor)
				{
				case 0:
					M_Menu_NehDemos_f ();
					break;
				case 1:
					M_Menu_Options_f ();
					break;
				case 2:
					key_dest = key_game;
					if (sv.active)
						Cbuf_AddText ("disconnect\n");
					Cbuf_AddText ("playdemo ENDCRED\n");
					break;
				case 3:
					M_Menu_Quit_f ();
					break;
				}
			}
			else
			{
				switch (m_main_cursor)
				{
				case 0:
					M_Menu_SinglePlayer_f ();
					break;
				case 1:
					M_Menu_NehDemos_f ();
					break;
				case 2:
					M_Menu_MultiPlayer_f ();
					break;
				case 3:
					M_Menu_Options_f ();
					break;
        	                case 4:
					key_dest = key_game;
					if (sv.active)
						Cbuf_AddText ("disconnect\n");
					Cbuf_AddText ("playdemo ENDCRED\n");
					break;
				case 5:
					M_Menu_Quit_f ();
					break;
				}
			}
		}
		else	// original quake menu
		{
#endif
			switch (m_main_cursor)
			{
			case 0:
				M_Menu_SinglePlayer_f ();
				break;
			case 1:
				M_Menu_MultiPlayer_f ();
				break;
			case 2:
				M_Menu_Options_f ();
				break;
			case 3:
				M_Menu_Maps_f ();
				break;
			case 4:
				M_Menu_Demos_f ();
				break;
			case 5:
				M_Menu_Quit_f ();
				break;
			}
#ifdef SUPPORTS_NEHAHRA
		}
#endif
	}
}
//=============================================================================
/* SINGLE PLAYER MENU */
int	m_singleplayer_cursor;
#define	SINGLEPLAYER_ITEMS	3

void M_Menu_SinglePlayer_f (void)
{
	key_dest = key_menu;
	m_state = m_singleplayer;
	m_entersound = true;
}

void M_SinglePlayer_Draw (void)
{
	int	f;
	MYPICT	*p;
	M_DrawTransPic (16, 4, Draw_CachePic("gfx/qplaque.lmp"));
	p = Draw_CachePic ("gfx/ttl_sgl.lmp");
	M_DrawPic ((320-p->width)/2, 4, p);
	M_DrawTransPic (72, 32, Draw_CachePic("gfx/sp_menu.lmp"));
	f = (int)(realtime * 10)%6;  //johnfitz -- was host_time
	M_DrawTransPic (54, 32 + m_singleplayer_cursor * 20, Draw_CachePic(va("gfx/menudot%i.lmp", f+1)));
}
void M_SinglePlayer_Key (int key, int ascii)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Main_f ();
		break;
	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (++m_singleplayer_cursor >= SINGLEPLAYER_ITEMS)
			m_singleplayer_cursor = 0;
		break;
	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (--m_singleplayer_cursor < 0)
			m_singleplayer_cursor = SINGLEPLAYER_ITEMS - 1;
		break;
#ifdef KEYS_NOVEAU
	case K_HOME:
	case K_PGUP:
		S_LocalSound ("misc/menu1.wav");
		m_singleplayer_cursor = 0;
		break;
	case K_END:
	case K_PGDN:
		S_LocalSound ("misc/menu1.wav");
		m_singleplayer_cursor = SINGLEPLAYER_ITEMS - 1;
		break;
	case K_SPACE:
#endif
	case K_ENTER:
		m_entersound = true;
		switch (m_singleplayer_cursor)
		{
		case 0:
			if (sv.active)
				if (SCR_ModalMessage("\bNew Game Confirmation\b\n\nAre you sure you want to\nstart a new game?\n",0.0f, NULL)<=1)
					break;
			key_dest = key_game;
			if (sv.active)
				Cbuf_AddText ("disconnect\n");
			Cbuf_AddText ("maxplayers 1\n");
#ifdef SUPPORTS_NEHAHRA
			if (nehahra)
				Cbuf_AddText ("map nehstart\n");
			else
#endif
				Cbuf_AddText ("map start\n");
#if HIDE_DAMN_CONSOLE
			{
				extern qbool scr_disabled_for_newmap;
				extern qbool scr_drawloading;
				extern float scr_disabled_time;
				scr_disabled_for_newmap = true;
				scr_drawloading = true;
				scr_disabled_for_loading = true;
				scr_disabled_time = realtime;
			}
#endif
			break;
		case 1:
			M_Menu_Load_f ();
			break;
		case 2:
			M_Menu_Save_f ();
			break;
		}
	}
}
//=============================================================================
/* LOAD/SAVE MENU */
int	load_cursor;		// 0 < load_cursor < MAX_SAVEGAMES
#define	MAX_SAVEGAMES	12
char	m_filenames[MAX_SAVEGAMES][SAVEGAME_COMMENT_LENGTH+1];
int	loadable[MAX_SAVEGAMES];
void M_ScanSaves (void)
{
	int		i, j;
	char	name[MAX_OSPATH];
	FILE	*f;
	int		version;
	for (i=0 ; i<MAX_SAVEGAMES ; i++)
	{
		strcpy (m_filenames[i], "--- UNUSED SLOT ---");
		loadable[i] = false;
		snprintf (name, sizeof(name), "%s/s%i.sav", FOLDER_SAVES, i);
		f = FS_fopen_read (name, "r");	// Read save games
		if (!f)
			continue;
		fscanf (f, "%i\n", &version);
		fscanf (f, "%79s\n", name);
		strncpy (m_filenames[i], name, sizeof(m_filenames[i])-1);
	// change _ back to space
		for (j=0 ; j<SAVEGAME_COMMENT_LENGTH ; j++)
			if (m_filenames[i][j] == '_')
				m_filenames[i][j] = ' ';
		loadable[i] = true;
		fclose (f);
	}
}
void M_Menu_Load_f (void)
{
	m_entersound = true;
	m_state = m_load;
	key_dest = key_menu;
	M_ScanSaves ();
}

void M_Menu_Save_f (void)
{
	if (!sv.active)
		return;
	if (cl.intermission)
		return;
	if (svs.maxclients != 1)
		return;
	m_entersound = true;
	m_state = m_save;
	key_dest = key_menu;
	M_ScanSaves ();
}

void M_Load_Draw (void)
{
	int	i;
	MYPICT	*p;
	p = Draw_CachePic ("gfx/p_load.lmp");
	M_DrawPic ((320-p->width)/2, 4, p);
	for (i=0 ; i<MAX_SAVEGAMES ; i++)
		M_Print (16, 32 + 8*i, m_filenames[i]);
// line cursor
	M_DrawCharacter (8, 32 + load_cursor*8, 12+((int)(realtime*4)&1));
}

void M_Save_Draw (void)
{
	int	i;
	MYPICT	*p;
	p = Draw_CachePic ("gfx/p_save.lmp");
	M_DrawPic ((320-p->width)/2, 4, p);
	for (i=0 ; i<MAX_SAVEGAMES ; i++)
		M_Print (16, 32 + 8*i, m_filenames[i]);
// line cursor
	M_DrawCharacter (8, 32 + load_cursor*8, 12+((int)(realtime*4)&1));
}

void M_Load_Key (int key, int ascii)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_SinglePlayer_f ();
		break;
#ifdef KEYS_NOVEAU
	case K_SPACE:
#endif
	case K_ENTER:
		S_LocalSound ("misc/menu2.wav");
		if (!loadable[load_cursor])
			return;
		m_state = m_none;
		key_dest = key_game;
	// Host_Loadgame_f can't bring up the loading plaque because too much
	// stack space has been used, so do it now
		Con_DevPrintf (DEV_PROTOCOL, "Load Save Game in Menu: Begin loading plaque\n");
		SCR_BeginLoadingPlaque ();
	// issue the load command
		Cbuf_AddText (va("load s%i\n", load_cursor));
		return;
	case K_UPARROW:
	case K_LEFTARROW:
		S_LocalSound ("misc/menu1.wav");
		load_cursor--;
		if (load_cursor < 0)
			load_cursor = MAX_SAVEGAMES-1;
		break;
	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("misc/menu1.wav");
		load_cursor++;
		if (load_cursor >= MAX_SAVEGAMES)
			load_cursor = 0;
		break;
#ifdef KEYS_NOVEAU
	case K_HOME:
	case K_PGUP:
		S_LocalSound ("misc/menu1.wav");
		load_cursor = 0;
		break;
	case K_END:
	case K_PGDN:
		S_LocalSound ("misc/menu1.wav");
		load_cursor = MAX_SAVEGAMES - 1;
		break;
#endif
	}
}

void M_Save_Key (int key, int ascii)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_SinglePlayer_f ();
		break;
#ifdef KEYS_NOVEAU
	case K_SPACE:
#endif
	case K_ENTER:
		m_state = m_none;
		key_dest = key_game;
		Cbuf_AddText (va("save s%i\n", load_cursor));
		return;
	case K_UPARROW:
	case K_LEFTARROW:
		S_LocalSound ("misc/menu1.wav");
		load_cursor--;
		if (load_cursor < 0)
			load_cursor = MAX_SAVEGAMES - 1;
		break;
	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("misc/menu1.wav");
		load_cursor++;
		if (load_cursor >= MAX_SAVEGAMES)
			load_cursor = 0;
		break;
#ifdef KEYS_NOVEAU
	case K_HOME:
	case K_PGUP:
		S_LocalSound ("misc/menu1.wav");
		load_cursor = 0;
		break;
	case K_END:
	case K_PGDN:
		S_LocalSound ("misc/menu1.wav");
		load_cursor = MAX_SAVEGAMES - 1;
		break;
#endif
	}
}
//=============================================================================
/* MULTIPLAYER MENU */
int	m_multiplayer_cursor;
#define	MULTIPLAYER_ITEMS	4

void M_Menu_MultiPlayer_f (void)
{
	key_dest = key_menu;
	m_state = m_multiplayer;
	m_entersound = true;
}

void M_MultiPlayer_Draw (void)
{
	int	f;
	MYPICT	*p;
	M_DrawTransPic (16, 4, Draw_CachePic("gfx/qplaque.lmp"));
	p = Draw_CachePic ("gfx/p_multi.lmp");
	M_DrawPic ((320-p->width)/2, 4, p);
	M_DrawTransPic (72, 32, Draw_CachePic("gfx/mp_menu.lmp"));
	f = (int)(realtime * 10)%6;  //johnfitz -- was host_time
	M_DrawTransPic (54, 32 + m_multiplayer_cursor * 20, Draw_CachePic(va("gfx/menudot%i.lmp", f+1)));
	if (serialAvailable || ipxAvailable || tcpipAvailable)
		return;
	M_PrintWhite ((320/2) - ((27*8)/2), 148, "No Communications Available");
}

void M_MultiPlayer_Key (int key, int ascii)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Main_f ();
		break;
	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (++m_multiplayer_cursor >= MULTIPLAYER_ITEMS)
			m_multiplayer_cursor = 0;
		break;
	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (--m_multiplayer_cursor < 0)
			m_multiplayer_cursor = MULTIPLAYER_ITEMS - 1;
		break;
#ifdef KEYS_NOVEAU
	case K_HOME:
	case K_PGUP:
		S_LocalSound ("misc/menu1.wav");
		m_multiplayer_cursor = 0;
		break;
	case K_END:
	case K_PGDN:
		S_LocalSound ("misc/menu1.wav");
		m_multiplayer_cursor = MULTIPLAYER_ITEMS - 1;
		break;
	case K_SPACE:
#endif
	case K_ENTER:
		m_entersound = true;
		switch (m_multiplayer_cursor)
		{
		case 0:
			break;
		case 1:
		case 2:
			if (serialAvailable || ipxAvailable || tcpipAvailable)
				M_Menu_Net_f ();
			break;
		case 3:
			M_Menu_Setup_f ();
			break;
		}
	}
}
//=============================================================================
/* SETUP MENU */
int		setup_cursor = 5;
int	setup_cursor_table[] = {40, 56, 80, 104, 128, 152};
char	namemaker_name[16]; // Baker 3.83: Name maker
char	setup_hostname[16], setup_myname[16];
int	setup_oldtop, setup_oldbottom, setup_top, setup_bottom;
#define	NUM_SETUP_CMDS	6
void M_Menu_Setup_f (void)
{
	key_dest = key_menu;
	m_state = m_setup;
	m_entersound = true;

	StringLCopy (setup_hostname, sv_hostname.string);

	if (!(strlen(setup_myname)))
		StringLCopy (setup_myname, cl_net_name.string);//R00k

	setup_top = setup_oldtop = ((int)cl_net_color.integer) >> 4;
	setup_bottom = setup_oldbottom = ((int)cl_net_color.integer) & 15;
}
void M_Setup_Draw (void)
{
	MYPICT	*p;
	M_DrawTransPic (16, 4, Draw_CachePic("gfx/qplaque.lmp"));
	p = Draw_CachePic ("gfx/p_multi.lmp");
	M_DrawPic ((320-p->width)/2, 4, p);
	M_Print (64, 40, "Hostname");
	M_DrawTextBox (160, 32, 16, 1);
	M_Print (168, 40, setup_hostname);
	M_Print (64, 56, "Your name");
	M_DrawTextBox (160, 48, 16, 1);
	M_PrintWhite (168, 56, setup_myname);  // Baker 3.83: Draw it correctly!
	M_Print (64, 80, "Name Maker");
	M_Print (64, 104, "Shirt color");
	M_Print (64, 128, "Pants color");
	M_DrawTextBox (64, 152-8, 14, 1);
	M_Print (72, 152, "Accept Changes");
	p = Draw_CachePic ("gfx/bigbox.lmp");
	M_DrawTransPic (160, 64, p);
	p = Draw_CachePic ("gfx/menuplyr.lmp");
	M_BuildTranslationTable (setup_top*16, setup_bottom*16);
	M_DrawTransPicTranslate (176, 76, p);

	M_DrawCharacter (56, setup_cursor_table [setup_cursor], 12+((int)(realtime*4)&1));
	if (setup_cursor == 0)
		M_DrawCharacter (168 + 8*strlen(setup_hostname), setup_cursor_table [setup_cursor], 10+((int)(realtime*4)&1));
	if (setup_cursor == 1)
		M_DrawCharacter (168 + 8*strlen(setup_myname), setup_cursor_table [setup_cursor], 10+((int)(realtime*4)&1));
}

void M_Setup_Key (int key, int ascii)
{
	int	l;
	switch (key)
	{
	case K_ESCAPE:
		StringLCopy (setup_myname, cl_net_name.string);//R00k
		M_Menu_MultiPlayer_f ();
		break;
	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		setup_cursor--;
		if (setup_cursor < 0)
			setup_cursor = NUM_SETUP_CMDS - 1;
		break;
	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		setup_cursor++;
		if (setup_cursor >= NUM_SETUP_CMDS)
			setup_cursor = 0;
		break;
	case K_HOME:
		S_LocalSound ("misc/menu1.wav");
		setup_cursor = 0;
		break;
	case K_END:
		S_LocalSound ("misc/menu1.wav");
		setup_cursor = NUM_SETUP_CMDS - 1;
		break;
	case K_LEFTARROW:
		if (setup_cursor < 2)
			return;
		S_LocalSound ("misc/menu3.wav");
		if (setup_cursor == 3)
			setup_top = setup_top - 1;
		if (setup_cursor == 4)
			setup_bottom = setup_bottom - 1;
		break;
	case K_RIGHTARROW:
		if (setup_cursor < 2)
			return;
forward:
		S_LocalSound ("misc/menu3.wav");
		if (setup_cursor == 3)
			setup_top = setup_top + 1;
		if (setup_cursor == 4)
			setup_bottom = setup_bottom + 1;
		break;
	case K_SPACE:
	case K_ENTER:
		if (setup_cursor == 0 || setup_cursor == 1)
			return;
		if (setup_cursor == 3 || setup_cursor == 4)
			goto forward;
		if (setup_cursor == 2)
		{
			m_entersound = true;
			M_Menu_NameMaker_f ();
			break;
		}
		// setup_cursor == 5 (OK)
		Cbuf_AddText (va("name \"%s\"\n", setup_myname));
		Cvar_SetStringByRef(&sv_hostname, setup_hostname);
		Cbuf_AddText(va("color %i %i\n", setup_top, setup_bottom));
		m_entersound = true;
		M_Menu_MultiPlayer_f ();
		break;
	case K_BACKSPACE:
		if (setup_cursor == 0)
		{
			if (strlen(setup_hostname))
				setup_hostname[strlen(setup_hostname)-1] = 0;
		}
		if (setup_cursor == 1)
		{
			if (strlen(setup_myname))
				setup_myname[strlen(setup_myname)-1] = 0;
		}
		break;
	default:
		if (ascii < 32 || ascii > 127)
			break;
		Key_Extra (&ascii);
		if (setup_cursor == 0)
		{
			l = strlen(setup_hostname);
			if (l < 15)
			{
				setup_hostname[l+1] = 0;
				setup_hostname[l] = ascii;
			}
		}
		if (setup_cursor == 1)
		{
			l = strlen(setup_myname);
			if (l < 15)
			{
				setup_myname[l+1] = 0;
				setup_myname[l] = ascii;
			}
		}
	}
	if (setup_top > 15)
		setup_top = 0;
	if (setup_top < 0)
		setup_top = 15;
	if (setup_bottom > 15)
		setup_bottom = 0;
	if (setup_bottom < 0)
		setup_bottom = 15;
}

//=============================================================================
/* NAME MAKER MENU */ //From: JoeQuake 1.5Dev!!
//=============================================================================
int	namemaker_cursor_x, namemaker_cursor_y;
#define	NAMEMAKER_TABLE_SIZE	16
void M_Menu_NameMaker_f (void)
{
	key_dest = key_menu;
	key_special_dest = 1;
	m_state = m_namemaker;
	m_entersound = true;
	StringLCopy (namemaker_name, setup_myname);
}
void M_Shortcut_NameMaker_f (void)
{
// Baker: our little shortcut into the name maker
	menu_fastexit = true;
	StringLCopy (setup_myname, cl_net_name.string);//R00k
	namemaker_cursor_x = 0;
	namemaker_cursor_y = 0;
	M_Menu_NameMaker_f();
}

void M_NameMaker_Draw (void)
{
	int	x, y;

	M_Print (48, 16, "Your name");
	M_DrawTextBox (120, 8, 16, 1);
	M_PrintWhite (128, 16, namemaker_name);

	for (y=0 ; y<NAMEMAKER_TABLE_SIZE ; y++)
	{
		for (x=0 ; x<NAMEMAKER_TABLE_SIZE ; x++)
		{


#if 0
// Baker debug
//			int rectx = (32 + (16 * x)); //+ ((menuwidth - 320) >> 1);
//			int recty = 40 + (8 * y);// + (scr_menu_center.integer ? (menuheight - 200) / 2 : 0);
//			rectx = rectx + ((menuwidth - 320) >> 1);
//			recty = recty + m_yofs;
//			Draw_Fill(rectx, recty, 8,8, NAMEMAKER_TABLE_SIZE * y + x); // Draw our hotspots
#endif
			M_DrawCharacter (32 + (16 * x), 40 + (8 * y), NAMEMAKER_TABLE_SIZE * y + x);

		}
	}

	if (namemaker_cursor_y == NAMEMAKER_TABLE_SIZE)
		M_DrawCharacter (128, 184, 12 + ((int)(realtime*4)&1));
	else
		M_DrawCharacter (24 + 16*namemaker_cursor_x, 40 + 8*namemaker_cursor_y, 12 + ((int)(realtime*4)&1));

//	M_DrawTextBox (136, 176, 2, 1);
	M_Print (144, 184, "Press ESC to exit");
}

void Key_Extra (int *key);
void M_NameMaker_Key (int key, int ascii)
{
	int	l;

	switch (key)
	{
	case K_ESCAPE:
		key_special_dest = false;
		if (menu_fastexit)
		{// Allow quick exit for namemaker command
			key_dest = key_game;
			m_state = m_none;

			//Save the name
			Cbuf_AddText (va("name \"%s\"\n", namemaker_name));
			//Cvar_SetStringByRef(&sv_hostname, namemaker_name);
			// Clear the state
			menu_fastexit = false;
		}
		else
		{
			StringLCopy (setup_myname, namemaker_name);//R00k
			M_Menu_Setup_f ();
		}

		break;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		namemaker_cursor_y--;
		if (namemaker_cursor_y < 0)
			namemaker_cursor_y = NAMEMAKER_TABLE_SIZE-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		namemaker_cursor_y++;
		if (namemaker_cursor_y > NAMEMAKER_TABLE_SIZE-1)
			namemaker_cursor_y = 0;
		break;

	case K_PGUP:
		S_LocalSound ("misc/menu1.wav");
		namemaker_cursor_y = 0;
		break;

	case K_PGDN:
		S_LocalSound ("misc/menu1.wav");
		namemaker_cursor_y = NAMEMAKER_TABLE_SIZE-1;
		break;

	case K_LEFTARROW:
		S_LocalSound ("misc/menu1.wav");
		namemaker_cursor_x--;
		if (namemaker_cursor_x < 0)
			namemaker_cursor_x = NAMEMAKER_TABLE_SIZE - 1;
		break;

	case K_RIGHTARROW:
		S_LocalSound ("misc/menu1.wav");
		namemaker_cursor_x++;
		if (namemaker_cursor_x > NAMEMAKER_TABLE_SIZE - 1)
			namemaker_cursor_x = 0;
		break;

	case K_HOME:
		S_LocalSound ("misc/menu1.wav");
		namemaker_cursor_x = 0;
		break;

	case K_END:
		S_LocalSound ("misc/menu1.wav");
		namemaker_cursor_x = NAMEMAKER_TABLE_SIZE - 1;
		break;

	case K_BACKSPACE:
		if ((l = strlen(namemaker_name)))
			namemaker_name[l-1] = 0;
		break;

#ifdef _WIN32 // This K_MOUSECLICK namemaker and you'd need to figure out how to do it in OSX
#pragma message ("Get external mouse name maker to work with OSX")
	case K_MOUSECLICK_BUTTON1:
		{
			extern int extmousex, extmousey;
			extern int newmousex, newmousey;
			int		x, y, rectx, recty;
			qbool match=false;

#ifdef SUPPORTS_CENTERED_MENUS
			if (scr_menu_scale.integer) { // This is the default
				// we need to adjust the extmousex/y for menu effective size!
				extmousex = (float)extmousex*((float)menuwidth/(float)vid.width);
				extmousey = (float)extmousey*((float)menuheight/(float)vid.height);
			}
#endif
			for (y=0 ; y<NAMEMAKER_TABLE_SIZE ; y++)
			{
				for (x=0 ; x<NAMEMAKER_TABLE_SIZE ; x++)
				{
					//Draw_Character (cx + ((menuwidth - 320) >> 1), line + m_yofs, num);
//					rectx = (32 + (16 * x)) + ((menuwidth - 320) >> 1);
//					recty = 40 + (8 * y) + scr_menu_center.integer ? (menuheight - 200) / 2 : 0;
					rectx = (32 + (16 * x)); //+ ((menuwidth - 320) >> 1);
					recty = 40 + (8 * y);// + (scr_menu_center.integer ? (menuheight - 200) / 2 : 0);
					rectx = rectx + ((menuwidth - 320) >> 1);
					recty = recty + m_yofs;
					//M_DrawCharacter (32 + (16 * x), 40 + (8 * y), NAMEMAKER_TABLE_SIZE * y + x);
					//Draw_Fill(rectx, recty, 8,8, NAMEMAKER_TABLE_SIZE * y + x); // Draw our hotspots

					//Draw_Fill(rectx, recty, 8,8, 0); // Draw our hotspots
					if (extmousex >= rectx && extmousey >=recty)
					{
						if (extmousex <=rectx+7 && extmousey <= recty+7)
						{
							namemaker_cursor_x = x;
							namemaker_cursor_y = y;
							match = true;
						}
					}
					if (match) break;
				}
				if (match) break;
			}
//			Con_Printf("Mouse click x/y %d/%d\n", extmousex, extmousey);
//			Con_Printf("Match is %d = %d\n", match, namemaker_cursor_y * 16 + namemaker_cursor_x);
#ifdef _WIN32 // Baker ... fix this sloppyness
#ifdef _DEBUG
			{
				extern HWND mainwindow;
				SetWindowText(mainwindow, va("Mouse click %d %d", extmousex, extmousey));
			}
#endif // DEBUG
			if (!match)
			{
				// Baker: nothing was hit
				return;
			}
#endif // WIN32
		}
#endif // K MOUSECLICK
		// If we reached this point, we are simulating ENTER
	case K_SPACE:
	case K_ENTER:
		if (namemaker_cursor_y == NAMEMAKER_TABLE_SIZE)
		{
			StringLCopy (setup_myname, namemaker_name);
			M_Menu_Setup_f ();
		}
		else
		{
			l = strlen(namemaker_name);
			if (l < 15)
			{
				namemaker_name[l] = NAMEMAKER_TABLE_SIZE * namemaker_cursor_y + namemaker_cursor_x;
				namemaker_name[l+1] = 0;
			}
		}
		break;

	default:
		if (ascii < 32 || ascii > 127)
			break;

		Key_Extra (&ascii);

		l = strlen(namemaker_name);
		if (l < 15)
		{
			namemaker_name[l] = ascii;
			namemaker_name[l+1] = 0;
		}
		break;
	}
}
//=============================================================================
/* NET MENU */
int	m_net_cursor;
int m_net_items;
int m_net_saveHeight;
char *net_helpMessage[] =
{
/* .........1.........2.... */
  "                        ",
  " Two computers connected",
  "   through two modems.  ",
  "                        ",
  "                        ",
  " Two computers connected",
  " by a null-modem cable. ",
  "                        ",
  " Novell network LANs    ",
  " or Windows 95 DOS-box. ",
  "                        ",
  "(LAN=Local Area Network)",
  " Commonly used to play  ",
  " over the Internet, but ",
  " also used on a Local   ",
  " Area Network.          "
};
void M_Menu_Net_f (void)
{
	key_dest = key_menu;
	m_state = m_net;
	m_entersound = true;
	m_net_items = 4;
	if (m_net_cursor >= m_net_items)
		m_net_cursor = 0;
	m_net_cursor--;
	M_Net_Key (K_DOWNARROW,0);
}

void M_Net_Draw (void)
{
	int	f;
	MYPICT	*p;
	M_DrawTransPic (16, 4, Draw_CachePic("gfx/qplaque.lmp"));
	p = Draw_CachePic ("gfx/p_multi.lmp");
	M_DrawPic ((320-p->width)/2, 4, p);
	f = 32;
	if (serialAvailable)
	{
		p = Draw_CachePic ("gfx/netmen1.lmp");
	}
	else
	{
#ifdef _WIN32 // Baker: Unsure and doesn't seem important?
		p = NULL;
#else
		p = Draw_CachePic ("gfx/dim_modm.lmp");
#endif
	}
	if (p)
		M_DrawTransPic (72, f, p);
	f += 19;
	if (serialAvailable)
	{
		p = Draw_CachePic ("gfx/netmen2.lmp");
	}
	else
	{
#ifdef _WIN32 // Baker: Unsure and doesn't seem important?
		p = NULL;
#else
		p = Draw_CachePic ("gfx/dim_drct.lmp");
#endif
	}
	if (p)
		M_DrawTransPic (72, f, p);
	f += 19;
	if (ipxAvailable)
		p = Draw_CachePic ("gfx/netmen3.lmp");
	else
		p = Draw_CachePic ("gfx/dim_ipx.lmp");
	M_DrawTransPic (72, f, p);
	f += 19;
	if (tcpipAvailable)
		p = Draw_CachePic ("gfx/netmen4.lmp");
	else
		p = Draw_CachePic ("gfx/dim_tcp.lmp");
	M_DrawTransPic (72, f, p);
	if (m_net_items == 5)	// JDC, could just be removed
	{
		f += 19;
		p = Draw_CachePic ("gfx/netmen5.lmp");
		M_DrawTransPic (72, f, p);
	}
	f = (320 - 26 * 8) / 2;
	M_DrawTextBox (f, 134, 24, 4);
	f += 8;
	M_Print (f, 142, net_helpMessage[m_net_cursor*4+0]);
	M_Print (f, 150, net_helpMessage[m_net_cursor*4+1]);
	M_Print (f, 158, net_helpMessage[m_net_cursor*4+2]);
	M_Print (f, 166, net_helpMessage[m_net_cursor*4+3]);
	f = (int)(realtime * 10)%6;  //johnfitz -- was host_time
	M_DrawTransPic (54, 32 + m_net_cursor * 20, Draw_CachePic(va("gfx/menudot%i.lmp", f+1)));
}

void M_Net_Key (int key, int ascii)
{
again:
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_MultiPlayer_f ();
		break;
	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (++m_net_cursor >= m_net_items)
			m_net_cursor = 0;
		break;
	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (--m_net_cursor < 0)
			m_net_cursor = m_net_items - 1;
		break;
#ifdef KEYS_NOVEAU
	case K_SPACE:
#endif
	case K_ENTER:
		m_entersound = true;
		switch (m_net_cursor)
		{
		case 0:
			M_Menu_SerialConfig_f ();
			break;
		case 1:
			M_Menu_SerialConfig_f ();
			break;
		case 2:
			M_Menu_LanConfig_f ();
			break;
		case 3:
			M_Menu_LanConfig_f ();
			break;
		case 4:
// multiprotocol
			break;
		}
	}
	if (m_net_cursor == 0 && !serialAvailable)
		goto again;
	if (m_net_cursor == 1 && !serialAvailable)
		goto again;
	if (m_net_cursor == 2 && !ipxAvailable)
		goto again;
	if (m_net_cursor == 3 && !tcpipAvailable)
		goto again;
}
//=============================================================================
/* OPTIONS MENU */
//#ifdef GLQUAKE
#define	OPTIONS_ITEMS	19 // increased by 1 becoz of contrast - joe
//#else
//#define	OPTIONS_ITEMS	18 // ditto
//#endif
#define	SLIDER_RANGE	10
#ifdef GLQUAKE
#define gamma		"gl_gamma"
#define contrast	"gl_contrast"
#else
#define gamma		"gamma"
#define contrast	"contrast"
#endif
int	options_cursor;
void M_Menu_Options_f (void)
{
	key_dest = key_menu;
	m_state = m_options;
	m_entersound = true;
#ifdef _WIN32 // Extra option for windowed mouse on Win32.  I don't see why this would be Win32 specific
	if ((options_cursor == 18) && (modestate != MODE_WINDOWED)) // Baker 3.60 - New menu items
	{
		options_cursor = 0;
	}
#endif
}

void M_AdjustSliders (int dir)
{
//	float newval;
	S_LocalSound ("misc/menu3.wav");
	switch (options_cursor)
	{
	case 3:	// screen size
		Cvar_SetFloatByRef (&scene_viewsize, CLAMP (30, scene_viewsize.floater + dir * 10, 120));
		break;
	case 4:	// Baker 3.60 --- fov size
		Cvar_SetFloatByRef (&scene_fov_x, CLAMP (90, scene_fov_x.floater + dir * 1, 110));
		break;
	case 5:	// gamma
		Cvar_SetFloatByRef (&vid_brightness_gamma, CLAMP (vid_min_gamma,    vid_brightness_gamma.floater - dir * 0.05, vid_max_gamma));
		break;
	case 6:	// contrast
		Cvar_SetFloatByRef (&vid_brightness_contrast,  CLAMP (vid_min_contrast, vid_brightness_contrast.floater + dir * 0.1,   vid_max_contrast));
		break;
	case 7:	// mouse speed
		Cvar_SetFloatByRef (&in_sensitivity, CLAMP (1, in_sensitivity.floater + dir * 1,21));
		break;
	case 8:	// music volume
		Cvar_SetFloatByRef (&mp3_volume, CLAMP (0,mp3_volume.floater + dir * 0.1,1));
		break;
	case 9:	// sfx volume
		Cvar_SetFloatByRef (&snd_volume, CLAMP (0,snd_volume.floater + dir * 0.1,1));
		break;
	case 10:	// always run
		if (cl_speed_up.floater > 200)
		{
			// Maxxed to OFF
			Cvar_SetFloatByRef (&cl_speed_forward, 200);
			Cvar_SetFloatByRef (&cl_speed_back, 200);
			Cvar_SetFloatByRef (&cl_speed_side, 350);    // Baker 3.60 - added 350 is the default
			Cvar_SetFloatByRef (&cl_speed_up, 200);	// Baker 3.60 - added 350 is the default
		}
		else if (cl_speed_forward.floater > 200)
		{
			// Classic to Maxxed
			Cvar_SetFloatByRef (&cl_speed_forward, 999); // Baker 3.60 - previously 400
			Cvar_SetFloatByRef (&cl_speed_back, 999);    // Baker 3.60 - previously 400
			Cvar_SetFloatByRef (&cl_speed_side, 999);    // Baker 3.60 - added
			Cvar_SetFloatByRef (&cl_speed_up, 999);		// Baker 3.60 - added
		}
		else
		{
			// OFF to Classic
			Cvar_SetFloatByRef (&cl_speed_forward, 400); // Baker 3.60 - previously 400
			Cvar_SetFloatByRef (&cl_speed_back, 400);    // Baker 3.60 - previously 400
			Cvar_SetFloatByRef (&cl_speed_side, 350);    // Baker 3.60 - added
			Cvar_SetFloatByRef (&cl_speed_up, 200);		// Baker 3.60 - added
		}
		break;
	case 11:	// freelook
		Cvar_SetFloatByRef (&in_freelook, !in_freelook.integer);
		if (in_freelook.integer == 0)
			Cbuf_AddText ("force_centerview\n");	// Baker: courtesy
		break;
	case 12:	// invert mouse
		Cvar_SetFloatByRef (&in_invert_pitch, !in_invert_pitch.integer);
		break;
	case 13:	// mp3_enabled
#ifndef MACOSX
		if (MP3Audio_IsEnabled() != -1)
		{
			// Only switch the value if cdaudio is init'ed
			// otherwise we are just being confusing.

			Cvar_SetFloatByRef (&mp3_enabled, !mp3_enabled.integer);
		}
#endif
#pragma message ("OSX Fix and make MP3Audio_IsEnabled like Win32")
		break;
	case 14:	// lookspring
		Cvar_SetFloatByRef (&cl_ent_disable_blood, !cl_ent_disable_blood.integer);
		break;
	case 15:
		{
			int newval = (int)(user_effectslevel.integer + dir);
			if (newval < -1)
				newval = 1;
			if (newval > 1)
				newval = -1;
			Cvar_SetFloatByRef (&user_effectslevel, (float)newval);
		}
		break;
	case 18:
		// _windowed_mouse
		Cvar_SetFloatByRef (&_windowed_mouse, !_windowed_mouse.integer);
		break;
	}
}

void M_DrawSlider (int x, int y, float range)
{
	int	i;
	range = CLAMP (0, range, 1);
	M_DrawCharacter (x-8, y, 128);
	for (i=0 ; i<SLIDER_RANGE ; i++)
		M_DrawCharacter (x + i*8, y, 129);
	M_DrawCharacter (x+i*8, y, 130);
	M_DrawCharacter (x + (SLIDER_RANGE-1)*8 * range, y, 131);
}
void M_DrawCheckbox (int x, int y, int on)
{
	if (on)
		M_Print (x, y, "on");
	else
		M_Print (x, y, "off");
}
void M_Options_Draw (void)
{
	float	r;
	MYPICT	*p;
//	char	explanation[32];
	int 	i = 32;
	M_DrawTransPic (16, 4, Draw_CachePic("gfx/qplaque.lmp"));
	p = Draw_CachePic ("gfx/p_option.lmp");
	M_DrawPic ((320-p->width)/2, 4, p);
	M_Print (16, i, "    Customize controls"); i += 8;
	M_Print (16, i, "         Go to console"); i += 8;
	M_Print (16, i, "     Reset to defaults"); i += 8;
	M_Print (16, i, "           Screen size");
	r = (scene_viewsize.floater - 30) / (120 - 30);
	M_DrawSlider (220, i, r);  i += 8;
	M_Print (16, i, "         Field of view");
	r = (scene_fov_x.floater - 90) / (110 - 90);
	M_DrawSlider (220, i, r);  i += 8;
	M_Print (16, i, "                 Gamma");
	//r = (vid_brightness_gamma.floater-vid_min_gamma) / (vid_max_gamma-vid_min_gamma);
	r = (vid_max_gamma - vid_brightness_gamma.floater) / (vid_max_gamma - vid_min_gamma);
	M_DrawSlider (220, i, r);  i += 8;
	M_Print (16, i, "              Contrast");
//	r = vid_brightness_contrast.floater - 1.0;
	r =  (vid_brightness_contrast.floater - vid_min_contrast) / (vid_max_contrast - vid_min_contrast);
	M_DrawSlider (220, i, r);  i += 8;
	M_Print (16, i, "           Mouse Speed");
	r = (in_sensitivity.floater - 1)/20;
	M_DrawSlider (220, i, r);  i += 8;
	M_Print (16, i, "          Music Volume");
	r = mp3_volume.floater;
	M_DrawSlider (220, i, r);  i += 8;
	M_Print (16, i, "          Sound Volume");
	r = snd_volume.floater;
	M_DrawSlider (220, i, r);  i += 8;
	M_Print (16, i,  "            Always Run");
	M_Print (220, i, cl_speed_up.floater > 200 ? "maximum" : cl_speed_forward.floater > 200 ? "on" : "off");  i += 8;
	M_Print (16, i, "            Mouse Look");
	M_DrawCheckbox (220, i, in_freelook.integer);  i += 8;
	// End
	M_Print (16, i, "          Invert Mouse");
	M_DrawCheckbox (220, i, in_invert_pitch.integer);  i += 8;
	M_Print (16, i, "             MP3 Music");
#ifndef MACOSX
	switch (MP3Audio_IsEnabled())
	{
		case -1: M_Print (220, i, "[disabled]");	break;
		case  0: M_Print (220, i, "off");			break;
		default: M_Print (220, i, "on");			break;
	}
#endif
#pragma message ("OSX mp3audioenabled ")
 	i += 8;
	M_Print (16, i, "         Disable Blood");
	M_DrawCheckbox (220, i, cl_ent_disable_blood.integer);  i += 8;
	M_Print (16, i, "         Effects Level");
	switch (CLAMP (-2,user_effectslevel.integer,1))
	{
		case -1:	M_Print (220, i, "[minimum]");		break;
		case  0:	M_Print (220, i, "[automatic]");	break;
		case  1:	M_Print (220, i, "[maximum]");		break;
		default:	M_Print (220, i, "[free style]");	break;
	}
	i += 8;
	M_Print (16, i, "     Advanced Settings");  i += 8;
	if (vid_menudrawfn)
	{
#ifdef _WIN32
		char *VID_CurModeShort (void);
#endif
		M_Print (16, i, "           Video Modes");
#ifdef _WIN32
		M_Print (220, i, VID_CurModeShort());
#endif
#pragma message ("Make this work in OSX")

		i += 8;
	}

#ifdef _WIN32 // Extra option for windowed mouse on Win32.  I don't see why this would be Win32 specific
	if (vid_default == 0 /* windowed */)
#else
	if (vid_windowedmouse)
#endif
	{
		M_Print (16, i, "             Use Mouse");
		M_DrawCheckbox (220, i, _windowed_mouse.integer);
  		i += 8;
	}
// cursor
	M_DrawCharacter (200, 32 + options_cursor*8, 12+((int)(realtime*4)&1));
	switch 	(options_cursor)
	{
//	case  0: M_PrintWhite (16+8*7, 184, "Press ENTER to customize");		break;
//	case  1: M_PrintWhite (16+8*7, 184, "Press ENTER to customize");		break;
//	case  2: M_PrintWhite (16+8*7, 184, "Press ENTER to customize");		break;
	case  3: M_PrintWhite (16+8*7, 184, va("screen size: %i", scene_viewsize.integer));		break;
	case  4: M_PrintWhite (16+8*7, 184, va("field of view: %i degrees", scene_fov_x.integer));	break;
	case  5: M_PrintWhite (16+8*7, 184, va("gamma: %2.2f", vid_brightness_gamma.floater)); break;
	case  6: M_PrintWhite (16+8*7, 184, va("contrast: %2.1f", vid_brightness_contrast.floater)); break;
	case  7: M_PrintWhite (16+8*7, 184, va("mouse sensitivity: %2.1f", in_sensitivity.floater)); break;
//	case  8: M_PrintWhite (16+8*7, 184, "Press ENTER to customize");		break;
//	case  9: M_PrintWhite (16+8*7, 184, "Press ENTER to customize");		break;
//	case 10: M_PrintWhite (16+8*7, 184, "Press ENTER to customize");		break;
//	case 11: M_PrintWhite (16+8*7, 184, "Press ENTER to customize");		break;
//	case 12: M_PrintWhite (16+8*7, 184, "Press ENTER to customize");		break;
//	case 13: M_PrintWhite (16+8*7, 184, "Press ENTER to customize");		break;
	case 14: M_PrintWhite (16+8*7, 184,            "Removes gratitous effects");		break;
	case 15:
		switch (CLAMP (-2,user_effectslevel.integer,1))
		{
			case -1:	M_PrintWhite (16+8*7, 184, "Fast although a bit uncool");	break;
			case  0:	M_PrintWhite (16+8*7, 184, "Auto adjusts for game type");	break;
			case  1:	M_PrintWhite (16+8*7, 184, "Everything on and unwieldy");	break;
			default:	M_PrintWhite (16+8*7, 184, "Free form customization");		break;
		}
		break;
//	case 16: M_PrintWhite (16+8*7, 184, "Press ENTER to customize");		break;
//	case 17: M_PrintWhite (16+8*7, 184, ;		break;
	default:
			// do nothing
			break;
	}
}

void M_Options_Key (int key, int ascii)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Main_f ();
		break;
#ifdef KEYS_NOVEAU
	case K_SPACE:
#endif
	case K_ENTER:
		m_entersound = true;
		switch (options_cursor)
		{
		case 0:
			M_Menu_Keys_f ();
			break;
		case 1:
			m_state = m_none;
			key_dest = key_console;
//			Con_ToggleConsole_f ();
			break;
		case 2:
			if (SCR_ModalMessage("\bReset Settings Confirmation\b\n\nAre you sure you want to reset\nyour settings? (y/n)",0.0f, NULL)<=1)
				break;
			switch (SCR_ModalMessage("\bReset To Perform\b\n\nPress one of the following\nor ESC to abort\n\nD: Reset settings only\nK: Reset settings plus key binds\n\nC: Reset your keys and settings\n   to your most recent config",0.0f, "dkc"))
			{
			case 0: 	break; // Abort
			case 1:		// Just settings
						Cbuf_AddText ("cvar_resetall\n"); //johnfitz  Baker: My method can do a reset without screwing up your keys
						SCR_ModalMessage("\bReset Settings Successful\b\n\nYour settings have been reset\nto game defaults.\n\nPress ESC to clear this message.",0.0f, "y");
						break;
			case 2:		// Keys plus settings
						Cbuf_AddText ("cvar_resetall\n"); //johnfitz
						accepts_default_values = true;  // Baker: Is this necessary with my system?
						Cbuf_AddText ("exec default.cfg\n"); //Baker: this isn't quite gamedir neutral (We need this for keys mostly)
						Cbuf_AddText ("\nhint_defaults_done\n");	// Put this stuff in reverse order, InsertText jumps the line
						SCR_ModalMessage("\bReset Settings and Keys Successful\b\n\nKeys and settings have\nbeen reset to game defaults.\n\nPress ESC to clear this message.",0.0f, "y");
						break;
			case 3:
						Cbuf_AddText ("cvar_resetall\n"); //johnfitz
						accepts_default_values = true;	// Baker: Is this necessary with my system?
						Cbuf_AddText ("exec default.cfg\n"); //Baker: this isn't quite gamedir neutral
						Cbuf_AddText ("\nhint_defaults_done\n");	// Put this stuff in reverse order, InsertText jumps the line
						Cbuf_AddText ("exec config.cfg\n"); //Baker: this isn't quite gamedir neutral
						Cbuf_AddText ("exec autoexec.cfg\n"); //Baker: this isn't quite gamedir neutral
						SCR_ModalMessage("\bReset To Recent Config\b\n\nSettings reset to most recent\nconfiguration except for video mode\nand mp3 preferences.\n\nPress ESC to clear this message.",0.0f, "y");
						break;

			}
			// Question: should everything be unaliased too?
			// Question: what about instances where the default has been faked?
			// To do:    Add Cvar_SetDefaultFloatByRef
			break;
		case 15:
			M_Menu_VideoEffects_f ();
			break;
		case 16:
			M_Menu_Preferences_f ();
			break;
		case 17:
			//if (vid_menudrawfn)
			M_Menu_VideoModes_f ();
			break;
//			if (vid_menudrawfn)
//				M_Menu_VideoModes_f ();
			break;
		default:
			M_AdjustSliders (1);
			break;
		}
		return;
	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		options_cursor--;
		if (options_cursor < 0)
			options_cursor = OPTIONS_ITEMS - 1;
		break;
	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		options_cursor++;
		if (options_cursor >= OPTIONS_ITEMS)
			options_cursor = 0;
		break;
#ifdef KEYS_NOVEAU
	case K_HOME:
	case K_PGUP:
		S_LocalSound ("misc/menu1.wav");
		options_cursor = 0;
		break;
	case K_END:
	case K_PGDN:
		S_LocalSound ("misc/menu1.wav");
#ifndef MACOSX
		options_cursor = OPTIONS_ITEMS - (modestate == MODE_WINDOWED ? 1 : 2);
#endif
#pragma message ("OSX fix this windowed mouse modestate thing.  Make glvidosx do modestates")
		break;
#endif
	case K_LEFTARROW:
		M_AdjustSliders (-1);
		break;
	case K_RIGHTARROW:
		M_AdjustSliders (1);
		break;
	}
/*	if (options_cursor == 15 && vid_menudrawfn == NULL)
	{
		if (key == K_UPARROW)
			options_cursor = 14;
		else

	// JPG 1.05 - patch by CSR
#if defined(X11) || defined(_BSD)	 CSR - for _windowed_mouse
		if (key == K_DOWNARROW)
			options_cursor = 17;
		else
#endif
			options_cursor = 0;
		} */
#ifdef _WIN32 // Extra option for windowed mouse on Win32.  I don't see why this would be Win32 specific
#ifdef GLQUAKE
	if ((options_cursor == 18) && (vid_default !=0))
	{
		if (key == K_UPARROW)
			options_cursor = 17;
		else
			options_cursor = 0;
	}
#else
	if ((options_cursor == 17) && (vid_default !=0))
	{
		if (key == K_UPARROW)
			options_cursor = 16;
		else
			options_cursor = 0;
	}
#endif
#endif
}
//=============================================================================
/* KEYS MENU */
char *bindnames[][2] = // Baker 3.60 - more sensible customize controls, same options just organized better
{
{"+attack", 		"attack"},
{"+jump", 			"jump"},
{"+forward", 		"move forward"},
{"+back", 			"move back"},
{"+moveleft", 		"move left"},
{"+moveright", 		"move right"},
{"+moveup", 		"swim up"},
{"+movedown", 		"swim down"},
{"impulse 10", 		"change weapon"},
{"+speed", 			"run"},
{"+left", 			"turn left"},
{"+right", 			"turn right"},
{"+lookup", 		"look up"},
{"+lookdown", 		"look down"},
{"+mlook", 			"mouse look"},
{"+klook", 			"keyboard look"},
{"+strafe",			"sidestep"},
{"centerview",		"center view"}
};
// Messagemode2
// Hook
// Switch to best safe weapon
// Quickgrenade
// Say team location
// Say enemy location
// Rune Use
// Rune Delete
// Rune Tell
// Next weapon
/* Baker 3.60 -- Old menu
{
{"+attack", 		"attack"},
{"impulse 10", 		"change weapon"},
{"+jump", 			"jump / swim up"},
{"+forward", 		"walk forward"},
{"+back", 			"backpedal"},
{"+left", 			"turn left"},
{"+right", 			"turn right"},
{"+speed", 			"run"},
{"+moveleft", 		"step left"},
{"+moveright", 		"step right"},
{"+strafe", 		"sidestep"},
{"+lookup", 		"look up"},
{"+lookdown", 		"look down"},
{"centerview", 		"center view"},
{"+mlook", 			"mouse look"},
{"+klook", 			"keyboard look"},
{"+moveup",			"swim up"},
{"+movedown",		"swim down"}
}; */

#define	NUMCOMMANDS	(sizeof(bindnames)/sizeof(bindnames[0]))
int	keys_cursor;
int	bind_grab;
void M_Menu_Keys_f (void)
{
	key_dest = key_menu;
	key_special_dest = 2;
	m_state = m_keys;
	m_entersound = true;
}

void M_FindKeysForCommand (char *command, int *twokeys)
{
	int		count;
	int		j;
	int		l;
	char	*b;
	twokeys[0] = twokeys[1] = -1;
	l = strlen(command);
	count = 0;
	for (j=0 ; j<256 ; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		if (!strncmp(b, command, l))
		{
			twokeys[count] = j;
			count++;
			if (count == 2)
				break;
		}
	}
}
void M_UnbindCommand (char *command)
{
	int		j;
	int		l;
	char	*b;
	l = strlen(command);
	for (j=0 ; j<256 ; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		if (!strncmp(b, command, l))
			Key_SetBinding (j, "");
	}
}

void M_Keys_Draw (void)
{
	int		i, l;
	int		keys[2];
	char	*name;
	int		x, y;
	MYPICT	*p;
	p = Draw_CachePic ("gfx/ttl_cstm.lmp");
	M_DrawPic ((320-p->width)/2, 4, p);
	if (bind_grab)
		M_Print (12, 32, "Press a key or button for this action");
	else
		M_Print (18, 32, "Enter to change, backspace to clear");
// search for known bindings
	for (i=0 ; i<NUMCOMMANDS ; i++)
	{
		y = 48 + 8*i;
		M_Print (16, y, bindnames[i][1]);
		l = strlen (bindnames[i][0]);
		M_FindKeysForCommand (bindnames[i][0], keys);
		if (keys[0] == -1)
		{
			M_Print (140, y, "???");
		}
		else
		{
			name = Key_KeynumToString (keys[0]);
			M_Print (140, y, name);
			x = strlen(name) * 8;
			if (keys[1] != -1)
			{
				M_Print (140 + x + 8, y, "or");
				M_Print (140 + x + 32, y, Key_KeynumToString (keys[1]));
			}
		}
	}
	if (bind_grab)
		M_DrawCharacter (130, 48 + keys_cursor*8, '=');
	else
		M_DrawCharacter (130, 48 + keys_cursor*8, 12+((int)(realtime*4)&1));
}

void M_Keys_Key (int key, int ascii, qbool down)
{
	char	cmd[80];
	int	keys[2];
	if (bind_grab)
	{	// defining a key
		S_LocalSound ("misc/menu1.wav");
		if (key == K_ESCAPE)
		{
			bind_grab = false;
		}
		else if (ascii != '`')
		{
			//Con_Printf("Trying to bind %d",key);
			//Con_Printf("name is  %s",Key_KeynumToString (key));
			snprintf (cmd, sizeof(cmd), "bind \"%s\" \"%s\"\n", Key_KeynumToString (key), bindnames[keys_cursor][0]);
			Cbuf_InsertText (cmd);
		}
		bind_grab = false;
		return;
	}
	switch (key)
	{
	case K_ESCAPE:
		key_special_dest = false;
		M_Menu_Options_f ();
		break;
	case K_LEFTARROW:
	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		keys_cursor--;
		if (keys_cursor < 0)
			keys_cursor = NUMCOMMANDS-1;
		break;
	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("misc/menu1.wav");
		keys_cursor++;
		if (keys_cursor >= NUMCOMMANDS)
			keys_cursor = 0;
		break;
#ifdef KEYS_NOVEAU
	case K_HOME:
	case K_PGUP:
		S_LocalSound ("misc/menu1.wav");
		keys_cursor = 0;
		break;
	case K_END:
	case K_PGDN:
		S_LocalSound ("misc/menu1.wav");
		keys_cursor = NUMCOMMANDS - 1;
		break;
	case K_SPACE:
#endif
	case K_ENTER:		// go into bind mode
		M_FindKeysForCommand (bindnames[keys_cursor][0], keys);
		S_LocalSound ("misc/menu2.wav");
		if (keys[1] != -1)
			M_UnbindCommand (bindnames[keys_cursor][0]);
		bind_grab = true;
		break;
	case K_BACKSPACE:		// delete bindings
	case K_DEL:				// delete bindings
		S_LocalSound ("misc/menu2.wav");
		M_UnbindCommand (bindnames[keys_cursor][0]);
		break;
	}
}
//=============================================================================
/* ADVANCED SETTINGS MENU */

// Baker 3.60 - Added Advanced Settings Menu

#define	PREFERENCES_ITEMS	18 // Baker 3.60 - New Menu
//#define	PREF_SLIDER_RANGE	10 // Baker 3.60 - Needed for pq_maxfps ???

int		preferences_cursor=2;

void M_Menu_Preferences_f (void)
{
	key_dest = key_menu;
	m_state = m_preferences;  // Baker 3.60 - we are in the preferences menu
	m_entersound = true;
}

extern qbool commandline_dinput; // Baker 3.85 to support dinput switching


void M_Pref_AdjustSliders (int dir)
{
//	int newval;
	S_LocalSound ("misc/menu3.wav");
	// Baker 3.61 - Made menu bi-directional
	switch (preferences_cursor)
	{
	case 2:	Cvar_SetStringByRef(&scr_crosshair, scr_crosshair.integer ? "0" : "1");				break;  // crosshair  off | on | centered
	case 3:	Cvar_SetStringByRef(&r_drawviewmodel, r_drawviewmodel.floater ? "0" : "1");	break;  // draw weapon  on | off | always
	case 4: Cvar_SetStringByRef(&r_viewmodel_hackpos, r_viewmodel_hackpos.integer ? "0" : "1");	break;  // DarkPlaces versus normal
	case 5:
			if (vb_suitcshift.floater >= 1)
			{		// From full to lite
					Cvar_SetStringByRef(&vb_ringcshift,"0");
					Cvar_SetStringByRef(&vb_quadcshift,"0.3");
					Cvar_SetStringByRef(&vb_pentcshift,"0.3");
					Cvar_SetStringByRef(&vb_suitcshift,"0.3");
					Cvar_SetStringByRef(&vb_contentblend,"0");
			}
			else
			{		// From light to full
//					Cvar_SetStringByRef(&r_water_warp,"1");
					Cvar_SetStringByRef(&vb_ringcshift,"1");
					Cvar_SetStringByRef(&vb_quadcshift,"1");
					Cvar_SetStringByRef(&vb_pentcshift,"1");
					Cvar_SetStringByRef(&vb_suitcshift,"1");
					Cvar_SetStringByRef(&vb_contentblend,"1");
			}
			break;

		case 6:
			if (!v_rollangle.floater)  // Using most obscure value to avoid confusing someone who sets some of this
			{
				// Ok, we will assume it's all off so we turn everything on
				Cvar_SetStringByRef(&v_kickpitch,"0.6");
				Cvar_SetStringByRef(&v_kickroll,"0.6");
				Cvar_SetStringByRef(&v_kicktime,"0.5");
				Cvar_SetStringByRef(&v_bob,"0.02");
				Cvar_SetStringByRef(&v_bobcycle,"0.6");
				Cvar_SetStringByRef(&v_bobup, "0.5");
				Cvar_SetStringByRef(&v_rollangle,"2");
			}
			else
			{
				Cvar_SetStringByRef(&v_kickpitch,"0");
				Cvar_SetStringByRef(&v_kickroll,"0");
				Cvar_SetStringByRef(&v_kicktime,"0");
				Cvar_SetStringByRef(&v_bob,"0");
				Cvar_SetStringByRef(&v_bobcycle,"0");
				Cvar_SetStringByRef(&v_bobup, "0");
				Cvar_SetStringByRef(&v_rollangle,"0");
			}
			break;
		case 10: Cvar_SetStringByRef(&in_keypad, in_keypad.integer ? "0" : "1");  break;
		case 11: Cvar_SetStringByRef(&vid_altenter_toggle, vid_altenter_toggle.integer ? "0" : "1");  break;
		case 12: Cvar_SetStringByRef(&session_quickstart, session_quickstart.integer  ? "0" : "1");  break;
		case 13: Cvar_SetStringByRef(&session_confirmquit, session_confirmquit.integer ? "0" : "1");  break;
		case 14: Cvar_SetStringByRef(&in_smartjump, in_smartjump.integer ? "0" : "1");  break;
		case 15: Cvar_SetStringByRef(&snd_ambient_level, snd_ambient_level.floater ? "0" : "0.3"); break;
		case 16: Cvar_SetStringByRef(&mouse_directinput, mouse_directinput.integer ? "0" : "1"); break;
		case 17: Cvar_SetStringByRef(&in_keymap, in_keymap.integer ? "0" : "1"); break;
		default:	break;
	}
}

qbool IN_DirectInputON(void);

void M_Pref_Options_Draw (void)
{
	int i = 32;
	char *title;
	MYPICT	*p;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_option.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);


	title = "view setup"; M_PrintWhite ((320-8*strlen(title))/2, i, title); 	i += 8;								  // 0
	i += 8;
	M_Print     (16, i, "     crosshair      "); M_Print (220, i, scr_crosshair.integer ? "on" : "off");		i += 8; 	  // 2
	M_Print     (16, i, "     draw weapon    "); M_Print (220, i, r_drawviewmodel.floater ? "on": "off"); i += 8;     // 3
	M_Print     (16, i, "     weapon style   "); M_Print (220, i, r_viewmodel_hackpos.integer ? "classic" : "darkplaces" ); i += 8; 	  // 4
	M_Print     (16, i, "     view blends    "); M_Print (220, i, vb_suitcshift.floater >=1 ? "classic" : "deathmatch"); i += 8; 	  // 5
	M_Print     (16, i, "     disable bobbing"); M_Print (220, i, v_rollangle.floater ?  "off" : "on" ); i += 8; 	  // 6
	i += 8;																									  // 7
	title = "interface"; M_PrintWhite ((320-8*strlen(title))/2, i, title);  i += 8;							  // 8
	i += 8;																									  // 9
	M_Print     (16, i, "     keypad binding "); M_Print (220, i, in_keypad.integer ? "on" : "off"); i += 8; 	  // 10
	M_Print     (16, i, "     alt-enter key  "); M_Print (220, i, vid_altenter_toggle.integer ? "enabled" : "off"); i += 8; 	  // 10
	M_Print     (16, i, "     quick startup  "); M_Print (220, i, session_quickstart.integer  ? "enabled" : "disabled"); i += 8; 	  // 10
	M_Print     (16, i, "     quick quit     "); M_Print (220, i, session_confirmquit.integer ? "disabled" : "enabled"); i += 8; 	  // 10
	M_Print     (16, i, "     jump is moveup "); M_Print (220, i, in_smartjump.integer ? "on" : "off" ); i += 8; 	  // 11
	M_Print     (16, i, "     ambient sound  "); M_Print (220, i, snd_ambient_level.floater ? "on" : "off" ); i += 8; 	  // 12
	M_Print     (16, i, "     directinput mouse ");
#if SUPPORTS_DIRECTINPUT
	// Handle various cases
	{
		extern qbool in_dinput;
		if (mouse_directinput.integer && (Cvar_GetFlagsByRef(&mouse_directinput) & CVAR_ROM))	// Set to on by command line
			M_Print (220, i, in_dinput ? "[locked: on]"  : "[locked: err]");
		else if (!mouse_directinput.integer && (Cvar_GetFlagsByRef(&mouse_directinput) & CVAR_ROM))	// Set to on by command line
			M_Print (220, i, in_dinput ?  "[locked: err]" : "[locked: off]");
		else if ( mouse_directinput.integer && !in_dinput)
			M_Print (220, i, "[error]");
		else
			M_Print (220, i, mouse_directinput.integer ? "on" : "off" );
	}
#else
	M_Print (220, i, "Not available");
#endif
	 i += 8;

	M_Print     (16, i, "     keyboard automap"); M_Print (220, i, "n/a" );
	switch 	(preferences_cursor)
	{
//	case  0: M_PrintWhite (16+8*7, 184, "Press ENTER to customize");		break;
//	case  1: M_PrintWhite (16+8*7, 184, "Press ENTER to customize");		break;
	case  2: M_PrintWhite (16+5*8, 184, "enables/disable crosshair");		break;
	case  3: M_PrintWhite (16+5*8, 184, "draws your current weapon");		break;
	case  4: M_PrintWhite (16+5*8, 184, "affects how the weapon is drawn");	break;
	case  5: M_PrintWhite (16+5*8, 184, "on-screen powerup hues");		break;
	case  6: M_PrintWhite (16+5*8, 184, "bobbing gun and view kicks");		break;
//	case  7: M_PrintWhite (16+5*8, 184, va("mouse sensitivity: %2.1f", sensitivity.floater)); break;
//	case  8: M_PrintWhite (16+5*8, 184, "Press ENTER to customize");		break;
//	case  9: M_PrintWhite (16+5*8, 184, "Press ENTER to customize");		break;
	case 10: M_PrintWhite (16+5*8, 184, "separate binding of keypad");		break;
	case 11: M_PrintWhite (16+5*8, 184, "ALT-ENTER switches fullscreen");		break;
	case 12: M_PrintWhite (16+5*8, 184, "bypasses demos on startup");		break;
	case 13: M_PrintWhite (16+5*8, 184, "should quit get confirmation");		break;
	case 14: M_PrintWhite (16+5*8, 184, "makes jump \"key\" swim fast");		break;
	case 15: M_PrintWhite (16+5*8, 184, "silences sky/lava/etc sounds");		break;
	case 16: M_PrintWhite (16+5*8, 184, "uses DirectInput for mouse");		break;
	case 17: M_PrintWhite (16+5*8, 184, "automap for non-US keyboards");		break;
	default:
			// do nothing
			break;
	}
	M_DrawCharacter (200, 32 + preferences_cursor *8, 12+((int)(realtime*4)&1));


}



void M_Pref_Options_Key (int key, int ascii)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Options_f ();
		break;
#ifdef KEYS_NOVEAU
	case K_SPACE:
#endif
	case K_ENTER:
		m_entersound = true;
		switch (preferences_cursor)
		{
		default:
			M_Pref_AdjustSliders (1);
			break;
		}
		return;
#ifdef KEYS_NOVEAU
	case K_HOME:
	case K_PGUP:
		S_LocalSound ("misc/menu1.wav");
		preferences_cursor = 0;
		break;
	case K_END:
	case K_PGDN:
		S_LocalSound ("misc/menu1.wav");
		preferences_cursor = PREFERENCES_ITEMS-1;
		break;
#endif
	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		preferences_cursor--;
		if (preferences_cursor < 0)
			preferences_cursor = PREFERENCES_ITEMS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		preferences_cursor++;
		if (preferences_cursor >= PREFERENCES_ITEMS)
			preferences_cursor = 0;
		break;

	case K_LEFTARROW:
		M_Pref_AdjustSliders (-1);
		break;

	case K_RIGHTARROW:
		M_Pref_AdjustSliders (1);
		break;
	}

	if (preferences_cursor < 2) // the gray zone part 1!
	{
		if (key == K_UPARROW)
			preferences_cursor = PREFERENCES_ITEMS-1;
		else
			preferences_cursor = 2;
	}

	if (preferences_cursor >= 7 && preferences_cursor <= 9)
	{
		if (key == K_UPARROW)
			preferences_cursor = 6;
		else
			preferences_cursor = 10;
	}

	if (preferences_cursor == PREFERENCES_ITEMS)
	{
		if (key == K_UPARROW)
			preferences_cursor = PREFERENCES_ITEMS-1;
		else
			preferences_cursor = PREFERENCES_ITEMS-1;
	}


}

//=============================================================================
/* VIDEO MENU */
void M_Menu_VideoModes_f (void)
{
#if SUPPORTS_GLVIDEO_MODESWITCH
	(*vid_menucmdfn) (); //johnfitz
#else
	key_dest = key_menu;
	m_state = m_videomodes;
	m_entersound = true;
#endif
}

void M_VideoModes_Draw (void)
{
	(*vid_menudrawfn) ();
}

void M_VideoModes_Key (int key, int ascii)
{
	(*vid_menukeyfn) (key);
}
//=============================================================================
/* HELP MENU */	// joe: I decided to left it in, coz svc_sellscreen use it
int	help_page;
#if PROQUAKE_HELP_SCREEN
#define	NUM_HELP_PAGES	7  // JPG - was 6 (added ProQuake page)
#else
#define	NUM_HELP_PAGES	6
#endif

void M_Menu_Help_f (void)
{
	key_dest = key_menu;
	m_state = m_help;
	m_entersound = true;
	help_page = 0;
}
#if PROQUAKE_HELP_SCREEN
void M_ProQuake_Page (void)
{
	int f;
	M_DrawTextBox (16, 16, 34, 16);
	M_PrintWhite(32, 48,  va("     %s version %s", ENGINE_NAME, ENGINE_VERSION));
// Baker: fixme this isn't going to line up properly ^^^^^
	M_Print		(32, 72,  "          New Updates By Baker");
	M_PrintWhite(32, 80,  "       http://www.quakeone.com");
	M_Print		(32, 96,  "   Programmed by J.P. Grossman    ");
	M_PrintWhite(36, 112,  "                jpg@ai.mit.edu    ");
	M_Print		(28, 120,  " http://planetquake.com/proquake  ");
	M_Print     (32, 136, "<-- Previous            Next -->");
	f = (int)(realtime * 8)%6;
	M_DrawTransPic (48, 40 ,Draw_CachePic( va("gfx/menudot%i.lmp", f+1 ) ) );
	M_DrawTransPic (248, 40 ,Draw_CachePic( va("gfx/menudot%i.lmp", f?7-f:1) ) );
}
#endif

void M_Help_Draw (void)
{
#if PROQUAKE_HELP_SCREEN
// JPG - added ProQuake page
	if (help_page)
		M_DrawPic (0, 0, Draw_CachePic ( va("gfx/help%i.lmp", help_page - 1)) );
	else
		M_ProQuake_Page ();
#else
	M_DrawPic (0, 0, Draw_CachePic (va("gfx/help%i.lmp", help_page)));
#endif
}

void M_Help_Key (int key, int ascii)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Main_f ();
		break;
	case K_UPARROW:
	case K_RIGHTARROW:
		m_entersound = true;
		if (++help_page >= NUM_HELP_PAGES)
			help_page = 0;
		break;
	case K_DOWNARROW:
	case K_LEFTARROW:
		m_entersound = true;
		if (--help_page < 0)
			help_page = NUM_HELP_PAGES-1;
		break;
	}
}
//=============================================================================
/* QUIT MENU */
int		msgNumber;
int		m_quit_prevstate;
qbool	wasInMenus;
#ifndef	_WIN32 // Varying quit message.
char *quitMessage [] =
{
/* .........1.........2.... */
  "  Are you gonna quit    ",
  "  this game just like   ",
  "   everything else?     ",
  "                        ",
  " Milord, methinks that  ",
  "   thou art a lowly     ",
  " quitter. Is this true? ",
  "                        ",
  " Do I need to bust your ",
  "  face open for trying  ",
  "        to quit?        ",
  "                        ",
  " Man, I oughta smack you",
  "   for trying to quit!  ",
  "     Press Y to get     ",
  "      smacked out.      ",
  " Press Y to quit like a ",
  "   big loser in life.   ",
  "  Press N to stay proud ",
  "    and successful!     ",
  "   If you press Y to    ",
  "  quit, I will summon   ",
  "  Satan all over your   ",
  "      hard drive!       ",
  "  Um, Asmodeus dislikes ",
  " his children trying to ",
  " quit. Press Y to return",
  "   to your Tinkertoys.  ",
  "  If you quit now, I'll ",
  "  throw a blanket-party ",
  "   for you next time!   ",
  "                        "
};
#endif
void M_Menu_Quit_f (void)
{
	if (m_state == m_quit)
		return;
	wasInMenus = (key_dest == key_menu);
	key_dest = key_menu;
	m_quit_prevstate = m_state;
	m_state = m_quit;
	m_entersound = true;
	msgNumber = rand()&7;
}

void M_Quit_Key (int key, int ascii)
{
	switch (key)
	{
	case K_ESCAPE:
	case 'n':
	case 'N':
		if (wasInMenus)
		{
			m_state = m_quit_prevstate;
			m_entersound = true;
		}
		else
		{
			key_dest = key_game;
			m_state = m_none;
		}
		break;
#ifdef KEYS_NOVEAU
	case K_ENTER:
#endif
	case 'Y':
	case 'y':
		key_dest = key_console;
		Host_Quit_f ();
		break;
	default:
		break;
	}
}
void M_Quit_Draw (void)
{
#if WANTS_TO_DRAW_MENU_BEHIND
	if (wasInMenus)
	{
		m_state = m_quit_prevstate;
		m_recursiveDraw = true;
		M_Draw ();
		m_state = m_quit;
	}
#endif
#ifdef SUPPORTS_CENTERED_MENUS
	// Centered Canvas ON
	if (scr_menu_scale.integer)
	{
		menuwidth = 320;
		menuheight = min(vid.height, 240);
		eglMatrixMode (GL_PROJECTION);
		eglLoadIdentity ();
		eglOrtho (0, menuwidth, menuheight, 0, -99999, 99999);
	}
	else
	{
		menuwidth = vid.width;
		menuheight = vid.height;
	}
	m_yofs = scr_menu_center.integer ? (menuheight - 200) / 2 : 0;
#endif
#ifdef _WIN32 // Varying quit message
	M_DrawTextBox (0, 0, 38, 23);
	M_PrintWhite  (16, 12,  "  Quake version 1.09 by id Software\n\n");
	M_PrintWhite  (16, 28,  "Programming        Art \n");
	M_Print		  (16, 36,  " John Carmack       Adrian Carmack\n");
	M_Print		  (16, 44,  " Michael Abrash     Kevin Cloud\n");
	M_Print		  (16, 52,  " John Cash          Paul Steed\n");
	M_Print		  (16, 60,  " Dave 'Zoid' Kirsch\n");
	M_PrintWhite  (16, 68,  "Design             Biz\n");
	M_Print		  (16, 76,  " John Romero        Jay Wilbur\n");
	M_Print		  (16, 84,  " Sandy Petersen     Mike Wilson\n");
	M_Print		  (16, 92,  " American McGee     Donna Jackson\n");
	M_Print		  (16, 100, " Tim Willits        Todd Hollenshead\n");
	M_PrintWhite  (16, 108, "Support            Projects\n");
	M_Print		  (16, 116, " Barrett Alexander  Shawn Green\n");
	M_PrintWhite  (16, 124, "Sound Effects\n");
	M_Print		  (16, 132, " Trent Reznor and Nine Inch Nails\n\n");
	M_PrintWhite  (16, 140, "Quake is a trademark of Id Software,\n");
	M_PrintWhite  (16, 148, "inc., (c)1996 Id Software, inc. All\n");
	M_PrintWhite  (16, 156, "rights reserved. NIN logo is a\n");
	M_PrintWhite  (16, 164, "registered trademark licensed to\n");
	M_PrintWhite  (16, 172, "Nothing Interactive, Inc. All rights\n");
	M_PrintWhite  (16, 180, "reserved. Press y to exit\n");
#else
	M_DrawTextBox (56, 76, 24, 4);
	M_Print (64, 84,  quitMessage[msgNumber*4+0]);
	M_Print (64, 92,  quitMessage[msgNumber*4+1]);
	M_Print (64, 100, quitMessage[msgNumber*4+2]);
	M_Print (64, 108, quitMessage[msgNumber*4+3]);
#endif
#ifdef SUPPORTS_CENTERED_MENUS
	// Centered Canvas OFF
	if (scr_menu_scale.integer)
	{
		eglMatrixMode (GL_PROJECTION);
		eglLoadIdentity ();
		eglOrtho (0, vid.width, vid.height, 0, -99999, 99999);
	}
#endif
}
//=============================================================================
/* SERIAL CONFIG MENU */
int	serialConfig_cursor;
int	serialConfig_cursor_table[] = {48, 64, 80, 96, 112, 132};
#define	NUM_SERIALCONFIG_CMDS	6
static	int	ISA_uarts[] = {0x3f8, 0x2f8, 0x3e8, 0x2e8};
static	int	ISA_IRQs[] = {4, 3, 4, 3};
int		serialConfig_baudrate[] = {9600, 14400, 19200, 28800, 38400, 57600};
int	serialConfig_comport;
int	serialConfig_irq ;
int	serialConfig_baud;
char	serialConfig_phone[16];
void M_Menu_SerialConfig_f (void)
{
	int		n;
	int		port;
	int		baudrate;
	qbool	useModem;
	key_dest = key_menu;
	m_state = m_serialconfig;
	m_entersound = true;
	if (JoiningGame && SerialConfig)
		serialConfig_cursor = 4;
	else
		serialConfig_cursor = 5;
	(*GetComPortConfig)(0, &port, &serialConfig_irq, &baudrate, &useModem);
	// map uart's port to COMx
	for (n=0 ; n<4 ; n++)
		if (ISA_uarts[n] == port)
			break;
	if (n == 4)
	{
		n = 0;
		serialConfig_irq = 4;
	}
	serialConfig_comport = n + 1;
	// map baudrate to index
	for (n=0 ; n<6 ; n++)
		if (serialConfig_baudrate[n] == baudrate)
			break;
	if (n == 6)
		n = 5;
	serialConfig_baud = n;
	m_return_onerror = false;
	m_return_reason[0] = 0;
}

void M_SerialConfig_Draw (void)
{
	MYPICT	*p;
	int	basex;
	char	*startJoin;
	char	*directModem;
	M_DrawTransPic (16, 4, Draw_CachePic("gfx/qplaque.lmp"));
	p = Draw_CachePic ("gfx/p_multi.lmp");
	basex = (320-p->width)/2;
	M_DrawPic (basex, 4, p);
	if (StartingGame)
		startJoin = "New Game";
	else
		startJoin = "Join Game";
	if (SerialConfig)
		directModem = "Modem";
	else
		directModem = "Direct Connect";
	M_Print (basex, 32, va("%s - %s", startJoin, directModem));
	basex += 8;
	M_Print (basex, serialConfig_cursor_table[0], "Port");
	M_DrawTextBox (160, 40, 4, 1);
	M_Print (168, serialConfig_cursor_table[0], va("COM%u", serialConfig_comport));
	M_Print (basex, serialConfig_cursor_table[1], "IRQ");
	M_DrawTextBox (160, serialConfig_cursor_table[1]-8, 1, 1);
	M_Print (168, serialConfig_cursor_table[1], va("%u", serialConfig_irq));
	M_Print (basex, serialConfig_cursor_table[2], "Baud");
	M_DrawTextBox (160, serialConfig_cursor_table[2]-8, 5, 1);
	M_Print (168, serialConfig_cursor_table[2], va("%u", serialConfig_baudrate[serialConfig_baud]));
	if (SerialConfig)
	{
		M_Print (basex, serialConfig_cursor_table[3], "Modem Setup...");
		if (JoiningGame)
		{
			M_Print (basex, serialConfig_cursor_table[4], "Phone number");
			M_DrawTextBox (160, serialConfig_cursor_table[4]-8, 16, 1);
			M_Print (168, serialConfig_cursor_table[4], serialConfig_phone);
		}
	}
	if (JoiningGame)
	{
		M_DrawTextBox (basex, serialConfig_cursor_table[5]-8, 7, 1);
		M_Print (basex+8, serialConfig_cursor_table[5], "Connect");
	}
	else
	{
		M_DrawTextBox (basex, serialConfig_cursor_table[5]-8, 2, 1);
		M_Print (basex+8, serialConfig_cursor_table[5], "OK");
	}
	M_DrawCharacter (basex-8, serialConfig_cursor_table [serialConfig_cursor], 12+((int)(realtime*4)&1));
	if (serialConfig_cursor == 4)
		M_DrawCharacter (168 + 8*strlen(serialConfig_phone), serialConfig_cursor_table [serialConfig_cursor], 10+((int)(realtime*4)&1));
	if (*m_return_reason)
		M_PrintWhite (basex, 148, m_return_reason);
}

void M_SerialConfig_Key (int key, int ascii)
{
	int	l;
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Net_f ();
		break;
	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		serialConfig_cursor--;
		if (serialConfig_cursor < 0)
			serialConfig_cursor = NUM_SERIALCONFIG_CMDS-1;
		break;
	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		serialConfig_cursor++;
		if (serialConfig_cursor >= NUM_SERIALCONFIG_CMDS)
			serialConfig_cursor = 0;
		break;
	case K_LEFTARROW:
		if (serialConfig_cursor > 2)
			break;
		S_LocalSound ("misc/menu3.wav");
		if (serialConfig_cursor == 0)
		{
			serialConfig_comport--;
			if (serialConfig_comport == 0)
				serialConfig_comport = 4;
			serialConfig_irq = ISA_IRQs[serialConfig_comport-1];
		}
		if (serialConfig_cursor == 1)
		{
			serialConfig_irq--;
			if (serialConfig_irq == 6)
				serialConfig_irq = 5;
			if (serialConfig_irq == 1)
				serialConfig_irq = 7;
		}
		if (serialConfig_cursor == 2)
		{
			serialConfig_baud--;
			if (serialConfig_baud < 0)
				serialConfig_baud = 5;
		}
		break;
	case K_RIGHTARROW:
		if (serialConfig_cursor > 2)
			break;
forward:
		S_LocalSound ("misc/menu3.wav");
		if (serialConfig_cursor == 0)
		{
			serialConfig_comport++;
			if (serialConfig_comport > 4)
				serialConfig_comport = 1;
			serialConfig_irq = ISA_IRQs[serialConfig_comport-1];
		}
		if (serialConfig_cursor == 1)
		{
			serialConfig_irq++;
			if (serialConfig_irq == 6)
				serialConfig_irq = 7;
			if (serialConfig_irq == 8)
				serialConfig_irq = 2;
		}
		if (serialConfig_cursor == 2)
		{
			serialConfig_baud++;
			if (serialConfig_baud > 5)
				serialConfig_baud = 0;
		}
		break;
	case K_ENTER:
		if (serialConfig_cursor < 3)
			goto forward;
		m_entersound = true;
		if (serialConfig_cursor == 3)
		{
			(*SetComPortConfig) (0, ISA_uarts[serialConfig_comport-1], serialConfig_irq, serialConfig_baudrate[serialConfig_baud], SerialConfig);
			M_Menu_ModemConfig_f ();
			break;
		}
		if (serialConfig_cursor == 4)
		{
			serialConfig_cursor = 5;
			break;
		}
		// serialConfig_cursor == 5 (OK/CONNECT)
		(*SetComPortConfig) (0, ISA_uarts[serialConfig_comport-1], serialConfig_irq, serialConfig_baudrate[serialConfig_baud], SerialConfig);
		M_ConfigureNetSubsystem ();
		if (StartingGame)
		{
			M_Menu_GameOptions_f ();
			break;
		}
		m_return_state = m_state;
		m_return_onerror = true;
		key_dest = key_game;
		m_state = m_none;
		if (SerialConfig)
			Cbuf_AddText (va ("connect \"%s\"\n", serialConfig_phone));
		else
			Cbuf_AddText ("connect\n");
		break;
	case K_BACKSPACE:
		if (serialConfig_cursor == 4)
		{
			if (strlen(serialConfig_phone))
				serialConfig_phone[strlen(serialConfig_phone)-1] = 0;
		}
		break;
	default:
		if (ascii < 32 || key > ascii)
			break;
		if (serialConfig_cursor == 4)
		{
			l = strlen(serialConfig_phone);
			if (l < 15)
			{
				serialConfig_phone[l+1] = 0;
				serialConfig_phone[l] = key;
			}
		}
	}
	if (DirectConfig && (serialConfig_cursor == 3 || serialConfig_cursor == 4))
		if (key == K_UPARROW)
			serialConfig_cursor = 2;
		else
			serialConfig_cursor = 5;
	if (SerialConfig && StartingGame && serialConfig_cursor == 4)
		if (key == K_UPARROW)
			serialConfig_cursor = 3;
		else
			serialConfig_cursor = 5;
}
//=============================================================================
/* MODEM CONFIG MENU */
int	modemConfig_cursor;
int	modemConfig_cursor_table [] = {40, 56, 88, 120, 156};
#define NUM_MODEMCONFIG_CMDS	5
char	modemConfig_dialing;
char	modemConfig_clear [16];
char	modemConfig_init [32];
char	modemConfig_hangup [16];
void M_Menu_ModemConfig_f (void)
{
	key_dest = key_menu;
	m_state = m_modemconfig;
	m_entersound = true;
	(*GetModemConfig) (0, &modemConfig_dialing, modemConfig_clear, modemConfig_init, modemConfig_hangup);
}

void M_ModemConfig_Draw (void)
{
	MYPICT	*p;
	int	basex;
	M_DrawTransPic (16, 4, Draw_CachePic("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_multi.lmp");
	basex = (320-p->width)/2;
	M_DrawPic (basex, 4, p);
	basex += 8;
	if (modemConfig_dialing == 'P')
		M_Print (basex, modemConfig_cursor_table[0], "Pulse Dialing");
	else
		M_Print (basex, modemConfig_cursor_table[0], "Touch Tone Dialing");
	M_Print (basex, modemConfig_cursor_table[1], "Clear");
	M_DrawTextBox (basex, modemConfig_cursor_table[1]+4, 16, 1);
	M_Print (basex+8, modemConfig_cursor_table[1]+12, modemConfig_clear);
	if (modemConfig_cursor == 1)
		M_DrawCharacter (basex+8 + 8*strlen(modemConfig_clear), modemConfig_cursor_table[1]+12, 10+((int)(realtime*4)&1));
	M_Print (basex, modemConfig_cursor_table[2], "Init");
	M_DrawTextBox (basex, modemConfig_cursor_table[2]+4, 30, 1);
	M_Print (basex+8, modemConfig_cursor_table[2]+12, modemConfig_init);
	if (modemConfig_cursor == 2)
		M_DrawCharacter (basex+8 + 8*strlen(modemConfig_init), modemConfig_cursor_table[2]+12, 10+((int)(realtime*4)&1));
	M_Print (basex, modemConfig_cursor_table[3], "Hangup");
	M_DrawTextBox (basex, modemConfig_cursor_table[3]+4, 16, 1);
	M_Print (basex+8, modemConfig_cursor_table[3]+12, modemConfig_hangup);
	if (modemConfig_cursor == 3)
		M_DrawCharacter (basex+8 + 8*strlen(modemConfig_hangup), modemConfig_cursor_table[3]+12, 10+((int)(realtime*4)&1));
	M_DrawTextBox (basex, modemConfig_cursor_table[4]-8, 2, 1);
	M_Print (basex+8, modemConfig_cursor_table[4], "OK");
	M_DrawCharacter (basex-8, modemConfig_cursor_table [modemConfig_cursor], 12+((int)(realtime*4)&1));
}

void M_ModemConfig_Key (int key, int ascii)
{
	int	l;
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_SerialConfig_f ();
		break;
	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		modemConfig_cursor--;
		if (modemConfig_cursor < 0)
			modemConfig_cursor = NUM_MODEMCONFIG_CMDS-1;
		break;
	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		modemConfig_cursor++;
		if (modemConfig_cursor >= NUM_MODEMCONFIG_CMDS)
			modemConfig_cursor = 0;
		break;
	case K_LEFTARROW:
	case K_RIGHTARROW:
		if (modemConfig_cursor == 0)
		{
			if (modemConfig_dialing == 'P')
				modemConfig_dialing = 'T';
			else
				modemConfig_dialing = 'P';
			S_LocalSound ("misc/menu1.wav");
		}
		break;
	case K_ENTER:
		if (modemConfig_cursor == 0)
		{
			if (modemConfig_dialing == 'P')
				modemConfig_dialing = 'T';
			else
				modemConfig_dialing = 'P';
			m_entersound = true;
		}
		if (modemConfig_cursor == 4)
		{
			(*SetModemConfig) (0, va ("%c", modemConfig_dialing), modemConfig_clear, modemConfig_init, modemConfig_hangup);
			m_entersound = true;
			M_Menu_SerialConfig_f ();
		}
		break;
	case K_BACKSPACE:
		if (modemConfig_cursor == 1)
		{
			if (strlen(modemConfig_clear))
				modemConfig_clear[strlen(modemConfig_clear)-1] = 0;
		}
		if (modemConfig_cursor == 2)
		{
			if (strlen(modemConfig_init))
				modemConfig_init[strlen(modemConfig_init)-1] = 0;
		}
		if (modemConfig_cursor == 3)
		{
			if (strlen(modemConfig_hangup))
				modemConfig_hangup[strlen(modemConfig_hangup)-1] = 0;
		}
		break;
	default:
		if (ascii < 32 || ascii > 127)
			break;
		if (modemConfig_cursor == 1)
		{
			l = strlen(modemConfig_clear);
			if (l < 15)
			{
				modemConfig_clear[l+1] = 0;
				modemConfig_clear[l] = key;
			}
		}
		if (modemConfig_cursor == 2)
		{
			l = strlen(modemConfig_init);
			if (l < 29)
			{
				modemConfig_init[l+1] = 0;
				modemConfig_init[l] = key;
			}
		}
		if (modemConfig_cursor == 3)
		{
			l = strlen(modemConfig_hangup);
			if (l < 15)
			{
				modemConfig_hangup[l+1] = 0;
				modemConfig_hangup[l] = key;
			}
		}
	}
}
//=============================================================================
/* LAN CONFIG MENU */
int	lanConfig_cursor = -1;
int	lanConfig_cursor_table [] = {72, 92, 124};
#define NUM_LANCONFIG_CMDS	3
int 	lanConfig_port;
char	lanConfig_portname[6];
char	lanConfig_joinname[22];
void M_Menu_LanConfig_f (void)
{
	key_dest = key_menu;
	m_state = m_lanconfig;
	m_entersound = true;
	if (lanConfig_cursor == -1)
	{
		if (JoiningGame && TCPIPConfig)
			lanConfig_cursor = 2;
		else
			lanConfig_cursor = 1;
	}
	if (StartingGame && lanConfig_cursor == 2)
		lanConfig_cursor = 1;
	lanConfig_port = DEFAULTnet_hostport;
	snprintf (lanConfig_portname, sizeof(lanConfig_portname), "%u", lanConfig_port);
	m_return_onerror = false;
	m_return_reason[0] = 0;
}

void M_LanConfig_Draw (void)
{
	MYPICT	*p;
	int	basex;
	char	*startJoin;
	char	*protocol;
	M_DrawTransPic (16, 4, Draw_CachePic("gfx/qplaque.lmp"));
	p = Draw_CachePic ("gfx/p_multi.lmp");
	basex = (320-p->width)/2;
	M_DrawPic (basex, 4, p);
	if (StartingGame)
		startJoin = "New Game";
	else
		startJoin = "Join Game";
	if (IPXConfig)
		protocol = "IPX";
	else
		protocol = "TCP/IP";
	M_Print (basex, 32, va ("%s - %s", startJoin, protocol));
	basex += 8;
	M_Print (basex, 52, "Address:");
	if (IPXConfig)
		M_Print (basex+9*8, 52, my_ipx_address);
	else
		M_Print (basex+9*8, 52, my_tcpip_address);
	M_Print (basex, lanConfig_cursor_table[0], "Port");
	M_DrawTextBox (basex+8*8, lanConfig_cursor_table[0]-8, 6, 1);
	M_Print (basex+9*8, lanConfig_cursor_table[0], lanConfig_portname);
	if (JoiningGame)
	{
		M_Print (basex, lanConfig_cursor_table[1], "Search for local games...");
		M_Print (basex, 108, "Join game at:");
		M_DrawTextBox (basex+8, lanConfig_cursor_table[2]-8, 22, 1);
		M_Print (basex+16, lanConfig_cursor_table[2], lanConfig_joinname);
	}
	else
	{
		M_DrawTextBox (basex, lanConfig_cursor_table[1]-8, 2, 1);
		M_Print (basex+8, lanConfig_cursor_table[1], "OK");
	}
	M_DrawCharacter (basex-8, lanConfig_cursor_table [lanConfig_cursor], 12+((int)(realtime*4)&1));
	if (lanConfig_cursor == 0)
		M_DrawCharacter (basex+9*8 + 8*strlen(lanConfig_portname), lanConfig_cursor_table [0], 10+((int)(realtime*4)&1));
	if (lanConfig_cursor == 2)
		M_DrawCharacter (basex+16 + 8*strlen(lanConfig_joinname), lanConfig_cursor_table [2], 10+((int)(realtime*4)&1));
	if (*m_return_reason)
		M_PrintWhite (basex, 148, m_return_reason);
}

void M_LanConfig_Key (int key, int ascii)
{
	int		l;
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Net_f ();
		break;
	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		lanConfig_cursor--;
		if (lanConfig_cursor < 0)
			lanConfig_cursor = NUM_LANCONFIG_CMDS-1;
		break;
	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		lanConfig_cursor++;
		if (lanConfig_cursor >= NUM_LANCONFIG_CMDS)
			lanConfig_cursor = 0;
		break;
	case K_ENTER:
		if (lanConfig_cursor == 0)
			break;
		m_entersound = true;
		M_ConfigureNetSubsystem ();
		if (lanConfig_cursor == 1)
		{
			if (StartingGame)
			{
				M_Menu_GameOptions_f ();
				break;
			}
			M_Menu_Search_f();
			break;
		}
		if (lanConfig_cursor == 2)
		{
			m_return_state = m_state;
			m_return_onerror = true;
			key_dest = key_game;
			m_state = m_none;
			Cbuf_AddText ( va ("connect \"%s\"\n", lanConfig_joinname) );
			break;
		}
		break;
	case K_BACKSPACE:
		if (lanConfig_cursor == 0)
		{
			if (strlen(lanConfig_portname))
				lanConfig_portname[strlen(lanConfig_portname)-1] = 0;
		}
		if (lanConfig_cursor == 2)
		{
			if (strlen(lanConfig_joinname))
				lanConfig_joinname[strlen(lanConfig_joinname)-1] = 0;
		}
		break;
	default:
		if (ascii < 32 || ascii > 127)
			break;
		if (lanConfig_cursor == 2)
		{
			l = strlen(lanConfig_joinname);
			if (l < 21)
			{
				lanConfig_joinname[l+1] = 0;
				lanConfig_joinname[l] = key;
			}
		}
		if (ascii < '0' || ascii > '9')
			break;
		if (lanConfig_cursor == 0)
		{
			l = strlen(lanConfig_portname);
			if (l < 5)
			{
				lanConfig_portname[l+1] = 0;
				lanConfig_portname[l] = key;
			}
		}
	}
	if (StartingGame && lanConfig_cursor == 2)
		if (key == K_UPARROW)
			lanConfig_cursor = 1;
		else
			lanConfig_cursor = 0;
	l = atoi(lanConfig_portname);
	if (l > 65535)
		l = lanConfig_port;
	else
		lanConfig_port = l;
	snprintf (lanConfig_portname, sizeof(lanConfig_portname), "%u", lanConfig_port);
}
//=============================================================================
/* GAME OPTIONS MENU */
typedef struct
{
	char	*name;
	char	*description;
} level_t;
level_t	levels[] =
{
	{"start", "Entrance"},	// 0
	{"e1m1", "Slipgate Complex"},				// 1
	{"e1m2", "Castle of the Damned"},
	{"e1m3", "The Necropolis"},
	{"e1m4", "The Grisly Grotto"},
	{"e1m5", "Gloom Keep"},
	{"e1m6", "The Door To Chthon"},
	{"e1m7", "The House of Chthon"},
	{"e1m8", "Ziggurat Vertigo"},
	{"e2m1", "The Installation"},				// 9
	{"e2m2", "Ogre Citadel"},
	{"e2m3", "Crypt of Decay"},
	{"e2m4", "The Ebon Fortress"},
	{"e2m5", "The Wizard's Manse"},
	{"e2m6", "The Dismal Oubliette"},
	{"e2m7", "Underearth"},
	{"e3m1", "Termination Central"},			// 16
	{"e3m2", "The Vaults of Zin"},
	{"e3m3", "The Tomb of Terror"},
	{"e3m4", "Satan's Dark Delight"},
	{"e3m5", "Wind Tunnels"},
	{"e3m6", "Chambers of Torment"},
	{"e3m7", "The Haunted Halls"},
	{"e4m1", "The Sewage System"},				// 23
	{"e4m2", "The Tower of Despair"},
	{"e4m3", "The Elder God Shrine"},
	{"e4m4", "The Palace of Hate"},
	{"e4m5", "Hell's Atrium"},
	{"e4m6", "The Pain Maze"},
	{"e4m7", "Azure Agony"},
	{"e4m8", "The Nameless City"},
	{"end", "Shub-Niggurath's Pit"},			// 31
	{"dm1", "Place of Two Deaths"},				// 32
	{"dm2", "Claustrophobopolis"},
	{"dm3", "The Abandoned Base"},
	{"dm4", "The Bad Place"},
	{"dm5", "The Cistern"},
	{"dm6", "The Dark Zone"}
};
//MED 01/06/97 added hipnotic levels
level_t	hipnoticlevels[] =
{
	{"start", "Command HQ"},	// 0
	{"hip1m1", "The Pumping Station"},		// 1
	{"hip1m2", "Storage Facility"},
	{"hip1m3", "The Lost Mine"},
	{"hip1m4", "Research Facility"},
	{"hip1m5", "Military Complex"},
	{"hip2m1", "Ancient Realms"},			// 6
	{"hip2m2", "The Black Cathedral"},
	{"hip2m3", "The Catacombs"},
	{"hip2m4", "The Crypt"},
	{"hip2m5", "Mortum's Keep"},
	{"hip2m6", "The Gremlin's Domain"},
	{"hip3m1", "Tur Torment"},			// 12
	{"hip3m2", "Pandemonium"},
	{"hip3m3", "Limbo"},
	{"hip3m4", "The Gauntlet"},
	{"hipend", "Armagon's Lair"},			// 16
	{"hipdm1", "The Edge of Oblivion"}		// 17
};
//PGM 01/07/97 added rogue levels
//PGM 03/02/97 added dmatch level
level_t	roguelevels[] =
{
	{"start", "Split Decision"},
	{"r1m1", "Deviant's Domain"},
	{"r1m2", "Dread Portal"},
	{"r1m3", "Judgement Call"},
	{"r1m4", "Cave of Death"},
	{"r1m5", "Towers of Wrath"},
	{"r1m6", "Temple of Pain"},
	{"r1m7", "Tomb of the Overlord"},
	{"r2m1", "Tempus Fugit"},
	{"r2m2", "Elemental Fury I"},
	{"r2m3", "Elemental Fury II"},
	{"r2m4", "Curse of Osiris"},
	{"r2m5", "Wizard's Keep"},
	{"r2m6", "Blood Sacrifice"},
	{"r2m7", "Last Bastion"},
	{"r2m8", "Source of Evil"},
	{"ctf1", "Division of Change"}
};
typedef struct
{
	char	*description;
	int		firstLevel;
	int		levels;
} episode_t;
episode_t episodes[] =
{
	{"Welcome to Quake", 0, 1},
	{"Doomed Dimension", 1, 8},
	{"Realm of Black Magic", 9, 7},
	{"Netherworld", 16, 7},
	{"The Elder World", 23, 8},
	{"Final Level", 31, 1},
	{"Deathmatch Arena", 32, 6}
};
//MED 01/06/97  added hipnotic episodes
episode_t hipnoticepisodes[] =
{
	{"Scourge of Armagon", 0, 1},
	{"Fortress of the Dead", 1, 5},
	{"Dominion of Darkness", 6, 6},
	{"The Rift", 12, 4},
	{"Final Level", 16, 1},
	{"Deathmatch Arena", 17, 1}
};
//PGM 01/07/97 added rogue episodes
//PGM 03/02/97 added dmatch episode
episode_t rogueepisodes[] =
{
	{"Introduction", 0, 1},
	{"Hell's Fortress", 1, 7},
	{"Corridors of Time", 8, 8},
	{"Deathmatch Arena", 16, 1}
};
int	startepisode;
int	startlevel;
int	maxplayers;
qbool m_serverInfoMessage = false;
double	m_serverInfoMessageTime;
void M_Menu_GameOptions_f (void)
{
	key_dest = key_menu;
	m_state = m_gameoptions;
	m_entersound = true;
	if (maxplayers == 0)
		maxplayers = svs.maxclients;
	if (maxplayers < 2)
		maxplayers = svs.maxclientslimit;
}

int	gameoptions_cursor_table[] = {40, 56, 64, 72, 80, 88, 96, 112, 120};
#define	NUM_GAMEOPTIONS	9
int	gameoptions_cursor;
void M_GameOptions_Draw (void)
{
	MYPICT	*p;
	int	x;
	M_DrawTransPic (16, 4, Draw_CachePic("gfx/qplaque.lmp"));
	p = Draw_CachePic ("gfx/p_multi.lmp");
	M_DrawPic ((320-p->width)/2, 4, p);
	M_DrawTextBox (152, 32, 10, 1);
	M_Print (160, 40, "begin game");
	M_Print (0, 56, "      Max players");
	M_Print (160, 56, va("%i", maxplayers));
	M_Print (0, 64, "        Game Type");
	if (pr_coop.floater)
		M_Print (160, 64, "Cooperative");
	else
		M_Print (160, 64, "Deathmatch");
	M_Print (0, 72, "        Teamplay");
	if (rogue)
	{
		char	*msg;
		switch ((int)pr_teamplay.floater)
		{
			case 1: msg = "No Friendly Fire"; break;
			case 2: msg = "Friendly Fire"; break;
			case 3: msg = "Tag"; break;
			case 4: msg = "Capture the Flag"; break;
			case 5: msg = "One Flag CTF"; break;
			case 6: msg = "Three Team CTF"; break;
			default: msg = "Off"; break;
		}
		M_Print (160, 72, msg);
	}
	else
	{
		char	*msg;
		switch ((int)pr_teamplay.floater)
		{
			case 1: msg = "No Friendly Fire"; break;
			case 2: msg = "Friendly Fire"; break;
			default: msg = "Off"; break;
		}
		M_Print (160, 72, msg);
	}
	M_Print (0, 80, "            Skill");
	if (pr_skill.floater == 0)
		M_Print (160, 80, "Easy difficulty");
	else if (pr_skill.floater == 1)
		M_Print (160, 80, "Normal difficulty");
	else if (pr_skill.floater == 2)
		M_Print (160, 80, "Hard difficulty");
	else
		M_Print (160, 80, "Nightmare difficulty");
	M_Print (0, 88, "       Frag Limit");
	if (pr_fraglimit.floater == 0)
		M_Print (160, 88, "none");
	else
		M_Print (160, 88, va("%i frags", (int)pr_fraglimit.floater));
	M_Print (0, 96, "       Time Limit");
	if (pr_timelimit.floater == 0)
		M_Print (160, 96, "none");
	else
		M_Print (160, 96, va("%i minutes", (int)pr_timelimit.floater));
	M_Print (0, 112, "         Episode");
//MED 01/06/97 added hipnotic episodes
	if (hipnotic)
		M_Print (160, 112, hipnoticepisodes[startepisode].description);
//PGM 01/07/97 added rogue episodes
	else if (rogue)
		M_Print (160, 112, rogueepisodes[startepisode].description);
	else
		M_Print (160, 112, episodes[startepisode].description);
	M_Print (0, 120, "           Level");
//MED 01/06/97 added hipnotic episodes
	if (hipnotic)
	{
		M_Print (160, 120, hipnoticlevels[hipnoticepisodes[startepisode].firstLevel + startlevel].description);
		M_Print (160, 128, hipnoticlevels[hipnoticepisodes[startepisode].firstLevel + startlevel].name);
	}
//PGM 01/07/97 added rogue episodes
	else if (rogue)
	{
		M_Print (160, 120, roguelevels[rogueepisodes[startepisode].firstLevel + startlevel].description);
		M_Print (160, 128, roguelevels[rogueepisodes[startepisode].firstLevel + startlevel].name);
	}
	else
	{
		M_Print (160, 120, levels[episodes[startepisode].firstLevel + startlevel].description);
		M_Print (160, 128, levels[episodes[startepisode].firstLevel + startlevel].name);
	}
// line cursor
	M_DrawCharacter (144, gameoptions_cursor_table[gameoptions_cursor], 12+((int)(realtime*4)&1));
	if (m_serverInfoMessage)
	{
		if ((realtime - m_serverInfoMessageTime) < 5.0)
		{
			x = (320-26*8)/2;
			M_DrawTextBox (x, 138, 24, 4);
			x += 8;
			M_Print (x, 146, "  More than 4 players   ");
			M_Print (x, 154, " requires using command ");
			M_Print (x, 162, "line parameters; please ");
			M_Print (x, 170, "   see techinfo.txt.    ");
		}
		else
		{
			m_serverInfoMessage = false;
		}
	}
}

void M_NetStart_Change (int dir)
{
	int	count;
	switch (gameoptions_cursor)
	{
	case 1:
		maxplayers += dir;
		if (maxplayers > svs.maxclientslimit)
		{
			maxplayers = svs.maxclientslimit;
			m_serverInfoMessage = true;
			m_serverInfoMessageTime = realtime;
		}
		if (maxplayers < 2)
			maxplayers = 2;
		break;
	case 2:
		Cvar_SetFloatByRef (&pr_coop, pr_coop.floater ? 0 : 1);
		break;
	case 3:
		if (rogue)
			count = 6;
		else
			count = 2;
		Cvar_SetFloatByRef (&pr_teamplay, pr_teamplay.floater + dir);
		if (pr_teamplay.floater > count)
			Cvar_SetStringByRef (&pr_teamplay, "0");
		else if (pr_teamplay.floater < 0)
			Cvar_SetFloatByRef (&pr_teamplay, count);
		break;
	case 4:
		Cvar_SetFloatByRef (&pr_skill, pr_skill.floater + dir);
		if (pr_skill.floater > 3)
			Cvar_SetStringByRef (&pr_skill, "0");
		if (pr_skill.floater < 0)
			Cvar_SetStringByRef (&pr_skill, "3");
		break;
	case 5:
		Cvar_SetFloatByRef (&pr_fraglimit, pr_fraglimit.floater + dir*10);
		if (pr_fraglimit.floater > 100)
			Cvar_SetStringByRef (&pr_fraglimit, "0");
		if (pr_fraglimit.floater < 0)
			Cvar_SetStringByRef (&pr_fraglimit, "100");
		break;
	case 6:
		Cvar_SetFloatByRef (&pr_timelimit, pr_timelimit.floater + dir*5);
		if (pr_timelimit.floater > 60)
			Cvar_SetStringByRef (&pr_timelimit, "0");
		if (pr_timelimit.floater < 0)
			Cvar_SetStringByRef (&pr_timelimit, "60");
		break;
	case 7:
		startepisode += dir;
	//MED 01/06/97 added hipnotic count
		if (hipnotic)
			count = 6;
	//PGM 01/07/97 added rogue count
	//PGM 03/02/97 added 1 for dmatch episode
		else if (rogue)
			count = 4;
		else if (session_registered.integer)
			count = 7;
		else
			count = 2;
		if (startepisode < 0)
			startepisode = count - 1;
		if (startepisode >= count)
			startepisode = 0;
		startlevel = 0;
		break;
	case 8:
		startlevel += dir;
	//MED 01/06/97 added hipnotic episodes
		if (hipnotic)
			count = hipnoticepisodes[startepisode].levels;
	//PGM 01/06/97 added hipnotic episodes
		else if (rogue)
			count = rogueepisodes[startepisode].levels;
		else
			count = episodes[startepisode].levels;
		if (startlevel < 0)
			startlevel = count - 1;
		if (startlevel >= count)
			startlevel = 0;
		break;
	}
}
void M_GameOptions_Key (int key, int ascii)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Net_f ();
		break;
	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		gameoptions_cursor--;
		if (gameoptions_cursor < 0)
			gameoptions_cursor = NUM_GAMEOPTIONS-1;
		break;
	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		gameoptions_cursor++;
		if (gameoptions_cursor >= NUM_GAMEOPTIONS)
			gameoptions_cursor = 0;
		break;
#ifdef KEYS_NOVEAU
	case K_HOME:
		S_LocalSound ("misc/menu1.wav");
		gameoptions_cursor = 0;
		break;
	case K_END:
		S_LocalSound ("misc/menu1.wav");
		gameoptions_cursor = NUM_GAMEOPTIONS-1;
		break;
#endif
	case K_LEFTARROW:
		if (gameoptions_cursor == 0)
			break;
		S_LocalSound ("misc/menu3.wav");
		M_NetStart_Change (-1);
		break;
	case K_RIGHTARROW:
		if (gameoptions_cursor == 0)
			break;
		S_LocalSound ("misc/menu3.wav");
		M_NetStart_Change (1);
		break;
#ifdef KEYS_NOVEAU
	case K_SPACE:
#endif
	case K_ENTER:
		S_LocalSound ("misc/menu2.wav");
		if (gameoptions_cursor == 0)
		{
			if (sv.active)
				Cbuf_AddText ("disconnect\n");
			Cbuf_AddText ("listen 0\n");	// so host_netport will be re-examined
			Cbuf_AddText (va("maxplayers %u\n", maxplayers));
			Con_DevPrintf (DEV_PROTOCOL, "Starting multiplayer listen server: Begin loading plaque\n");
			SCR_BeginLoadingPlaque ();
			if (hipnotic)
				Cbuf_AddText (va("map %s\n", hipnoticlevels[hipnoticepisodes[startepisode].firstLevel + startlevel].name));
			else if (rogue)
				Cbuf_AddText (va("map %s\n", roguelevels[rogueepisodes[startepisode].firstLevel + startlevel].name));
			else
				Cbuf_AddText (va("map %s\n", levels[episodes[startepisode].firstLevel + startlevel].name));
			return;
		}
		M_NetStart_Change (1);
		break;
	}
}
//=============================================================================
/* SEARCH MENU */
qbool	searchComplete = false;
double		searchCompleteTime;
void M_Menu_Search_f (void)
{
	key_dest = key_menu;
	m_state = m_search;
	m_entersound = false;
	slistSilent = true;
	slistLocal = false;
	searchComplete = false;
	NET_Slist_f ();
}

void M_Search_Draw (void)
{
	MYPICT	*p;
	int	x;
	p = Draw_CachePic ("gfx/p_multi.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);
	x = (320/2) - ((12*8)/2) + 4;
	M_DrawTextBox (x-8, 32, 12, 1);
	M_Print (x, 40, "Searching...");
	if (slistInProgress)
	{
		NET_Poll ();
		return;
	}
	if (!searchComplete)
	{
		searchComplete = true;
		searchCompleteTime = realtime;
	}
	if (hostCacheCount)
	{
		M_Menu_ServerList_f ();
		return;
	}
	M_PrintWhite ((320/2) - ((22*8)/2), 64, "No Quake servers found");
	if ((realtime - searchCompleteTime) < 3.0)
		return;
	M_Menu_LanConfig_f ();
}

void M_Search_Key (int key, int ascii)
{
}
//=============================================================================
/* SLIST MENU */
int	slist_cursor;
qbool slist_sorted;
void M_Menu_ServerList_f (void)
{
	key_dest = key_menu;
	m_state = m_slist;
	m_entersound = true;
	slist_cursor = 0;
	m_return_onerror = false;
	m_return_reason[0] = 0;
	slist_sorted = false;
}

void M_ServerList_Draw (void)
{
	int		n;
	char	string [64];
	MYPICT	*p;
	if (!slist_sorted)
	{
		if (hostCacheCount > 1)
		{
			int		i, j;
			hostcache_t	temp;
			for (i=0 ; i<hostCacheCount ; i++)
				for (j=i+1 ; j<hostCacheCount ; j++)
					if (strcmp(hostcache[j].name, hostcache[i].name) < 0)
					{
						memcpy (&temp, &hostcache[j], sizeof(hostcache_t));
						memcpy (&hostcache[j], &hostcache[i], sizeof(hostcache_t));
						memcpy (&hostcache[i], &temp, sizeof(hostcache_t));
					}
		}
		slist_sorted = true;
	}
	p = Draw_CachePic ("gfx/p_multi.lmp");
	M_DrawPic ((320-p->width)/2, 4, p);
	for (n=0 ; n<hostCacheCount ; n++)
	{
		if (hostcache[n].maxusers)
			snprintf (string, sizeof(string), "%-15.15s %-15.15s %2u/%2u\n", hostcache[n].name, hostcache[n].map, hostcache[n].users, hostcache[n].maxusers);
		else
			snprintf (string, sizeof(string), "%-15.15s %-15.15s\n", hostcache[n].name, hostcache[n].map);
		M_Print (16, 32 + 8*n, string);
	}
	M_DrawCharacter (0, 32 + slist_cursor*8, 12+((int)(realtime*4)&1));
	if (*m_return_reason)
		M_PrintWhite (16, 148, m_return_reason);
}

void M_ServerList_Key (int key, int ascii)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_LanConfig_f ();
		break;
	case K_SPACE:
		M_Menu_Search_f ();
		break;
	case K_UPARROW:
	case K_LEFTARROW:
		S_LocalSound ("misc/menu1.wav");
		slist_cursor--;
		if (slist_cursor < 0)
			slist_cursor = hostCacheCount - 1;
		break;
	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("misc/menu1.wav");
		slist_cursor++;
		if (slist_cursor >= hostCacheCount)
			slist_cursor = 0;
		break;
	case K_ENTER:
		S_LocalSound ("misc/menu2.wav");
		m_return_state = m_state;
		m_return_onerror = true;
		slist_sorted = false;
		key_dest = key_game;
		m_state = m_none;
		Cbuf_AddText (va ("connect \"%s\"\n", hostcache[slist_cursor].cname));
		break;
	default:
		break;
	}
}
//=============================================================================
/* Menu Subsystem */
void M_Init (void)
{
	Cvar_Registration_Client_Menu ();

	Cmd_AddCommand ("togglemenu", M_ToggleMenu_f);
	Cmd_AddCommand ("menu_main", M_Menu_Main_f);
	Cmd_AddCommand ("menu_singleplayer", M_Menu_SinglePlayer_f);
	Cmd_AddCommand ("menu_load", M_Menu_Load_f);
	Cmd_AddCommand ("menu_save", M_Menu_Save_f);
	Cmd_AddCommand ("menu_multiplayer", M_Menu_MultiPlayer_f);
	Cmd_AddCommand ("menu_setup", M_Menu_Setup_f);
	Cmd_AddCommand ("menu_namemaker", M_Menu_NameMaker_f);
//	Cmd_AddCommand ("menu_gameatron", M_Menu_Gameatron_f);
	Cmd_AddCommand ("namemaker", M_Shortcut_NameMaker_f);
//	Cmd_AddCommand ("gameatron", M_Shortcut_Gameatron_f);
	Cmd_AddCommand ("menu_options", M_Menu_Options_f);
	Cmd_AddCommand ("menu_keys", M_Menu_Keys_f);
	Cmd_AddCommand ("menu_preferences", M_Menu_Preferences_f);
	Cmd_AddCommand ("menu_videomodes", M_Menu_VideoModes_f);
#ifdef SUPPORTS_QMB
	Cmd_AddCommand ("menu_videoeffects", M_Menu_VideoEffects_f);
	Cmd_AddCommand ("menu_particles", M_Menu_Particles_f);
#endif
	Cmd_AddCommand ("help", M_Menu_Help_f);
#ifdef SUPPORTS_EXTENDED_MENUS
	Cmd_AddCommand ("menu_maps", M_Menu_Maps_f);
	Cmd_AddCommand ("menu_demos", M_Menu_Demos_f);
#endif
	Cmd_AddCommand ("menu_quit", M_Menu_Quit_f);
}

void M_Draw (void)
{
	if (m_state == m_none || key_dest != key_menu)
		return;
	if (!m_recursiveDraw)
	{
//		scr_copyeverything = 1;
		if (scr_con_current == vid.height)
		{
			Draw_ConsoleBackground (scr_con_current); // joe: was vid.height
//			VID_UnlockBuffer ();
			S_ExtraUpdate ();
//			VID_LockBuffer ();
		}
		else
		{
			Draw_FadeScreen ();
		}
//		scr_fullupdate = 0;
	}
	else
	{
		m_recursiveDraw = false;
	}
#ifdef SUPPORTS_CENTERED_MENUS
	// Baker: this is actually bad since the quit screen calls this
	//        so it restores the canvas when it shouldn't.
	//        And the centered menu stuff calling eglOrtho should be
	//        gl_draw or something anyway.  Not here.

	// Centered Canvas ON
	if (scr_menu_scale.integer)
	{
		menuwidth = 320;
		menuheight = min(vid.height, 240);
//		if (m_state == m_gameatron)
//		{
//			// We want a larger canvas; double precision
//			menuwidth *=2;
//			menuheight *=2;
//		}
		eglMatrixMode (GL_PROJECTION);
		eglLoadIdentity ();
		eglOrtho (0, menuwidth, menuheight, 0, -99999, 99999);
	}
	else
	{
		menuwidth = vid.width;
		menuheight = vid.height;
	}
	m_yofs = scr_menu_center.integer ? (menuheight - 200) / 2 : 0;
#endif

	switch (m_state)
	{
	case m_none:
		break;
	case m_main:
		M_Main_Draw ();
		break;
	case m_singleplayer:
		M_SinglePlayer_Draw ();
		break;
	case m_load:
		M_Load_Draw ();
		break;
	case m_save:
		M_Save_Draw ();
		break;
	case m_multiplayer:
		M_MultiPlayer_Draw ();
		break;
	case m_setup:
		M_Setup_Draw ();
		break;
	case m_namemaker:
		M_NameMaker_Draw ();
		break;
//	case m_gameatron:
//		M_Gameatron_Draw ();
//		break;
	case m_net:
		M_Net_Draw ();
		break;
	case m_options:
		M_Options_Draw ();
		break;
	case m_keys:
		M_Keys_Draw ();
		break;
#ifdef SUPPORTS_QMB
	case m_videoeffects:
		M_VideoEffects_Draw ();
		break;
	case m_particles:
		M_Particles_Draw ();
		break;
#endif
	case m_videomodes:
		M_VideoModes_Draw ();
		break;
#ifdef SUPPORTS_NEHAHRA
	case m_nehdemos:
		M_NehDemos_Draw ();
		break;
	case m_maps:
		M_Maps_Draw ();
		break;
#endif
#ifdef SUPPORTS_EXTENDED_MENUS
	case m_demos:
		M_Demos_Draw ();
		break;
#endif
	case m_help:
		M_Help_Draw ();
		break;
	case m_quit:
		M_Quit_Draw ();
		break;
	case m_serialconfig:
		M_SerialConfig_Draw ();
		break;
	case m_modemconfig:
		M_ModemConfig_Draw ();
		break;
	case m_lanconfig:
		M_LanConfig_Draw ();
		break;
	case m_gameoptions:
		M_GameOptions_Draw ();
		break;
	case m_search:
		M_Search_Draw ();
		break;
	case m_slist:
		M_ServerList_Draw ();
		break;
	case m_preferences:
		M_Pref_Options_Draw ();
		break;

	}
#ifdef SUPPORTS_CENTERED_MENUS
	// Centered Canvas OFF
	if (scr_menu_scale.integer)
	{
		eglMatrixMode (GL_PROJECTION);
		eglLoadIdentity ();
		eglOrtho (0, vid.width, vid.height, 0, -99999, 99999);
	}
#endif
	if (m_entersound)
	{
		S_LocalSound ("misc/menu2.wav");
		m_entersound = false;
	}
//	VID_UnlockBuffer ();
	S_ExtraUpdate ();
//	VID_LockBuffer ();
}

void M_Keydown (int key, int ascii, qbool down)
{
	if (key == K_MOUSECLICK_BUTTON1)
		if (m_state != m_namemaker) // K_MOUSECLICK is only valid for namemaker
			return;

	switch (m_state)
	{
	case m_none:
		return;
	case m_main:
		M_Main_Key (key, ascii);
		return;
	case m_singleplayer:
		M_SinglePlayer_Key (key, ascii);
		return;
	case m_load:
		M_Load_Key (key, ascii);
		return;
	case m_save:
		M_Save_Key (key, ascii);
		return;
	case m_multiplayer:
		M_MultiPlayer_Key (key, ascii);
		return;
	case m_setup:
		M_Setup_Key (key, ascii);
		return;
	case m_namemaker:
		M_NameMaker_Key (key, ascii);
		return;
//	case m_gameatron:
//
//		M_Gameatron_Key (key, ascii);
//		return;

	case m_net:
		M_Net_Key (key, ascii);
		return;
	case m_options:
		M_Options_Key (key, ascii);
		return;
	case m_keys:
		M_Keys_Key (key, ascii, down);
		return;
#ifdef SUPPORTS_QMB
	case m_videoeffects:
		M_VideoEffects_Key (key, ascii);
		return;
	case m_particles:
		M_Particles_Key (key, ascii);
		return;
#endif
	case m_videomodes:
		M_VideoModes_Key (key, ascii);
		return;
#ifdef SUPPORTS_NEHAHRA
	case m_nehdemos:
		M_NehDemos_Key (key, ascii);
		return;
#endif
#ifdef SUPPORTS_EXTENDED_MENUS
	case m_maps:
		M_Maps_Key (key, ascii);
		return;
	case m_demos:
		M_Demos_Key (key, ascii);
		return;
#endif
	case m_help:
		M_Help_Key (key, ascii);
		return;
	case m_quit:
		M_Quit_Key (key, ascii);
		return;
	case m_serialconfig:
		M_SerialConfig_Key (key, ascii);
		return;
	case m_modemconfig:
		M_ModemConfig_Key (key, ascii);
		return;
	case m_lanconfig:
		M_LanConfig_Key (key, ascii);
		return;
	case m_gameoptions:
		M_GameOptions_Key (key, ascii);
		return;
	case m_search:
		M_Search_Key (key, ascii);
		break;
	case m_slist:
		M_ServerList_Key (key, ascii);
		return;

	case m_preferences:
		M_Pref_Options_Key (key, ascii);
		return;

	}
}

void M_ConfigureNetSubsystem (void)
{
// enable/disable net systems to match desired config
	Cbuf_AddText ("stopdemo\n");
	if (SerialConfig || DirectConfig)
	{
		Cbuf_AddText ("com1 enable\n");
	}
	if (IPXConfig || TCPIPConfig)
		net_hostport = lanConfig_port;
}
//=============================================================================
/* COMMON STUFF FOR MAPS AND DEMOS MENUS */
// NOTE: 320x200 res can only handle no more than 17 lines +2 for file
// searching. In GL I use 1 more line, though 320x200 is also available
// under GL too, but I force _nobody_ using that, but 320x240 instead!
#ifndef GLQUAKE
#define	MAXLINES	17	// maximum number of files visible on screen
#else
#define	MAXLINES	18
#endif
char	demodir[MAX_QPATH] = "";
char	prevdir[MAX_QPATH] = "";
char	searchfile[MAX_FILELENGTH] = "";
static	int	demo_cursor = 0, demo_base = 0, globctr = 0;
static qbool	searchbox = false;
void PrintSortedFiles (void)
{
	int	i;
	demo_base = demo_cursor = 0;
	// TODO: position demo cursor
	if (prevdir)
	{
		for (i=0 ; i<num_files ; i++)
		{
			if (COM_StringMatch (filelist[i].name, prevdir))
			{
				demo_cursor = i;
				if (demo_cursor >= MAXLINES)
				{
					demo_base += demo_cursor - (MAXLINES-1);
					demo_cursor = MAXLINES-1;
				}
				*prevdir = 0;
			}
		}
	}
}
static char *toYellow (char *s)
{
	static	char	buf[20];
	strlcpy (buf, s, sizeof(buf));
	for (s = buf ; *s ; s++)
		if (*s >= '0' && *s <= '9')
			*s = *s - '0' + 18;
	return buf;
}
extern	int	key_insert;
void M_Files_Draw (char *title)
{
	int		i, y;
	direntry_t	*d;
	char		str[29];
	M_Print (140, 8, title);
	M_Print (8, 24, "\x1d\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1f \x1d\x1e\x1e\x1e\x1e\x1e\x1f");
	d = filelist + demo_base;
	for (i = 0, y = 32 ; i < num_files - demo_base && i < MAXLINES ; i++, y += 8, d++)
	{
		StringLCopy (str, d->name);
		if (d->type)
			M_PrintWhite (24, y, str);
		else
			M_Print (24, y, str);
		if (d->type == 1)
			M_PrintWhite (256, y, "folder");
		else if (d->type == 2)
			M_PrintWhite (256, y, "  up  ");
		else if (d->type == 0)
			M_Print (256, y, toYellow(va("%5ik", d->size >> 10)));
	}
	M_DrawCharacter (8, 32 + demo_cursor*8, 12+((int)(realtime*4)&1));
	if (searchbox)
	{
		M_PrintWhite (24, 48 + 8*MAXLINES, "search: ");
		M_DrawTextBox (80, 40 + 8*MAXLINES, 16, 1);
		M_PrintWhite (88, 48 + 8*MAXLINES, searchfile);
		M_DrawCharacter (88 + 8*strlen(searchfile), 48 + 8*MAXLINES, ((int)(realtime*4)&1) ? 11+(84*key_insert) : 10);
	}
}
static void KillSearchBox (void)
{
	searchbox = false;
	memset (searchfile, 0, sizeof(searchfile));
	globctr = 0;
}
void M_Files_Key (int k)
{
	int		i;
	qbool	worx;
	switch (k)
	{
	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (demo_cursor > 0)
			demo_cursor--;
		else if (demo_base > 0)
			demo_base--;
		break;
	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (demo_cursor+demo_base < num_files-1)
		{
			if (demo_cursor < MAXLINES-1)
				demo_cursor++;
			else
				demo_base++;
		}
		break;
	case K_HOME:
		S_LocalSound ("misc/menu1.wav");
		demo_cursor = 0;
		demo_base = 0;
		break;
	case K_END:
		S_LocalSound ("misc/menu1.wav");
		if (num_files > MAXLINES)
		{
			demo_cursor = MAXLINES-1;
			demo_base = num_files - demo_cursor - 1;
		}
		else
		{
			demo_base = 0;
			demo_cursor = num_files-1;
		}
		break;
	case K_PGUP:
		S_LocalSound ("misc/menu1.wav");
		demo_cursor -= MAXLINES-1;
		if (demo_cursor < 0)
		{
			demo_base += demo_cursor;
			if (demo_base < 0)
				demo_base = 0;
			demo_cursor = 0;
		}
		break;
	case K_PGDN:
		S_LocalSound ("misc/menu1.wav");
		demo_cursor += MAXLINES-1;
		if (demo_base + demo_cursor >= num_files)
			demo_cursor = num_files - demo_base - 1;
		if (demo_cursor >= MAXLINES)
		{
			demo_base += demo_cursor - (MAXLINES-1);
			demo_cursor = MAXLINES-1;
			if (demo_base + demo_cursor >= num_files)
				demo_base = num_files - demo_cursor - 1;
		}
		break;
	case K_BACKSPACE:
		if (strcmp(searchfile, ""))
			searchfile[--globctr] = 0;
		break;
	default:
		if (k < 32 || k > 127)
			break;
		searchbox = true;
		searchfile[globctr++] = k;
		worx = false;
		for (i=0 ; i<num_files ; i++)
		{
			if (strstr(filelist[i].name, searchfile) == filelist[i].name)
			{
				worx = true;
				S_LocalSound ("misc/menu1.wav");
				demo_base = i - 10;
				if (demo_base < 0)
				{
					demo_base = 0;
					demo_cursor = i;
				}
				else if (demo_base > (num_files - MAXLINES))
				{
					demo_base = num_files - MAXLINES;
					demo_cursor = MAXLINES - (num_files - i);
				}
				else
					demo_cursor = 10;
				break;
			}
		}
		if (!worx)
			searchfile[--globctr] = 0;
		break;
	}
}
//=============================================================================
/* MAPS MENU */
void PrintSortedMaps (void)
{
	searchpath_t	*search;
	extern	void EraseDirEntries (void);
	EraseDirEntries ();
	pak_files = 0;
	for (search = com_searchpaths ; search ; search = search->next)
	{
		if (!search->pack)
		{
			RDFlags |= (RD_STRIPEXT | RD_NOERASE);
			ReadDir (va("%s/maps", search->filename), "*.bsp");
		}
	}
	QFS_FindFilesInPak ("maps/*.bsp", 0 /*compl_len*/, NULL );
	PrintSortedFiles ();
}
void M_Menu_Maps_f (void)
{
	key_dest = key_menu;
	m_state = m_maps;
	m_entersound = true;
	PrintSortedMaps ();
}
void M_Maps_Draw (void)
{
	M_Files_Draw ("MAPS");
}
void M_Maps_Key (int key, int ascii)
{
	switch (key)
	{
	case K_ESCAPE:
		if (searchbox)
			KillSearchBox ();
		else
			M_Menu_Main_f ();
		return;
	case K_ENTER:
		if (!num_files || filelist[demo_cursor+demo_base].type == 3)
			return;
		key_dest = key_game;
		m_state = m_none;
		Cbuf_AddText (va("map %s\n", filelist[demo_cursor+demo_base].name));
		StringLCopy (prevdir, filelist[demo_cursor+demo_base].name);
		if (searchbox)
			KillSearchBox ();
		return;
	}
	M_Files_Key (key);
}

//=============================================================================
/* DEMOS MENU */
// Nehahra's Demos Menu
#define	MAXNEHLINES	20
typedef struct
{
	char	name[50];
	char	desc[50];
} demonames_t;
demonames_t	NehDemos[35];
static	int	num_nehdemos, nehdemo_cursor = 0, nehdemo_base = 0;
void M_Menu_NehDemos_f (void)
{
	key_dest = key_menu;
	m_state = m_nehdemos;
	m_entersound = true;
	num_nehdemos = 34;
	strcpy (NehDemos[0].name,  "INTRO");	strcpy (NehDemos[0].desc,  "Prologue");
	strcpy (NehDemos[1].name,  "GENF");	strcpy (NehDemos[1].desc,  "The Beginning");
	strcpy (NehDemos[2].name,  "GENLAB");	strcpy (NehDemos[2].desc,  "A Doomed Project");
	strcpy (NehDemos[3].name,  "NEHCRE");	strcpy (NehDemos[3].desc,  "The New Recruits");
	strcpy (NehDemos[4].name,  "MAXNEH");	strcpy (NehDemos[4].desc,  "Breakthrough");
	strcpy (NehDemos[5].name,  "MAXCHAR");	strcpy (NehDemos[5].desc,  "Renewal and Duty");
	strcpy (NehDemos[6].name,  "CRISIS");	strcpy (NehDemos[6].desc,  "Worlds Collide");
	strcpy (NehDemos[7].name,  "POSTCRIS");	strcpy (NehDemos[7].desc,  "Darkening Skies");
	strcpy (NehDemos[8].name,  "HEARING");	strcpy (NehDemos[8].desc,  "The Hearing");
	strcpy (NehDemos[9].name,  "GETJACK");	strcpy (NehDemos[9].desc,  "On a Mexican Radio");
	strcpy (NehDemos[10].name, "PRELUDE");	strcpy (NehDemos[10].desc, "Honor and Justice");
	strcpy (NehDemos[11].name, "ABASE");	strcpy (NehDemos[11].desc, "A Message Sent");
	strcpy (NehDemos[12].name, "EFFECT");	strcpy (NehDemos[12].desc, "The Other Side");
	strcpy (NehDemos[13].name, "UHOH");	strcpy (NehDemos[13].desc, "Missing in Action");
	strcpy (NehDemos[14].name, "PREPARE");	strcpy (NehDemos[14].desc, "The Response");
	strcpy (NehDemos[15].name, "VISION");	strcpy (NehDemos[15].desc, "Farsighted Eyes");
	strcpy (NehDemos[16].name, "MAXTURNS");	strcpy (NehDemos[16].desc, "Enter the Immortal");
	strcpy (NehDemos[17].name, "BACKLOT");	strcpy (NehDemos[17].desc, "Separate Ways");
	strcpy (NehDemos[18].name, "MAXSIDE");	strcpy (NehDemos[18].desc, "The Ancient Runes");
	strcpy (NehDemos[19].name, "COUNTER");	strcpy (NehDemos[19].desc, "The New Initiative");
	strcpy (NehDemos[20].name, "WARPREP");	strcpy (NehDemos[20].desc, "Ghosts to the World");
	strcpy (NehDemos[21].name, "COUNTER1");	strcpy (NehDemos[21].desc, "A Fate Worse Than Death");
	strcpy (NehDemos[22].name, "COUNTER2");	strcpy (NehDemos[22].desc, "Friendly Fire");
	strcpy (NehDemos[23].name, "COUNTER3");	strcpy (NehDemos[23].desc, "Minor Setback");
	strcpy (NehDemos[24].name, "MADMAX");	strcpy (NehDemos[24].desc, "Scores to Settle");
	strcpy (NehDemos[25].name, "QUAKE");	strcpy (NehDemos[25].desc, "One Man");
	strcpy (NehDemos[26].name, "CTHMM");	strcpy (NehDemos[26].desc, "Shattered Masks");
	strcpy (NehDemos[27].name, "SHADES");	strcpy (NehDemos[27].desc, "Deal with the Dead");
	strcpy (NehDemos[28].name, "GOPHIL");	strcpy (NehDemos[28].desc, "An Unlikely Hero");
	strcpy (NehDemos[29].name, "CSTRIKE");	strcpy (NehDemos[29].desc, "War in Hell");
	strcpy (NehDemos[30].name, "SHUBSET");	strcpy (NehDemos[30].desc, "The Conspiracy");
	strcpy (NehDemos[31].name, "SHUBDIE");	strcpy (NehDemos[31].desc, "Even Death May Die");
	strcpy (NehDemos[32].name, "NEWRANKS");	strcpy (NehDemos[32].desc, "An Empty Throne");
	strcpy (NehDemos[33].name, "SEAL");	strcpy (NehDemos[33].desc, "The Seal is Broken");
}
void M_NehDemos_Draw (void)
{
	int		i, y;
	demonames_t	*d;
	M_Print (140, 8, "DEMOS");
	M_Print (8, 24, "\x1d\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1f");
	d = NehDemos + nehdemo_base;
	for (i = 0, y = 32 ; i < num_nehdemos - nehdemo_base && i < MAXNEHLINES ; i++, y += 8, d++)
		M_Print (24, y, d->desc);
// line cursor
	M_DrawCharacter (8, 32 + nehdemo_cursor*8, 12+((int)(realtime*4)&1));
}
void M_NehDemos_Key (int key, int ascii)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Main_f ();
		break;
	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (nehdemo_cursor > 0)
			nehdemo_cursor--;
		else if (nehdemo_base > 0)
			nehdemo_base--;
		break;
	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (nehdemo_cursor+nehdemo_base < num_nehdemos-1)
		{
			if (nehdemo_cursor < MAXNEHLINES-1)
				nehdemo_cursor++;
			else
				nehdemo_base++;
		}
		break;
	case K_HOME:
		S_LocalSound ("misc/menu1.wav");
		nehdemo_cursor = 0;
		nehdemo_base = 0;
		break;
	case K_END:
		S_LocalSound ("misc/menu1.wav");
		if (num_nehdemos > MAXNEHLINES)
		{
			nehdemo_cursor = MAXNEHLINES-1;
			nehdemo_base = num_nehdemos - nehdemo_cursor - 1;
		}
		else
		{
			nehdemo_base = 0;
			nehdemo_cursor = num_nehdemos-1;
		}
		break;
	case K_PGUP:
		S_LocalSound ("misc/menu1.wav");
		nehdemo_cursor -= MAXNEHLINES-1;
		if (nehdemo_cursor < 0)
		{
			nehdemo_base += nehdemo_cursor;
			if (nehdemo_base < 0)
				nehdemo_base = 0;
			nehdemo_cursor = 0;
		}
		break;
	case K_PGDN:
		S_LocalSound ("misc/menu1.wav");
		nehdemo_cursor += MAXNEHLINES-1;
		if (nehdemo_base + nehdemo_cursor >= num_nehdemos)
			nehdemo_cursor = num_nehdemos - nehdemo_base - 1;
		if (nehdemo_cursor >= MAXNEHLINES)
		{
			nehdemo_base += nehdemo_cursor - (MAXNEHLINES-1);
			nehdemo_cursor = MAXNEHLINES-1;
			if (nehdemo_base + nehdemo_cursor >= num_nehdemos)
				nehdemo_base = num_nehdemos - nehdemo_cursor - 1;
		}
		break;
	case K_ENTER:
		S_LocalSound ("misc/menu2.wav");
		m_state = m_none;
		key_dest = key_game;
		Con_DevPrintf (DEV_PROTOCOL, "Nehahra Demos: Begin loading plaque\n");
		SCR_BeginLoadingPlaque ();
		Cbuf_AddText (va("playdemo %s\n", NehDemos[nehdemo_base+nehdemo_cursor].name));
		break;
 	}
}
// Engine X's Demos Menu
void PrintSortedDemos (void)
{
	extern	char	com_basedir[MAX_OSPATH];
	RDFlags |= RD_MENU_DEMOS;
	if (!demodir[0])
	{
		RDFlags |= RD_MENU_DEMOS_MAIN;
		ReadDir (com_basedir, "*");
	}
	else
	{
		int test = Sys_FileTime(va("%s%s", com_basedir, demodir));
		if(test == -1) // Could not get
			COM_CreatePath (va("%s%s/", com_basedir, demodir));
		ReadDir (va("%s%s", com_basedir, demodir), "*");  // Baker: old ... wait, let's trust this for now
		//ReadDir (va("%s", com_demodirfull), "*");  // Baker: new ... what if this is being redefined?
	}
	PrintSortedFiles ();
}
void M_Menu_Demos_f (void)
{
	key_dest = key_menu;
	m_state = m_demos;
	m_entersound = true;
	PrintSortedDemos ();
}
void M_Demos_Draw (void)
{
	M_Print (16, 16, demodir);
	M_Files_Draw ("DEMOS");
}
void M_Demos_Key (int key, int ascii)
{
	switch (key)
	{
	case K_ESCAPE:
		if (searchbox)
		{
			KillSearchBox ();
		}
		else
		{
			StringLCopy (prevdir, filelist[demo_cursor+demo_base].name);
			M_Menu_Main_f ();
		}
		return;
	case K_ENTER:
		if (!num_files || filelist[demo_cursor+demo_base].type == 3)
			return;
		if (filelist[demo_cursor+demo_base].type)
		{
			if (filelist[demo_cursor+demo_base].type == 2)
			{
				char	*p;
				if ((p = strrchr(demodir, '/')))
				{
					//strlcpy (prevdir, p + 1, sizeof(prevdir));
					StringLCopy (prevdir, p + 1);
					*p = 0;
				}
			}
			else
			{
				strncat (demodir, va("/%s", filelist[demo_cursor+demo_base].name), sizeof(demodir)-1);
			}
			PrintSortedDemos ();
		}
		else
		{
			key_dest = key_game;
			m_state = m_none;
			Cbuf_AddText (va("playdemo \"..%s/%s\"\n", demodir, filelist[demo_cursor+demo_base].name));
			StringLCopy (prevdir, filelist[demo_cursor+demo_base].name);
		}
		if (searchbox)
			KillSearchBox ();
		return;
	}
	M_Files_Key (key);
}
//=============================================================================
/* VIDEO OPTIONS MENU */	// joe
#ifdef GLQUAKE
#define	VOM_ITEMS	16
int	vom_cursor = 0;
char *popular_filters[] = {
	"GL_NEAREST",
	"GL_LINEAR_MIPMAP_NEAREST",
	"GL_LINEAR_MIPMAP_LINEAR"
};
//extern	cvar_t	gl_texturemode;
extern	qbool particle_mode;
void R_SetParticles (int val);
void M_Menu_VideoEffects_f (void)
{
	key_dest = key_menu;
	m_state = m_videoeffects;
	m_entersound = true;
	CheckParticles ();
}
void M_AdjustVOMSliders (int dir)
{
	S_LocalSound ("misc/menu3.wav");
	switch (vom_cursor)
	{
	case 4:
		Cvar_SetFloatByRef (&r_shadows, CLAMP (0, r_shadows.floater + dir * 0.1, 1));
		break;
	case 7:
		Cvar_SetFloatByRef (&tex_picmip, CLAMP (0, tex_picmip.integer - dir, 4));
		break;
	case 11:
		Cvar_SetFloatByRef (&r_water_fog_density, CLAMP (0, r_water_fog_density.floater + dir * 0.1, 1));
		break;
	case 12:
		Cvar_SetFloatByRef (&r_water_alpha, CLAMP (0, r_water_alpha.floater + dir * 0.1, 1));
		break;
	}
}
void M_VideoEffects_Draw (void)
{
	float	r;
	mpic_t	*p;
	M_DrawTransPic (16, 4, Draw_CachePic("gfx/qplaque.lmp"));
	p = Draw_CachePic ("gfx/ttl_cstm.lmp");
	M_DrawPic ((320-p->width)/2, 4, p);
	M_Print (16, 32, "       Coloured lights");
	M_DrawCheckbox (220, 32, external_lits.integer);
	M_Print (16, 40, "        Dynamic lights");
	M_DrawCheckbox (220, 40, scene_dynamiclight.integer);
	M_Print (16, 48, "       Vertex lighting");
	M_DrawCheckbox (220, 48, r_vertex_lights.integer);
	M_Print (16, 56, "     Smooth animations");
	M_DrawCheckbox (220, 56, scene_lerpmodels.integer);
	M_Print (16, 64, "               Shadows");
	r = r_shadows.floater;
	M_DrawSlider (220, 64, r);
	M_Print (16, 72, "        Particle style");
	M_Print (220, 72, !particle_mode ? "classic" : particle_mode == 1 ? "qmb" : "mixed");
	M_Print (16, 80, "        Texture filter");
	M_Print (220, 80, COM_StringMatchCaseless (tex_scene_texfilter.string, "GL_LINEAR_MIPMAP_NEAREST") ? "bilinear" :
			COM_StringMatchCaseless (tex_scene_texfilter.string, "GL_LINEAR_MIPMAP_LINEAR") ? "trilinear" :
			COM_StringMatchCaseless (tex_scene_texfilter.string, "GL_NEAREST") ? "off" : tex_scene_texfilter.string);
	M_Print (16, 88, "       Texture quality");
	r = (4 - tex_picmip.integer) * 0.25;
	M_DrawSlider (220, 88, r);
	M_Print (16, 96, "        Detail texture");
	M_DrawCheckbox (220, 96, r_pass_detail.integer);
	M_Print (16, 104, "        Water caustics");
	M_DrawCheckbox (220, 104, r_pass_caustics.integer);
	M_Print (16, 112, "        Underwater fog");
	M_Print (220, 112, !r_water_fog.integer ? "off" : r_water_fog.integer == 2 ? "extra" : "normal");
	M_Print (16, 120, "      Waterfog density");
	r = r_water_fog_density.floater;
	M_DrawSlider (220, 120, r);
	M_Print (16, 128, "           Water alpha");
	r = r_water_alpha.floater;
	M_DrawSlider (220, 128, r);
	M_Print (16, 136, "             Particles");
	M_PrintWhite (16, 144, "             Fast mode");
	M_PrintWhite (16, 152, "          High quality");
	// cursor
	M_DrawCharacter (200, 32 + vom_cursor*8, 12+((int)(realtime*4)&1));
}
void M_VideoEffects_Key (int key, int ascii)
{
	int	i;
	extern	void R_ToggleParticles (void);
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Options_f ();
		break;
	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		vom_cursor--;
		if (vom_cursor < 0)
			vom_cursor = VOM_ITEMS - 1;
		break;
	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		vom_cursor++;
		if (vom_cursor >= VOM_ITEMS)
			vom_cursor = 0;
		break;
	case K_HOME:
	case K_PGUP:
		S_LocalSound ("misc/menu1.wav");
		vom_cursor = 0;
		break;
	case K_END:
	case K_PGDN:
		S_LocalSound ("misc/menu1.wav");
		vom_cursor = VOM_ITEMS - 1;
		break;
	case K_LEFTARROW:
	case K_RIGHTARROW:
	case K_ENTER:
		S_LocalSound ("misc/menu2.wav");
		switch (vom_cursor)
		{
		case 0:
			Cvar_SetFloatByRef (&external_lits, !external_lits.integer);
			break;
		case 1:
			Cvar_SetFloatByRef (&scene_dynamiclight, !scene_dynamiclight.integer);
			break;
		case 2:
			Cvar_SetFloatByRef (&r_vertex_lights, !r_vertex_lights.integer);
			break;
		case 3:
			Cvar_SetFloatByRef (&scene_lerpmodels, !scene_lerpmodels.integer);
			break;
		case 5:
			R_SetParticles (!particle_mode);
			break;
		case 6:
			for (i=0 ; i<3 ; i++)
				if (COM_StringMatchCaseless (popular_filters[i], tex_scene_texfilter.string))
					break;
			if (i >= 2)
				i = -1;
			Cvar_SetStringByRef (&tex_scene_texfilter, popular_filters[i+1]);
			break;
		case 8:
			Cvar_SetFloatByRef (&r_pass_detail, !r_pass_detail.integer);
			break;
		case 9:
			Cvar_SetFloatByRef (&r_pass_caustics, !r_pass_caustics.integer);
			break;
		case 10:
			Cvar_SetFloatByRef (&r_water_fog, !r_water_fog.integer ? 1 : r_water_fog.integer == 1 ? 2 : 0);
			break;
		case 13:
			M_Menu_Particles_f ();
			break;
		case 14:
			Cvar_SetFloatByRef (&external_lits, 0);
			Cvar_SetFloatByRef (&scene_dynamiclight, 0);
			Cvar_SetFloatByRef (&r_vertex_lights, 0);
			Cvar_SetFloatByRef (&r_shadows, 0);
			if (particle_mode)
				R_SetParticles (0);
			Cvar_SetStringByRef (&tex_scene_texfilter, "GL_LINEAR_MIPMAP_NEAREST");
			Cvar_SetFloatByRef (&tex_picmip, 3);
			Cvar_SetFloatByRef (&r_pass_detail, 0);
			Cvar_SetFloatByRef (&r_pass_caustics, 0);
			Cvar_SetFloatByRef (&r_water_fog, 0);
			Cvar_SetFloatByRef (&scene_farclip, 4096);
			//Cvar_SetStringByRef ("r_wateralpha", "1");
			break;
		case 15:
			Cvar_SetFloatByRef (&external_lits, 1);
			Cvar_SetFloatByRef (&scene_dynamiclight, 1);
			Cvar_SetFloatByRef (&r_vertex_lights, 1);
			Cvar_SetFloatByRef (&r_shadows, 0.4);
			if (!particle_mode)
				R_SetParticles (1);
			Cvar_SetStringByRef (&tex_scene_texfilter, "GL_LINEAR_MIPMAP_LINEAR");
			Cvar_SetFloatByRef (&tex_picmip, 0);
			Cvar_SetFloatByRef (&r_pass_detail, 1);
			Cvar_SetFloatByRef (&r_pass_caustics, 1);
			Cvar_SetFloatByRef (&r_water_fog, 1);
			Cvar_SetFloatByRef (&scene_farclip, 16384); // Baker: high quality should mean no distance weirdness on big single player maps
			//Cvar_SetStringByRef ("r_wateralpha", "0.4");
			break;
		default:
			if (key == K_LEFTARROW)
				M_AdjustVOMSliders (-1);
			else
			M_AdjustVOMSliders (1);
			break;
		}
	}
}
//=============================================================================
/* PARTICLES MENU */
#define	PART_ITEMS	15
int	part_cursor = 0;
void M_Menu_Particles_f (void)
{
	key_dest = key_menu;
	m_state = m_particles;
	m_entersound = true;
}
void M_Particles_Draw (void)
{
	mpic_t	*p;
	M_DrawTransPic (16, 4, Draw_CachePic("gfx/qplaque.lmp"));
	p = Draw_CachePic ("gfx/ttl_cstm.lmp");
	M_DrawPic ((320-p->width)/2, 4, p);
	M_Print (16, 32, "            Explosions");
	M_Print (220, 32, !qmb_explosions.integer ? "classic" : "qmb");
	M_Print (16, 40, "                Trails");
	M_Print (220, 40, !qmb_trails.integer ? "classic" : "qmb");
	M_Print (16, 48, "                Spikes");
	M_Print (220, 48, !qmb_spikes.integer ? "classic" : "qmb");
	M_Print (16, 56, "              Gunshots");
	M_Print (220, 56, !qmb_gunshots.integer ? "classic" : "qmb");
	M_Print (16, 64, "                 Blood");
	M_Print (220, 64, !qmb_blood.integer ? "classic" : "qmb");
	M_Print (16, 72, "     Teleport splashes");
	M_Print (220, 72, !qmb_telesplash.integer ? "classic" : "qmb");
	M_Print (16, 80, "      Spawn explosions");
	M_Print (220, 80, !qmb_blobexplosions.integer ? "classic" : "qmb");
	M_Print (16, 88, "         Lava splashes");
	M_Print (220, 88, !qmb_lavasplash.integer ? "classic" : "qmb");
	M_Print (16, 96, "               Inferno");
	M_Print (220, 96, !qmb_inferno.integer ? "classic" : "qmb");
	M_Print (16, 104, "                Flames");
	M_Print (220, 104, !qmb_flames.integer ? "classic" : "qmb");
	M_Print (16, 112, "             Lightning");
	M_Print (220, 112, !qmb_lightning.integer ? "classic" : "qmb");
	M_Print (16, 120, "   Spike bubble-trails");
	M_Print (220, 120, !qmb_spiketrails.integer ? "classic" : "qmb");
	M_Print (16, 128, "    Bouncing particles");
	M_DrawCheckbox (220, 128, qmb_bounceparticles.integer);
	M_Print (16, 136, "    Clipping Particles");
	M_DrawCheckbox (220, 136, qmb_clipparticles.integer);
	// cursor
	M_DrawCharacter (200, 32 + part_cursor*8, 12+((int)(realtime*4)&1));
}
void M_Particles_Key (int key, int ascii)
{
	switch (key)
	{
	case K_ESCAPE:
		CheckParticles ();
		M_Menu_VideoEffects_f ();
		break;
	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		part_cursor--;
		if (part_cursor < 0)
			part_cursor = PART_ITEMS - 1;
		break;
	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		part_cursor++;
		if (part_cursor >= PART_ITEMS)
			part_cursor = 0;
		break;
	case K_HOME:
	case K_PGUP:
		S_LocalSound ("misc/menu1.wav");
		part_cursor = 0;
		break;
	case K_END:
	case K_PGDN:
		S_LocalSound ("misc/menu1.wav");
		part_cursor = PART_ITEMS - 1;
		break;
	case K_LEFTARROW:
	case K_RIGHTARROW:
	case K_ENTER:
		S_LocalSound ("misc/menu2.wav");
		switch (part_cursor)
		{
		case 0:
			Cvar_SetFloatByRef (&qmb_explosions, !qmb_explosions.integer);
			break;
		case 1:
			Cvar_SetFloatByRef (&qmb_trails, !qmb_trails.integer);
			break;
		case 2:
			Cvar_SetFloatByRef (&qmb_spikes, !qmb_spikes.integer);
			break;
		case 3:
			Cvar_SetFloatByRef (&qmb_gunshots, !qmb_gunshots.integer);
			break;
		case 4:
			Cvar_SetFloatByRef (&qmb_blood, !qmb_blood.integer);
			break;
		case 5:
			Cvar_SetFloatByRef (&qmb_telesplash, !qmb_telesplash.integer);
			break;
		case 6:
			Cvar_SetFloatByRef (&qmb_blobexplosions, !qmb_blobexplosions.integer);
			break;
		case 7:
			Cvar_SetFloatByRef (&qmb_lavasplash, !qmb_lavasplash.integer);
			break;
		case 8:
			Cvar_SetFloatByRef (&qmb_inferno, !qmb_inferno.integer);
			break;
		case 9:
			Cvar_SetFloatByRef (&qmb_flames, !qmb_flames.integer);
			break;
		case 10:
			Cvar_SetFloatByRef (&qmb_lightning, !qmb_lightning.integer);
			break;
		case 11:
			Cvar_SetFloatByRef (&qmb_spiketrails, !qmb_spiketrails.integer);
			break;
		case 12:
			Cvar_SetFloatByRef (&qmb_bounceparticles, !qmb_bounceparticles.integer);
			break;
		case 13:
			Cvar_SetFloatByRef (&qmb_clipparticles, !qmb_clipparticles.integer);
			break;
		}
	}
}
#endif
