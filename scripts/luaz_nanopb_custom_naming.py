"""Custom nanopb naming style that produces snake_case C identifiers.

Loaded by nanopb's generator via the ``--naming-style`` option.  Converts
PascalCase / camelCase protobuf names into the snake_case convention used
by the lua_zephyr integration layer.
"""

import re


class NamingStyleLuaC:
    """nanopb naming style plugin: PascalCase -> snake_case for all C symbols."""

    def enum_name(self, name):
        return self.underscore(name)

    def struct_name(self, name):
        return self.underscore(name)

    def type_name(self, name):
        return "%s_t" % self.underscore(name)

    def define_name(self, name):
        return self.underscore(name)

    def var_name(self, name):
        return self.underscore(name)

    def enum_entry(self, name):
        return self.underscore(name).upper()

    def func_name(self, name):
        return self.underscore(name)

    def bytes_type(self, struct_name, name):
        return "%s_%s_t" % (self.underscore(struct_name), self.underscore(name))

    def underscore(self, word):
        word = str(word)
        word = re.sub(r"([A-Z]+)([A-Z][a-z])", r"\1_\2", word)
        word = re.sub(r"([a-z\d])([A-Z])", r"\1_\2", word)
        word = word.replace("-", "_")
        return word.lower()
