cmake_minimum_required(VERSION 3.31.6)
project(AsyncHttpProxy VERSION 1.0.0 LANGUAGES CXX)

if(MSVC)
    # У CMake 4.1 просто нет флага C++26 для MSVC: в Modules/Compiler/MSVC-CXX.cmake
    # стандарты описаны только до CXX23 (CMAKE_CXX_STANDARD_LATEST = 23), и запрос
    # 26 обрывает конфигурацию - "requires the language dialect CXX26 ... but the
    # current compiler MSVC does not support this".
    # При этом CXX23 для MSVC разворачивается ровно в /std:c++latest, то есть в тот
    # самый режим, где cl.exe 19.44 понимает и std::println, и co_await.
    set(CMAKE_CXX_STANDARD 23)
else()
    set(CMAKE_CXX_STANDARD 26)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Все предупреждения - это ошибки
    add_compile_options(-Wfatal-errors -Wall -Werror)

    # Не больше одной ошибки за раз
    # add_compile_options(-fmax-errors=1) I use clang...
elseif(MSVC)
    # Исходники - UTF-8 без BOM, а cl.exe по умолчанию читает их в кодовой
    # странице системы и спотыкается на русских комментариях (C4819).
    add_compile_options(/utf-8)

    # boost.asio + boost.beast в одном объектнике дают больше 65536 секций,
    # без /bigobj ассемблер обрывается на C1128.
    add_compile_options(/bigobj)

    # Без _WIN32_WINNT boost.asio печатает предупреждение и сам выбирает
    # Windows 7. Фиксируем Windows 10, иначе ветка выбора реализации
    # (IOCP, cancel_io_ex) зависит от машины, на которой шла сборка.
    add_compile_definitions(_WIN32_WINNT=0x0A00)
endif()

# list(APPEND ...), а не set(): conan_toolchain.cmake кладёт в CMAKE_PREFIX_PATH
# каталог со своими *-config.cmake, и присваивание затёрло бы его - следом
# find_package(Boost) и find_package(GTest) не нашли бы ничего.
list(APPEND CMAKE_PREFIX_PATH /usr/local/asio)

# Ищем необходимые библиотеки
find_package(Boost REQUIRED)
find_package(GTest REQUIRED)
file(GLOB HEADER_FILES "${CMAKE_SOURCE_DIR}/include/*.hpp")


# Создаём статическую библиотеку
add_library(${PROJECT_NAME}_imp STATIC
  "${CMAKE_SOURCE_DIR}/src/main.cpp"
  "${CMAKE_SOURCE_DIR}/src/headers.cpp"
  ${HEADER_FILES})

# Добавляем в проект используемые сторонние библиотеки
target_include_directories(${PROJECT_NAME}_imp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME}_imp PRIVATE ${Boost_LIBRARIES})

# Создаём исполняемый таргет и линкуем к нему статическую библиотеку
add_executable(${PROJECT_NAME}
  "${CMAKE_SOURCE_DIR}/src/main.cpp"
  "${CMAKE_SOURCE_DIR}/src/headers.cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_imp)

#
# Тесты
#

# Рекурсивно ищем все используемые в тестах .cpp файлы
file(GLOB TEST_SRC_FILES "${CMAKE_SOURCE_DIR}/tests/*.cpp")

add_executable(${PROJECT_NAME}_tests "${TEST_SRC_FILES}")
target_link_libraries(${PROJECT_NAME}_tests PRIVATE ${PROJECT_NAME}_imp GTest::GTest GTest::Main)

# Включаем тестирование
enable_testing()
add_test(NAME AsyncHttpProxy_Tests COMMAND ${PROJECT_NAME}_tests)
