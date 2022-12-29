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
// common.h  -- general definitions
// Baker: Validated 6-27-2011.  Boring functionary changes.



#ifndef __COMMON_H__
#define __COMMON_H__

#if !defined BYTE_DEFINED
typedef unsigned char 		byte;
#define BYTE_DEFINED 1
#endif

#undef	true
#undef	false

typedef enum {false, true}	qbool;

#ifndef NULL
#define NULL ((void *)0)
#endif

// Baker: these aren't used on MSVC6 (part of stdlib.h)
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif


//#define CLAMP(a, b, c) ((a) >= (c) ? (a) : (b) < (a) ? (a) : (b) > (c) ? (c) : (b))


typedef struct link_s
{
	struct link_s	*prev, *next;
} link_t;


//============================================================================
// com_byteorder.c

/*
#define Q_MAXCHAR	((char)0x7f)
#define Q_MAXSHORT	((short)0x7fff)
#define Q_MAXINT	((int)0x7fffffff)
#define Q_MAXLONG	((int)0x7fffffff)
#define Q_MAXFLOAT	((int)0x7fffffff)

#define Q_MINCHAR	((char)0x80)
#define Q_MINSHORT	((short)0x8000)
#define Q_MININT 	((int)0x80000000)
#define Q_MINLONG	((int)0x80000000)
#define Q_MINFLOAT	((int)0x7fffffff)
*/

//extern	qbool	bigendien;

extern	short	(*BigShort) 	(short l);
extern	short	(*LittleShort)  (short l);
extern	int		(*BigLong) 		(int l);
extern	int		(*LittleLong) 	(int l);
extern	float	(*BigFloat) 	(float l);
extern	float	(*LittleFloat) 	(float l);

void COM_EndianessCheck (void);

//============================================================================
// com_filesystem.c

// Real file system (i.e., Pak file unaware functions)
void COM_CreatePath 	(const char *absolutepath);
int  COM_FileLength 	(FILE *f);							// Seeks to end and then goes back to beginning, only used for directory tree files (non QFS)
int  COM_FileOpenRead 	(const char *path, FILE **hndl);	// Returns -1 on failure; only used for pak file loading and COM_CopyFile and VCR stuff

#define FS_CREATE_PATH 1
FILE *FS_fopen_write	(const char *filename, const char *mode, qbool bCreatePath);	// Baker: fopen OK with option to create the path if writing a file
FILE *FS_fopen_read		(const char *filename, const char *mode);

char *FS_Open_DirTree_GetName (const char *file_to_find);	// Tries to find file in search directory tree, ignores pak files

//============================================================================
// com_filesystem_closed.c

typedef enum
{
	QFS_SOURCE_INVALID = -1,
	QFS_SOURCE_PAKFILE,
	QFS_SOURCE_DIRTREE,
	QFS_SOURCE_MEMORY,
	QFS_SOURCE_FUNCTION
} qfs_source_type_t;

typedef struct
{
	char	searchpath[MAX_OSPATH];
} qfs_loadinfo_t;

typedef struct
{
	qfs_source_type_t	datasrc;
	int					offset;
	int					filelen;
	char				datapath [MAX_OSPATH];
	char				datapath_explicit [MAX_OSPATH];
} qfs_lastloaded_file_t;

extern qfs_lastloaded_file_t	qfs_lastload;

// Quake file system (pak file aware functions)

qbool QFS_FindFile_Pak_And_Path (const char *filename, const char *media_owner_path);
qbool QFS_FOpenFile 			(const char *filename, FILE **file, const char *media_owner_path);

byte *QFS_LoadFile 				(const char *path, int usehunk, const char *media_owner_path);
byte *QFS_LoadStackFile 		(const char *path, void *buffer, int bufsize, const char *media_owner_path);
byte *QFS_LoadTempFile 			(const char *path, const char *media_owner_path);		// Only used by Draw_CachePic
byte *QFS_LoadHunkFile 			(const char *path, const char *media_owner_path);
qbool QFS_WriteFile 			(const char *filename, void *data, int len);

void QFS_FindFilesInPak (char *the_arg, const int complete_length, const char *media_owner_path);
//============================================================================
// com_gamedir.c

extern	char	com_basedir[MAX_OSPATH];
extern	struct	cvar_s	registered;
extern	int	rogue, hipnotic, nehahra;


typedef struct
{
	char	name[MAX_QPATH];
	int	filepos, filelen;
} packfile_t;


