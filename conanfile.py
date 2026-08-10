from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain


class AsyncHttpProxyConan(ConanFile):
    """Зависимости проекта для платформ, где нет dev-контейнера.

    В Linux Boost и GTest приезжают из apt внутри образа `.devcontainer`, и
    сборка про Conan ничего не знает. В Windows такого источника нет, поэтому
    те же две библиотеки берутся отсюда. Файл добавлен ради Windows-сборки и
    Linux-путь не ломает: `find_package(Boost/GTest)` одинаково доволен и
    системными пакетами, и сгенерированными CMakeDeps конфигами.
    """

    name = "async_http_proxy"
    version = "1.0.1"
    settings = "os", "compiler", "build_type", "arch"

    default_options = {
        # Проекту нужны только boost.asio и boost.beast, а они целиком
        # header-only (boost::system стал header-only ещё в 1.69). Собирать
        # ради них скомпилированные библиотеки Boost - это лишние минут сорок
        # на первом прогоне и ноль пользы.
        "boost/*:header_only": True,
    }

    def requirements(self):
        self.requires("boost/1.86.0")
        self.requires("gtest/1.14.0")

    def layout(self):
        self.folders.source = "."
        self.folders.build = "build"
        self.folders.generators = "build/generators"

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        tc.variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
