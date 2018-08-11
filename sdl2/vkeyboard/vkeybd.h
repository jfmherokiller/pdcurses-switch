/*
 *  Copyright (C) 2002-2017  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


#include <dirent.h>

#ifdef __cplusplus
extern "C" {
#endif

void VKEYBD_Init();

bool VKEYBD_PurgeNext();

#if SDL_VERSION_ATLEAST(2, 0, 0)

void VKEYBD_Run(SDL_Window *window, SDL_PixelFormat *pixelFormat);

#else
void VKEYBD_Run(SDL_Surface *surface, SDL_PixelFormat *pixelFormat);
#endif
#ifdef __cplusplus
}
#endif


typedef unsigned short Bit16u;
typedef signed short Bit16s;
typedef unsigned char Bit8u;
typedef signed char Bit8s;
typedef unsigned int Bit32u;
typedef signed int Bit32s;
typedef unsigned long long Bit64u;
typedef signed long long Bit64s;
typedef Bit64u Bitu;
typedef Bit64s Bits;
