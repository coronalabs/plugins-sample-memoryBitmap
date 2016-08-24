// ----------------------------------------------------------------------------
// PluginMemBtm.cpp
// Copyright (c) 2016 Corona Labs Inc. All rights reserved.
// This software may be modified and distributed under the terms
// of the MIT license.  See the LICENSE.txt file for details.
// ----------------------------------------------------------------------------


#include "CoronaAssert.h"
#include "CoronaGraphics.h"
#include "PluginMemBtm.h"

#include <cstring>


const char PluginMemBtm_kName[] = "plugin.memoryBitmap";


struct MemBitmap
{
	int w,h;
	CoronaExternalBitmapFormat format;
	unsigned char*data;
};

static unsigned int MemBitmap_GetW(void *context)
{
	return ((MemBitmap*)context)->w;
}

static unsigned int MemBitmap_GetH(void *context)
{
	return ((MemBitmap*)context)->h;
}

static const void* MemBitmap_GetData(void *context)
{
	return ((MemBitmap*)context)->data;
}

static CoronaExternalBitmapFormat MemBitmap_Format(void *context)
{
	return ((MemBitmap*)context)->format;
}

static void MemBitmap_Dispose(void *context)
{
	MemBitmap * btm = (MemBitmap*)context;
	if (btm->data) {
		delete [] btm->data;
	}
	delete btm;
}


static inline unsigned char clampColor(double x)
{
	return x < 0 ? 0 : x > 255 ? 255 : (unsigned char)x;
}

static int MemBitmap_GetPixel( lua_State *L )
{
	int index = 1;
	int res = 0;
	MemBitmap *btm = (MemBitmap*)CoronaExternalGetUserData(L, index );
	index++;
	
	if ( btm != NULL )
	{
		int x=-1, y=-1;
		if (lua_type( L, index) == LUA_TNUMBER)
		{
			x = (int)lua_tointeger( L, index) - 1;
			if (x>=btm->w)
			{
				x = -1;
			}
		}
		index++;

		if (lua_type( L, index ) == LUA_TNUMBER)
		{
			y = (int)lua_tointeger( L, index) - 1;
			if (y>=btm->h)
			{
				y = -1;
			}
		}
		index++;
		
		if ( x>=0 && y>=0 )
		{
			const int bpp = CoronaExternalFormatBPP(btm->format);
			const int stride = btm->w * bpp;
			const double m = 1/255.0;
			for (int c = 0; c<bpp; c++)
			{
				lua_pushnumber( L, m * btm->data[ stride*y + x*bpp + c ]);
				res ++;
			}
		}
		
	}
	return res;
}

static int MemBitmap_SetPixel( lua_State *L )
{
	int index = 1;
	MemBitmap *btm = (MemBitmap*)CoronaExternalGetUserData(L, index );
	index++;
	
	if ( btm != NULL )
	{
		int x=-1, y=-1;
		
		if (lua_type( L, index) == LUA_TNUMBER )
		{
			x = (int)lua_tointeger( L, index) - 1;
			if (x>=btm->w)
			{
				x = -1;
			}
		}
		index++;
		
		if (lua_type( L, index) == LUA_TNUMBER )
		{
			y = (int)lua_tointeger( L, index) - 1;
			if (y>=btm->h)
			{
				y = -1;
			}
		}
		index++;
		
		if ( x>=0 && y>=0 )
		{
			const int bpp = CoronaExternalFormatBPP(btm->format);
			const int stride = btm->w * bpp;
			const double m = 255.0;
			if (lua_type(L, index) == LUA_TNUMBER)
			{
				for (int c=0; c<bpp; c++)
				{
					if ( lua_type(L, index) == LUA_TNUMBER )
					{
						double color = lua_tonumber(L, index);
						btm->data[ stride*y + x*bpp + c ] = clampColor(color*m);
					}
					index++;
				}
			}
			else if(lua_type(L, index) == LUA_TTABLE)
			{
				int len = (int)lua_objlen( L, index);
				if (len > bpp)
				{
					len = bpp;
				}
				for (int c=0; c<len; c++)
				{
					lua_rawgeti( L, index, c+1);
					if ( lua_type(L, -1) == LUA_TNUMBER )
					{
						double color = lua_tonumber(L, -1);
						btm->data[ stride*y + x*bpp + c ] = clampColor(color*m);
					}
					lua_pop(L, 1);
				}
			}
		}
		
	}
	return 0;
}


