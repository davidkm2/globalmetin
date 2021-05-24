#if defined(__cplusplus)
extern "C" {
#endif

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

// #ifdef __NEWSTUFF__
#ifndef lua_String
#define lua_String const char*
#endif

#ifndef ALUA
#define ALUA(name) int name(lua_State* L)
#endif
// #endif

#if defined(__cplusplus)
}
#endif
