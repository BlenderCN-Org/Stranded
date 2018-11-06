#include <stdlib.h>
#include <string.h>

#include "luaCompile.h"

extern void PrintInfo(bool newLine, const char* format, ...);

#define luac_c
#define LUA_CORE

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "ldo.h"
#include "lfunc.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstring.h"
#include "lundump.h"
}

lua_State* g_L = nullptr;

struct DataBuffer
{
	char* ptr;
	size_t length;
};

static int writer(lua_State* L, const void* p, size_t size, void* u)
{
	UNUSED(L);

	if (size > 0)
	{
		DataBuffer* buffer = (DataBuffer*)u;
		memcpy(buffer->ptr + buffer->length, p, size);
		buffer->length += size;
	}

	return 0;
}

#define toproto(L,i) (clvalue(L->top+(i))->l.p)


int compileLua(const char* name, const char* src, int size, char* dest, bool isStripLua)
{
	// utf8 bom
	if (size > 3 && (unsigned char)src[0] == 0xef && (unsigned char)src[1] == 0xbb && (unsigned char)src[2] == 0xbf)
	{
		src += 3;
		size -= 3;
	}

	if (luaL_loadbuffer(g_L, src, (size_t)size, name) != 0)
	{
		PrintInfo(true, "%s", lua_tostring(g_L, -1));
		return 0;
	}

	const Proto* proto = toproto(g_L, -1);
	// luaU_print(proto, 1);

	DataBuffer buffer;
	buffer.ptr = dest;
	buffer.length = 0;

	lua_lock(g_L);
	luaU_dump(g_L, proto, writer, &buffer, isStripLua ? 1 : 0);
	lua_unlock(g_L);

	return (int)buffer.length;
}

void initLua()
{
	g_L = luaL_newstate();

	/*luaopen_base(L);
	luaopen_table(L);
	luaopen_io(L);
	luaopen_string(L);
	luaopen_math(L);
	luaopen_debug(L);*/
}

void closeLua()
{
	if (g_L)
	{
		lua_close(g_L);
		g_L = nullptr;
	}
}

