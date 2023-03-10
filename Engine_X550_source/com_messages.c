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
// com_messages.c -- misc functions used in client and server

#include "quakedef.h"
#include "net.h"
//#include <assert.h>
//#include <io.h>

/*
==============================================================================

			MESSAGE IO FUNCTIONS

Handles byte ordering and avoids alignment errors
==============================================================================
*/

// writing functions

void MSG_WriteChar (sizebuf_t *sb, const int c)
{
	byte	*buf;

#ifdef PARANOID
	if (c < -128 || c > 127)
		Sys_Error ("MSG_WriteChar: range error");
#endif

	buf = SZ_GetSpace (sb, 1, false /* not command buffer*/);
	buf[0] = c;
}

void MSG_WriteByte (sizebuf_t *sb, const int c)
{
	byte	*buf;

#ifdef PARANOID
	if (c < 0 || c > 255)
		Sys_Error ("MSG_WriteByte: range error");
#endif

	buf = SZ_GetSpace (sb, 1, false /* not command buffer*/);
	buf[0] = c;
}

void MSG_WriteShort (sizebuf_t *sb, const int c)
{
	byte	*buf;

#ifdef PARANOID
	if (c < ((short)0x8000) || c > (short)0x7fff)
		Sys_Error ("MSG_WriteShort: range error");
#endif

	buf = SZ_GetSpace (sb, 2, false /* not command buffer*/);
	buf[0] = c & 0xff;
	buf[1] = c >> 8;
}

void MSG_WriteLong (sizebuf_t *sb, const int c)
{
	byte	*buf;

	buf = SZ_GetSpace (sb, 4, false /* not command buffer*/);
	buf[0] = c & 0xff;
	buf[1] = (c >> 8) & 0xff;
	buf[2] = (c >> 16) & 0xff;
	buf[3] = c >> 24;
}

void MSG_WriteFloat (sizebuf_t *sb, const float f)
{
	union
	{
		float	f;
		int	l;
	} dat;


	dat.f = f;
	dat.l = LittleLong (dat.l);

	SZ_Write (sb, &dat.l, 4, false /* not command buffer*/);
}

void MSG_WriteString (sizebuf_t *sb, const char *s)
{
	if (!s)
		SZ_Write (sb, "", 1, false /* not command buffer*/);
	else
		SZ_Write (sb, s, strlen(s) + 1, false /* not command buffer*/);

}


#if FITZQUAKE_PROTOCOL
//johnfitz -- original behavior, 13.3 fixed point coords, max range +-4096
void MSG_WriteCoord16 (sizebuf_t *sb, float f)
{
	MSG_WriteShort (sb, Q_rint(f*8));
}

//johnfitz -- 16.8 fixed point coords, max range +-32768
void MSG_WriteCoord24 (sizebuf_t *sb, const float f)
{
	MSG_WriteShort (sb, f);
	MSG_WriteByte (sb, (int)(f*255)%255);
}

//johnfitz -- 32-bit float coords
void MSG_WriteCoord32f (sizebuf_t *sb, const float f)
{
	MSG_WriteFloat (sb, f);
}
#endif

void MSG_WriteCoord (sizebuf_t *sb, const float f)
{
#if FITZQUAKE_PROTOCOL
	MSG_WriteCoord16 (sb, f);
#else
	MSG_WriteShort (sb, (int)(f * 8));
#endif
}

void MSG_WriteAngle (sizebuf_t *sb, const float f)
{
	//MSG_WriteByte (sb, (int)floor(f * 256 / 360 + 0.5) & 255); // Baker 3.76 - LordHavoc precision aiming fix
#if FITZQUAKE_PROTOCOL
	MSG_WriteByte (sb, Q_rint(f * 256.0 / 360.0) & 255); //johnfitz -- use Q_rint instead of (int)
#else
	MSG_WriteByte (sb, ((int)f * 256 / 360) & 255);
#endif
}

#if FITZQUAKE_PROTOCOL
//johnfitz -- for PROTOCOL_FITZQUAKE
void MSG_WriteAngle16 (sizebuf_t *sb, const float f)
{
	MSG_WriteShort (sb, Q_rint(f * 65536.0 / 360.0) & 65535);
}
//johnfitz
#endif

// JPG - precise aim for ProQuake!
void MSG_WritePreciseAngle (sizebuf_t *sb, const float f)
{
	int val = (int)f*65536/360;
	MSG_WriteShort (sb, val & 65535);
}

// reading functions
int		msg_readcount;
qbool	msg_badread;

void MSG_BeginReading (void)
{
	msg_readcount = 0;
	msg_badread = false;
}

