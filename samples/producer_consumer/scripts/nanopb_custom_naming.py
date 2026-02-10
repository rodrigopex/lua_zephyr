import re


class NamingStyleLuaC:
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
