/**
 * @file lua_repl.c
 * @brief Interactive Lua REPL integrated with the Zephyr shell.
 *
 * Registers a `lua` shell command that launches a read-eval-print loop.
 * The REPL runs in its own sys_heap and supports Ctrl+D (exit) and
 * Ctrl+L (clear screen).  Enabled via CONFIG_LUA_REPL.
 */

#ifdef CONFIG_LUA_REPL

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <lua_zephyr/utils.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/printk.h>

/** @brief End-of-transmission (Ctrl+D) — exits the REPL. */
#define EOT 0x04
/** @brief Backspace character. */
#define BS  0x08
/** @brief Delete character (treated the same as backspace). */
#define DEL 0x7F
/** @brief Form-feed (Ctrl+L) — clears the screen. */
#define FF  0x0C

static struct {
	struct {
		char buffer[CONFIG_LUA_THREAD_HEAP_SIZE];
		struct sys_heap heap;
	} lua_heap;
	char input_line[CONFIG_LUA_REPL_LINE_SIZE];
} self = {};

/**
 * @brief Read one line of input from the shell transport.
 *
 * Blocks until a newline, EOF (Ctrl+D), or buffer-full.  Handles
 * backspace/delete and Ctrl+L (clear screen) inline.
 *
 * @param sh             Shell instance.
 * @param buf            Destination buffer.
 * @param len            Buffer capacity (including NUL).
 * @param received_exit  Set to true if the user pressed Ctrl+D.
 */
static void shell_getline(const struct shell *sh, char *buf, const size_t len, bool *received_exit)
{
	__ASSERT_NO_MSG(sh != NULL);
	__ASSERT_NO_MSG(buf != NULL);
	__ASSERT_NO_MSG(len > 0);
	__ASSERT_NO_MSG(len <= CONFIG_LUA_REPL_LINE_SIZE);
	__ASSERT_NO_MSG(received_exit != NULL);

	const struct shell_transport_api *sh_api = sh->iface->api;

	(void)memset(buf, 0, len);

	for (size_t i = 0; i < (len - 1);) {
		char c;
		size_t cnt = 0;

		WAIT_FOR(cnt > 0, UINT32_MAX, (sh_api->read(sh->iface, &c, sizeof(c), &cnt)));

		if (c == BS || c == DEL) {
			i--;
			buf[i] = '\0';
			shell_fprintf(sh, SHELL_NORMAL, "\b \b");

			continue;
		}

		sh_api->write(sh->iface, &c, sizeof(c), &cnt);
		if (cnt == 1) {
			if (c == EOT) {
				*received_exit = true;
				return;
			}

			if (c == FF) {
				shell_print(sh, "\033[H\033[J");
				shell_fprintf(sh, SHELL_NORMAL, "lua> ");
			}
		}

		if (c == '\n' || c == '\r') {
			break;
		}

		buf[i] = c;
		i++;
	}
}

/** @brief Print all values on the Lua stack using the global `print` function. */
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

/**
 * @brief Shell command handler that runs the Lua REPL loop.
 *
 * Creates a Lua state backed by a dedicated sys_heap, loads the `zephyr`
 * and `base` libraries, then enters the read-eval-print loop until the
 * user presses Ctrl+D.
 */
static int lua_repl_cmd(const struct shell *sh, size_t argc, char **argv, void *data)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	ARG_UNUSED(data);

	bool received_exit = false;

	sys_heap_init(&self.lua_heap.heap, self.lua_heap.buffer, CONFIG_LUA_THREAD_HEAP_SIZE);

	lua_State *L = lua_newstate(lua_zephyr_allocator, &self.lua_heap.heap);
	if (L == NULL) {
		shell_error(sh, "Failed to create Lua state");
		return -ENOMEM;
	}

	LUA_REQUIRE(zephyr);
	LUA_REQUIRE(base);

	shell_print(
		sh,
		"\nZephyr Lua v5.4.7 REPL. Press Ctrl+D to exit or Ctrl+L to clear the screen.\n");

	while (true) {
		shell_fprintf(sh, SHELL_NORMAL, "lua> ");
		shell_getline(sh, self.input_line, sizeof(self.input_line), &received_exit);

		if (received_exit) {
			return 0;
		}

		/* new line needed to avoid character superposition from previous input */
		shell_print(sh, "\n");

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