#define QFS_TYPE_NONE				0
#define QFS_TYPE_PCX_GLOW_IMAGE		1
#define QFS_TYPE_TGA_GLOW_IMAGE		2
#define QFS_TYPE_PCX_IMAGE			3
#define QFS_TYPE_TGA_IMAGE			4
#define QFS_TYPE_LMP				5
#define QFS_TYPE_MDL				6
#define QFS_TYPE_SPR				7
#define QFS_TYPE_WAV				8
#define QFS_TYPE_BSP				9
#define QFS_TYPE_LIT				10
#define QFS_TYPE_LOC				11
#define QFS_TYPE_ENT				12
#define QFS_TYPE_VIS				13
#define QFS_TYPE_DAT				14
#define QFS_TYPE_CFG				15
#define QFS_TYPE_RC					16
#define QFS_TYPE_WAD				17
#define QFS_TYPE_DEM				18
#define QFS_TYPE_DZ					19
#define NUM_QFS_TYPES				20

// Image finding optimization
#define QFSFOLDER_NONE				0
#define QFSFOLDER_GFX_ENV			1	// Skyboxes
#define QFSFOLDER_GFX				2	// Menu, conchars and background replacement elements
#define QFSFOLDER_PROGS				3	// Model and sprites
#define QFSFOLDER_CROSSHAIRS		4	// Crosshairs
#define QFSFOLDER_TEXTURES_EXMY		5	// id1 Quake replacement
#define QFSFOLDER_GFX_PART			6	// particles
#define QFSFOLDER_TEXTURES_ELSE		7	// Likely to be textures/<mapname>
#define QFSFOLDER_TEXTURES			8
#define NUM_QFSFOLDER_TYPES			9


typedef struct pack_s
{
	char		filename[MAX_OSPATH];
	FILE		*handle;
	int			numfiles;
	packfile_t	*files;

	qbool		has_filetype[NUM_QFS_TYPES * NUM_QFSFOLDER_TYPES];	// Baker: We are wasting a few bytes here, big deal
	int			first_index[NUM_QFS_TYPES * NUM_QFSFOLDER_TYPES];	// Baker: We are wasting a few bytes here, big deal
	int			last_index[NUM_QFS_TYPES * NUM_QFSFOLDER_TYPES];	// Baker: We are wasting a few bytes here, big deal
} pack_t;

typedef struct searchpath_s
{
	char		filename[MAX_OSPATH];	// This is actually a path
	pack_t		*pack;					// only one of filename / pack will be used
	struct		searchpath_s *next;
} searchpath_t;

searchpath_t	*com_searchpaths;

//============================================================================
// com_messages.c

typedef struct sizebuf_s
{
	qbool	allowoverflow;	// if false, do a Sys_Error
	qbool	overflowed;		// set to true if the buffer size failed
	byte	*data;
	int		maxsize;
	int		cursize;
} sizebuf_t;

void MSG_WriteChar 	 (sizebuf_t *sb, const int c);
void MSG_WriteByte 	 (sizebuf_t *sb, const int c);
void MSG_WriteShort  (sizebuf_t *sb, const int c);
void MSG_WriteLong 	 (sizebuf_t *sb, const int c);
void MSG_WriteFloat  (sizebuf_t *sb, const float f);
void MSG_WriteString (sizebuf_t *sb, const char *s);
void MSG_WriteCoord  (sizebuf_t *sb, const float f);
void MSG_WriteAngle  (sizebuf_t *sb, const float f);

void MSG_WritePreciseAngle (sizebuf_t *sb, const float f); // JPG - precise aim!!

#if FITZQUAKE_PROTOCOL
void MSG_WriteAngle16 (sizebuf_t *sb, const float f); //johnfitz
#endif

extern	int		msg_readcount;
extern	qbool	msg_badread;		// set if a read goes beyond end of message

void  MSG_BeginReading 	(void);
int   MSG_ReadChar 		(void);
int   MSG_ReadByte 		(void);
int   MSG_PeekByte 		(void); // JPG - need this to check for ProQuake messages
int   MSG_ReadShort 	(void);
int   MSG_ReadLong 		(void);
float MSG_ReadFloat 	(void);
char *MSG_ReadString 	(void);

float MSG_ReadCoord 	(void);
float MSG_ReadAngle 	(void);

float MSG_ReadPreciseAngle (void); // JPG - precise aim!!

#if FITZQUAKE_PROTOCOL
float MSG_ReadAngle16 (void); //johnfitz
#endif


void SZ_Alloc (sizebuf_t *buf, int startsize);
void SZ_Free  (sizebuf_t *buf);
void SZ_Clear (sizebuf_t *buf);
void SZ_Write (sizebuf_t *buf, const void *data, const int length, const qbool bCommandBuffer);
void SZ_Print (sizebuf_t *buf, const char *data);	// strcats onto the sizebuf

void *SZ_GetSpace (sizebuf_t *buf, const int length, const qbool bCommandBuffer);

//============================================================================
// com_parse.c

#define CMDLINE_LENGTH	256


extern	char		com_token[1024];
extern	qbool	com_eof;
extern	int	com_argc;
extern	char	**com_argv;

char *COM_Parse (char *data);
int COM_Commandline_GetInteger (const char *param);

