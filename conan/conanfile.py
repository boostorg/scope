# Copyright 2023 Dmitry Arkhipov (grisumbras@yandex.ru)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt)

from conan import ConanFile
from conan.errors import (
    ConanException,
    ConanInvalidConfiguration,
)
from conan.tools.files import (
    copy,
    patch,
)
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import (
    cmake_layout,
)
from conan.tools.scm import Version

import os

required_conan_version = ">=1.53.0"

class ScopeConan(ConanFile):
    name = "boost_scope"
    version = "1.0.0"
    description = (
        "Boost.Scope provides a number of scope guard utilities described " +
        "in C++ Extensions for Library Fundamentals, Version 3")

    url = "https://github.com/Lastique/scope"
    homepage = "https://github.com/Lastique/scope"
    license = "BSL-1.0"
    topics = "cpp"

    settings = "compiler", "build_type"

    requires = "boost/[>=1.83.0]"

    @property
    def _min_compiler_version_default_cxx11(self):
        # Minimum compiler version having c++ standard >= 11 by default
        return {
            "apple-clang": 99, # assume apple-clang will default to c++11 in the distant future
            "gcc": 6,
            "clang": 6,
            "Visual Studio": 14, # guess
            "msvc": 190, # guess
        }.get(str(self.settings.compiler))

    def validate(self):
        if self.settings.compiler.get_safe("cppstd"):
            check_min_cppstd(self, 11)
        else:
            version_cxx11_standard_json = self._min_compiler_version_default_cxx11
            if not version_cxx11_standard_json:
                self.output.warning("Assuming the compiler supports c++11 by default")
            elif Version(self.settings.compiler.version) < version_cxx11_standard_json:
                raise ConanInvalidConfiguration("Boost.Scope requires C++11")

    def export_sources(self):
        src = os.path.join(self.recipe_folder, "..")
        copy(self, "LICENSE",                  src, self.export_sources_folder)
        copy(self, "include*",                 src, self.export_sources_folder)
        copy(self, "conan/1.83_compat.patch",  src, self.export_sources_folder)

    def build(self):
        patch_file = os.path.join(self.source_folder, "conan/1.83_compat.patch")
        patch(self, patch_file=patch_file)

    def package(self):
        copy(self, "include/*", self.source_folder, self.package_folder)
        copy(self, "LICENSE", self.source_folder, self.package_folder)

    def package_id(self):
        self.info.clear()
