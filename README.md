# cpp-middle-project-sprint-7 <!-- omit in toc -->

- [Начало работы](#начало-работы)
- [Сборка проекта и запуск тестов](#сборка-проекта-и-запуск-тестов)
  - [Команды для сборки проекта](#команды-для-сборки-проекта)
  - [Команды для запуска приложения](#команды-для-запуска-приложения)
  - [Команда для запуска тестов](#команда-для-запуска-тестов)
- [Сборка под Windows](#сборка-под-windows)
  - [Что нужно установить](#что-нужно-установить)
  - [Как собрать](#как-собрать)
  - [Пресеты CMake](#пресеты-cmake)

Шаблон репозитория для практического задания 7-го спринта «Мидл разработчик С++»

## Начало работы

1. Нажмите зелёную кнопку `Use this template`, затем `Create a new repository`.
2. Назовите свой репозиторий.
3. Склонируйте созданный репозиторий командой `git clone your-repository-name`.
4. Создайте новую ветку командой `git switch -c development`.
5. Откройте проект в `Visual Studio Code`.
6. Нажмите `F1` и откройте проект в dev-контейнере командой `Dev Containers: Reopen in Container`.

## Сборка проекта и запуск тестов

Данный репозиторий использует три инструмента:

- **cmake** — генератор систем сборки для C и C++. Позволяет создавать проекты, которые могут компилироваться на различных платформах и с различными компиляторами. Подробнее о cmake:
  - https://dzen.ru/a/ZzZGUm-4o0u-IQlb
  - https://neerc.ifmo.ru/wiki/index.php?title=CMake_Tutorial
  - https://cmake.org/cmake/help/book/mastering-cmake/cmake/Help/guide/tutorial/index.html

- **VS Code Dev Docker container** - Docker контейнер, который содержит полностью настроенное окружение для выполнение задания. Подробнее об этой функциональности:
  - https://habr.com/ru/articles/822707/ - "Почти все, что вы хотели бы знать про Docker"
  - https://code.visualstudio.com/docs/devcontainers/containers - официальная документация VS Code
  - https://www.youtube.com/watch?v=p9L7YFqHGk4 - "Docker container for VS Code"
  - https://www.youtube.com/watch?v=pg19Z8LL06w&t=174s&pp=ygUPRG9ja2VyY29udGFpbmVy - "Docker in 1 hour"

### Команды для сборки проекта

- Создайте папку `build`
- Перейдите в нее `cd build`
- Запустите `cmake ..`
- Запустите `make`

### Команды для запуска приложения

```bash
cd build

./AsyncHttpProxy 5555 &

python3 -c 'print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 4096\r\n\r\n" + "A"*4096, end="")' | nc -l 127.0.0.1 -p 8000 &

wget -e use_proxy=yes -e http_proxy=127.0.0.1:5555 127.0.0.1:8000

```

### Команда для запуска тестов

```bash
cd build
./AsyncHttpProxy_tests
```

## Сборка под Windows

Dev-контейнер остаётся основным способом сборки, но проект собирается и нативно —
компилятором MSVC, без Docker и без WSL. Точка входа одна: `scripts\build_windows.ps1`.

### Что нужно установить

- **Visual Studio 2022** (Community достаточно) с рабочей нагрузкой
  «Разработка классических приложений на C++». Нужен компилятор MSVC v143;
  проверялось на `cl.exe` 19.44. Проект опирается на корутины (`co_await`),
  `std::println` и `std::views::split`, так что более ранние версии, скорее
  всего, не соберут его.
- **CMake ≥ 3.25** и **Ninja** на `PATH`. Оба приезжают вместе с компонентом
  Visual Studio «C++ CMake tools for Windows», если ставить их отдельно не хочется.
- **Conan 2** (`pip install conan`) — из него приезжают Boost и GTest, которых
  в Windows неоткуда взять: в Linux их даёт `apt` внутри dev-контейнера.
  Профиль `default` скрипт создаёт сам, если его ещё нет.

Отдельно запускать «Developer Command Prompt» не требуется — скрипт сам находит
`vcvars64.bat` (через `vswhere`, с запасными путями) и вносит окружение MSVC в
свой процесс.

### Как собрать

```powershell
powershell -ExecutionPolicy Bypass -File scripts\build_windows.ps1
```

Скрипт вызывает Conan, настраивает CMake, собирает всё и прогоняет `ctest`, а в
конце печатает пути к получившимся `.exe`. Результат лежит в `build\windows`:
прокси — `build\windows\AsyncHttpProxy.exe`, тесты — `build\windows\AsyncHttpProxy_tests.exe`.

Запуск прокси не отличается от Linux, порт задаётся аргументом:

```powershell
build\windows\AsyncHttpProxy.exe 5555
```

Полезные ключи:

| Ключ | Зачем |
| --- | --- |
| `-BuildType Debug` | сборка с отладочной информацией в `build\windows-debug` |
| `-Clean` | удалить каталог сборки и собрать с нуля |
| `-SkipTests` | не запускать `ctest` |

Под MSVC у стандарта есть одна оговорка. В Linux проект собирается как C++26,
а у CMake нет флага C++26 для MSVC: `Modules/Compiler/MSVC-CXX.cmake` знает
стандарты только до CXX23. Поэтому в Windows-ветке `CMakeLists.txt` запрашивает
23 — для MSVC это разворачивается ровно в `/std:c++latest`, то есть в самый
свежий режим, который умеет `cl.exe`.

Boost затребован в конфигурации `header_only`: проекту нужны только `boost.asio`
и `boost.beast`, а они целиком заголовочные. Скомпилированные библиотеки Boost
собирать не нужно, поэтому первый прогон занимает минуты, а не полчаса.

### Пресеты CMake

`CMakePresets.json` описывает обе платформы, поэтому IDE (Visual Studio, VS Code,
CLion) подхватывает конфигурацию сама:

| Пресет | Платформа | Генератор | Каталог сборки |
| --- | --- | --- | --- |
| `linux-default` | Linux / dev-контейнер | Unix Makefiles | `build/` |
| `windows-msvc-release` | Windows, MSVC x64 | Ninja | `build/windows/` |
| `windows-msvc-debug` | Windows, MSVC x64 | Ninja | `build/windows-debug/` |

Каждый пресет ссылается на `conan_toolchain.cmake`, поэтому перед запуском пресета
руками нужно сначала выполнить `conan install` — именно это и делает
`scripts\build_windows.ps1`. Windows-пресеты вдобавок рассчитаны на окружение MSVC:
запускать их вручную надо из «Developer PowerShell for VS 2022», иначе CMake не
найдёт `cl.exe`.