// returns -1 and sets msg_badread if no more characters are available
int MSG_ReadChar (void)
{
	int     c;

//	if (msg_readcount+1 > net_message.cursize)
	if (msg_readcount >= net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	c = (signed char)net_message.data[msg_readcount];
	msg_readcount++;

	return c;
}

int MSG_ReadByte (void)
{
	int     c;

//	if (msg_readcount+1 > net_message.cursize)
	if (msg_readcount >= net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	c = (unsigned char)net_message.data[msg_readcount];
	msg_readcount++;

	return c;
}

// JPG - need this to check for ProQuake messages
int MSG_PeekByte (void)
{
	if (msg_readcount+1 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	return (unsigned char)net_message.data[msg_readcount];
}

int MSG_ReadShort (void)
{
	int	c;

	if (msg_readcount+2 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	c = (short)(net_message.data[msg_readcount]
	+ (net_message.data[msg_readcount+1] << 8));

	msg_readcount += 2;

	return c;
}

int MSG_ReadLong (void)
{
	int	c;

	if (msg_readcount+4 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	c = net_message.data[msg_readcount]
	+ (net_message.data[msg_readcount+1] << 8)
	+ (net_message.data[msg_readcount+2] << 16)
	+ (net_message.data[msg_readcount+3] << 24);

	msg_readcount += 4;

	return c;
}

float MSG_ReadFloat (void)
{
	union
	{
		byte	b[4];
		float	f;
		int	l;
	} dat;

	dat.b[0] = net_message.data[msg_readcount];
	dat.b[1] = net_message.data[msg_readcount+1];
	dat.b[2] = net_message.data[msg_readcount+2];
	dat.b[3] = net_message.data[msg_readcount+3];
	msg_readcount += 4;

	dat.l = LittleLong (dat.l);

	return dat.f;
}

char *MSG_ReadString (void)
{
	static	char	string[2048];
	int		l, c;

	l = 0;
	do
	{
		c = MSG_ReadChar ();
		if (c == -1 || c == 0)
			break;
		string[l] = c;
		l++;
	} while (l < sizeof(string)-1);

	string[l] = 0;

	return string;
}

#if FITZQUAKE_PROTOCOL
//johnfitz -- original behavior, 13.3 fixed point coords, max range +-4096
float MSG_ReadCoord16 (void)
{
	return MSG_ReadShort() * (1.0/8);
}

//johnfitz -- 16.8 fixed point coords, max range +-32768
float MSG_ReadCoord24 (void)
{
	return MSG_ReadShort() + MSG_ReadByte() * (1.0/255);
}

//johnfitz -- 32-bit float coords
float MSG_ReadCoord32f (void)
{
	return MSG_ReadFloat();
}
#endif

float MSG_ReadCoord (void)
{
#if FITZQUAKE_PROTOCOL
	return MSG_ReadCoord16();
#else
	return MSG_ReadShort() * (1.0 / 8);
#endif
}

float MSG_ReadAngle (void)
{
	return MSG_ReadChar() * (360.0 / 256);
}

// JPG - exact aim for proquake!
float MSG_ReadPreciseAngle (void)
{
	int val = MSG_ReadShort();
	return val * (360.0/65536);
}

#if FITZQUAKE_PROTOCOL
//johnfitz -- for PROTOCOL_FITZQUAKE
float MSG_ReadAngle16 (void)
{
	return MSG_ReadShort() * (360.0 / 65536);
}
//johnfitz
#endif

//===========================================================================

void SZ_Alloc (sizebuf_t *buf, int startsize)
{
	if (startsize < 256)
		startsize = 256;
	buf->data = Hunk_AllocName (1, startsize, "sizebuf");
	buf->maxsize = startsize;
	buf->cursize = 0;
}

void SZ_Free (sizebuf_t *buf)
{
//      Z_Free (buf->data);
//      buf->data = NULL;
//      buf->maxsize = 0;
	buf->cursize = 0;
}

void SZ_Clear (sizebuf_t *buf)
{
	buf->cursize = 0;
}

void *SZ_GetSpace (sizebuf_t *buf, const int length, const qbool bCommandBuffer)
{
	void	*data;

	if (buf->cursize + length > buf->maxsize)
	{
		if (!buf->allowoverflow)
			if (bCommandBuffer)
			{
				Sys_Error ( "SZ_GetSpace: overflow without allowoverflow set\n\nThis occurred during command execution. "
							"A likely cause is a very large config file.\n\n"
							"Buffer maxsize = %i\n"
							"Insertion length = %i\n"
							"Previous buffer contents length = %i",
							buf->maxsize, length, buf->cursize);
			}
			else
			{
				if (buf->maxsize = 7998)
				{
					Sys_Error ("SZ_GetSpace: overflow without allowoverflow set.\n\n"
							"It is likely that the signon buffer cannot handle the baseline.\n\n"
							"If this is the case, try using protocol 666.\n\n"
							"Buffer maxsize = %i\n"
							"Insertion length = %i\n"
							"Previous buffer contents length = %i",
							buf->maxsize, length, buf->cursize);
				}
				else
				{
					Sys_Error ("SZ_GetSpace: overflow without allowoverflow set.\n\n"
							"Buffer maxsize = %i\n"
							"Insertion length = %i\n"
							"Previous buffer contents length = %i",
							buf->maxsize, length, buf->cursize);
				}
			}

		if (length > buf->maxsize)
			Sys_Error ("SZ_GetSpace: %i is > full buffer size", length);

		buf->overflowed = true;
		Con_Printf ("SZ_GetSpace: overflow");
		SZ_Clear (buf);
	}

	data = buf->data + buf->cursize;
	buf->cursize += length;

	return data;
}

void SZ_Write (sizebuf_t *buf, const void *data, const int length, const qbool bCommandBuffer)
{
	memcpy (SZ_GetSpace(buf, length, bCommandBuffer), data, length);
}

void SZ_Print (sizebuf_t *buf, const char *data)
{
	int	len;

	len = strlen(data) + 1;

// byte * cast to keep VC++ happy
	if (buf->data[buf->cursize-1])
		memcpy ((byte *)SZ_GetSpace(buf, len, false /* not command buffer*/), data, len);	// no trailing 0
	else
		memcpy ((byte *)SZ_GetSpace(buf, len - 1, false /* not command buffer*/) - 1, data, len); // write over trailing 0
}

