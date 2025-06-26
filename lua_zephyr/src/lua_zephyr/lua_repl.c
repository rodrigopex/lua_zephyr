#ifdef CONFIG_LUA_REPL

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <lua_zephyr/utils.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/printk.h>

static struct {
	struct {
		char buffer[CONFIG_LUA_THREAD_HEAP_SIZE];
		struct sys_heap heap;
	} lua_heap;
	char input_line[CONFIG_LUA_REPL_LINE_SIZE];
} self = {};

void *shell_getline(const struct shell *shell, char *buf, const size_t len)
{
	__ASSERT(shell != NULL, "Shell cannot be NULL");
	__ASSERT(buf != NULL, "Buffer cannot be NULL");

	memset(buf, 0, len);

	for (size_t i = 0; i < (len - 1); i++) {
		char c;
		size_t cnt = 0;

		while (cnt == 0) {
			shell->iface->api->read(shell->iface, &c, sizeof(c), &cnt);
			if (cnt == 0) {
				k_sleep(K_MSEC(10));
			}
		}

		shell->iface->api->write(shell->iface, &c, sizeof(c), &cnt);
		if (c == '\n' || c == '\r') {
			break;
		}
		buf[i] = c;
	}

	return buf;
}

static void lua_repl_print(lua_State *L)
{
	int n = lua_gettop(L);

	if (n > 0) {
		luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
		lua_getglobal(L, "print");
		lua_insert(L, 1);

		if (lua_pcall(L, n, 0, 0) != LUA_OK) {
			printk("error: %s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
		}
	}
}

static int lua_repl_cmd(const struct shell *sh, size_t argc, char **argv, void *data)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	ARG_UNUSED(data);

	sys_heap_init(&self.lua_heap.heap, self.lua_heap.buffer, CONFIG_LUA_THREAD_HEAP_SIZE);

	lua_State *L = lua_newstate(lua_zephyr_allocator, &self.lua_heap.heap);
	if (L == NULL) {
		shell_error(sh, "Failed to create Lua state");
		return -ENOMEM;
	}

	LUA_REQUIRE(zephyr);
	LUA_REQUIRE(base);

	shell_print(sh, "\nZephyr Lua v5.4.7 REPL. Type ':quit' to exit.\n");

	while (true) {
		shell_fprintf(sh, SHELL_NORMAL, "lua> ");
		shell_getline(sh, self.input_line, sizeof(self.input_line));

		/* new line needed to avoid character superposition from previous input */
		shell_print(sh, "\n");

		if (strcmp(self.input_line, ":quit") == 0) {
			break;
		}

		const char *line = self.input_line;

		char line_with_ret[CONFIG_LUA_REPL_LINE_SIZE + sizeof("return ")];
		snprintf(line_with_ret, sizeof(line_with_ret), "return %s", line);
		int status = luaL_loadstring(L, line_with_ret);
		if (status != LUA_OK) {
			lua_pop(L, 1);
			status = luaL_loadstring(L, line);
		}

		if (status == LUA_OK) {
			status = lua_pcall(L, 0, LUA_MULTRET, 0);
			if (status == LUA_OK) {
				lua_repl_print(L);
			} else {
				shell_error(sh, "Error: %s", lua_tostring(L, -1));
				lua_pop(L, 1);
			}
		} else {
			shell_error(sh, "Syntax Error: %s", lua_tostring(L, -1));
			lua_pop(L, 1);
		}
	}

	lua_close(L);
	memset(&self, 0, sizeof(self));

	return 0;
}

SHELL_CMD_REGISTER(lua, NULL, "Launch Lua REPL", lua_repl_cmd);

#endif /* CONFIG_LUA_REPL */