static int MemBitmap_Resize( lua_State *L )
{
	int index = 1;
	MemBitmap *bitmap = (MemBitmap*)CoronaExternalGetUserData(L, index );
	index++;
	
	if ( bitmap != NULL )
	{
		int w=-1, h=-1;
		
		if (lua_type( L, index) == LUA_TNUMBER)
		{
			w = (int)lua_tointeger( L, index);
			if (w < 1)
			{
				w = -1;
			}
		}
		index++;
		
		if (lua_type( L, index) == LUA_TNUMBER)
		{
			h = (int)lua_tointeger( L, index);
			if (h < 1)
			{
				h = -1;
			}
		}
		index++;
		
		if ( w>=0 && h>=0 )
		{
			
			bitmap->w = w;
			bitmap->h = h;
			size_t sz = w*h*CoronaExternalFormatBPP(bitmap->format);
			delete [] bitmap->data;
			bitmap->data = new unsigned char[sz];
			memset( bitmap->data, 0, sz );
		}
		
	}
	return 0;
}

static int PushCachedFunction( lua_State *L, lua_CFunction f )
{
	// check cache for the funciton, cache key is function address
	lua_pushlightuserdata( L, (void*)f );
	lua_gettable( L, LUA_REGISTRYINDEX );
	
	// cahce miss
	if ( !lua_iscfunction( L, -1 ) )
	{
		lua_pop( L, 1 ); // pop nil on top of stack
		
		// create c function closure on top of stack
		lua_pushcfunction( L, f );
		
		// push cache key
		lua_pushlightuserdata( L, (void*)f );
		// copy function to be on top of stack as well
		lua_pushvalue( L, -2 );
		lua_settable( L, LUA_REGISTRYINDEX );
		
		// now original function is on top of stack, and cache key and function is in cache
	}
	
	return 1;
}

static int MemBitmap_GetField(lua_State *L, const char *field, void* context)
{
	int res = 0;
	if ( strcmp(field, "getPixel") == 0 )
	{
		res = PushCachedFunction(L, MemBitmap_GetPixel);
	}
	else if ( strcmp(field, "setPixel") == 0 )
	{
		res = PushCachedFunction(L, MemBitmap_SetPixel);
	}
	else if ( strcmp(field, "format") == 0 )
	{
		switch (((MemBitmap*)context)->format)
		{
			case kExternalBitmapFormat_Mask:
				lua_pushstring(L, "mask");
				break;
			case kExternalBitmapFormat_RGB:
				lua_pushstring(L, "rgb");
				break;
			default:
				lua_pushstring(L, "rgba");
				break;
		}
		res = 1;
	}
	else if( strcmp(field, "resize") == 0 )
	{
		res = PushCachedFunction(L, MemBitmap_Resize);
	}
	return res;
}



static int PluginMemBtm_New(lua_State *L )
{
	int index = 1;
	int res = 0;
	if(lua_type( L, index ) == LUA_TTABLE)
	{
		int w=0, h=0;
		CoronaExternalBitmapFormat format = kExternalBitmapFormat_RGBA;
		
		lua_getfield( L, index, "width" );
		if ( lua_type( L, -1) == LUA_TNUMBER )
		{
			w = (int)lua_tointeger( L, -1 );
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, index, "height" );
		if ( lua_type( L, -1) == LUA_TNUMBER )
		{
			h = (int)lua_tointeger( L, -1 );
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, index, "format" );
		if ( lua_type( L, -1) == LUA_TSTRING )
		{
			const char * fmt = lua_tostring( L, -1 );
			if ( strcmp(fmt, "rgb") == 0 )
			{
				format = kExternalBitmapFormat_RGB;
			}
			else if ( strcmp(fmt, "mask") == 0 )
			{
				format = kExternalBitmapFormat_Mask;
			}
		}
		lua_pop( L, 1 );

		

		if (w > 0 && h > 0)
		{
			// set up blank bitmap
			MemBitmap * bitmap = new MemBitmap();
			bitmap->w = w;
			bitmap->h = h;
			bitmap->format = format;
			size_t sz = w*h*CoronaExternalFormatBPP(format);
			bitmap->data = new unsigned char[sz];
			memset( bitmap->data, 0, sz );
			
			// set up callbacks
			CoronaExternalTextureCallbacks callbacks = {};
			callbacks.size = sizeof(CoronaExternalTextureCallbacks);
			callbacks.getWidth = MemBitmap_GetW;
			callbacks.getHeight = MemBitmap_GetH;
			callbacks.onRequestBitmap = MemBitmap_GetData;
			callbacks.getFormat = MemBitmap_Format;
			callbacks.onGetField = MemBitmap_GetField;
			callbacks.onFinalize = MemBitmap_Dispose;
			res = CoronaExternalPushTexture( L, &callbacks,  bitmap );
		}
	}
	
	
	return res;
}



CORONA_EXPORT
int luaopen_plugin_memoryBitmap( lua_State *L )
{
	static const luaL_Reg kVTable[] =
	{
		{ "newTexture", PluginMemBtm_New },
		
		{ NULL, NULL }
	};
	
	luaL_openlib( L, PluginMemBtm_kName, kVTable, 0 );
	
	return 1;
}

