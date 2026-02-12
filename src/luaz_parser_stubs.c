/**
 * @file luaz_parser_stubs.c
 * @brief Stub implementations for parser/lexer symbols when CONFIG_LUA_PRECOMPILE_ONLY is set.
 *
 * When the Lua parser is excluded from the build (lparser.c, llex.c, lcode.c),
 * the Lua core still references luaX_init() from lstate.c and luaY_parser()
 * from ldo.c. These stubs satisfy the linker while ensuring any accidental
 * call to the parser produces a clear error.
 */

#include "lua.h"
#include "lparser.h"
#include "llex.h"
#include "ldebug.h"

/* luaX_init is called during lua_newstate to initialize reserved keywords.
 * With no lexer, this is a no-op. */
void luaX_init(lua_State *L)
{
	UNUSED(L);
}

/* luaY_parser is called when luaL_loadstring/luaL_loadfile is used.
 * In bytecode-only mode, source parsing is not supported. */
LClosure *luaY_parser(lua_State *L, ZIO *z, Mbuffer *buff, Dyndata *dyd, const char *name,
		      int firstchar)
{
	UNUSED(z);
	UNUSED(buff);
	UNUSED(dyd);
	UNUSED(name);
	UNUSED(firstchar);
	luaG_runerror(L, "parser not available (bytecode-only build)");
	return NULL; /* unreachable */
}