// Command line
int COM_CheckParm (const char *parm);
int COM_Argc (void);
char *COM_Argv (int arg);	// range and null checked
void COM_Init (char *path);
void COM_InitArgv (int argc, char **argv);


//============================================================================
// com_string_fs.c

// StringTemp_ ... temp string created, will be overwritten next time function is called.
// COM_Copy_ ..... Takes const source and performs operation on copy that is dest
// COMD_ ......... MODIFIES source, performing modification to it.
// COM_IS ........ qBoolean (true = 1 or false = 0 on return)

char *StringTemp_SkipPath			(const char *pathname);		// Returns pointer within pathname so can't be const because we don't want to return const char
char *StringTemp_SkipPathAndExten	(const char *pathname);		// Returns pointer within pathname so can't be const because we don't want to return const char

void  COM_Copy_GetPath 				(const char *in, char *out);


char *StringTemp_FileExtension 		(const char *in);
void  COM_Copy_StripExtension 		(const char *in, char *out);
qbool COM_IsExtension 				(const char *filename, const char *extension);

void  COMD_DefaultExtension 		(char *path, const char *extension);
void  COMD_ForceExtension 			(char *path, const char *extension);

void  COMD_StripTrailing_UnixSlash	(char *modify_path);
void  COMD_SlashesBack_Like_Windows (char *UnixStylePath);
void  COMD_SlashesForward_Like_Unix (char *WindowsStylePath);


void  QCOM_FileBase 				(const char *in, char *out);  // This is rather Quakey

qbool COM_ArgIsDemofile 			(const char *potential_filename);

// This is used a couple of places
#define SLASHJMP(x, y)	(x = !(x = strrchr(y, '/')) ? y : x + 1)  // If no slash, returns y else returns after the last slash

//============================================================================
// com_stringf.c

// does a varargs printf into a temp buffer
char *va (const char *format, ...);

const char *StringTemp_NiceFloatString					(const float floatvalue);
	  char *StringTemp_ObtainValueFromClientWorldSpawn  (const char *keyname);
	  char *StringTemp_CreateSpaces						(const int amount); // Baker: temporary home for this
	  char *StringTemp_Quakebar 						(const int in_len);
	  char *StringTemp_AddSpace							(const char *mystring);

void COMD_toLower (char* str);
void COMD_RemoveTrailingSpaces (char *string_with_spaces);
void COMD_StringReplaceChar (char *str, const char find_this_char, const char replace_with_char);

void COMD_DeQuake 		(char *text_to_modify);
void COMD_DeQuake_Name 	(char *text_to_modify);


// Baker:	Newblarized functions to reduce code risks and make more readable
//			Tired of a missing "!" or some little typo screwing up stuff
//			Note: StringLCopy should be conservatively implemented
//			not because it doesn't rock, but because it isn't ideal.
//			These sizeof() are constants (x.name[12] will always have sizeof 12), 
//			yet speed-sapping potential exists
//			because sizeof() is a calculation.  Be careful in things that run every
//			or in big loops.  COM_StringMatch and COM_StringNOTMatch introduce
//			no change because they are just simple macros and get replaced 
//			upon compilation.

#define COM_StringMatch(s1, s2)					!strcmp(s1, s2)
#define COM_StringNOTMatch(s1, s2)				strcmp(s1, s2)
#define COM_StringMatchCaseless(s1, s2)			!strcasecmp(s1, s2)
#define COM_StringNOTMatchCaseless(s1, s2)		strcasecmp(s1, s2)

#define COM_StringMatchNCaseless(s1, s2, s3)	!strncasecmp(s1, s2, s3)
#define StringLCopy(s1, s2)						strlcpy(s1, s2, sizeof(s1))
#define StringLCat(s1, s2)						strlcat(s1, s2, sizeof(s1))

qbool COM_IsInList (const char *itemname, const char *listing_string); 	// Comma delimited, no consideration for spacing (don't use spacing)

int	COM_StringCount (const char *bigstring, const char *findwhat);
//============================================================================

#ifdef _WIN32 // fake snprintf ... change this so it isn't _WIN32 but rather MSVC specific

// Added by VVD
// vc++ snprintf and vsnprintf are non-standard and not compatible with C99.
int qsnprintf(char *str, size_t n, char const *fmt, ...);
int qvsnprintf(char *buffer, size_t count, const char *format, va_list argptr);
#define snprintf qsnprintf
#define vsnprintf qvsnprintf

// MSVC has a different name for several standard functions

# define strcasecmp stricmp
# define strncasecmp strnicmp

#endif //	_WIN32 ... should really be #ifdef _MSC_VER because GNUC even on Windows has this you know ...



char *strltrim(char *s);
size_t strlcat(char *dst, const char *src, size_t siz);
size_t strlcpy(char *dst, const char *src, size_t siz);

//============================================================================

#endif // __COMMON_H__
