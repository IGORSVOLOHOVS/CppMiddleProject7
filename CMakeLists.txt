cmake_minimum_required(VERSION 3.28)
project(AsyncHttpProxy VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# clang-tidy reads this: without it the FetchContent headers are not found.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# The proxy is the only part that needs Boost and <print>. recipe.ps1 switches it off, so the
# header parser builds and tests with g++ 13 and no Boost.
option(ASYNC_HTTP_PROXY_APP "Build AsyncHttpProxy, the proxy on Boost.Asio" ON)
option(BUILD_TESTING "Build tests" ON)

function(warn_as_errors target)
    if(NOT MSVC)
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endfunction()

if(MSVC)
    # The sources are UTF-8 without a BOM, and cl.exe reads them in the system code page otherwise.
    add_compile_options(/utf-8)
    # boost.asio and boost.beast in one object pass 65536 sections: without /bigobj the assembler stops on C1128.
    add_compile_options(/bigobj)
    # Without _WIN32_WINNT boost.asio picks Windows 7 and the build depends on the machine.
    add_compile_definitions(_WIN32_WINNT=0x0A00)
endif()

add_library(${PROJECT_NAME}_imp STATIC src/headers.cpp)
target_include_directories(${PROJECT_NAME}_imp PUBLIC include)
warn_as_errors(${PROJECT_NAME}_imp)

if(ASYNC_HTTP_PROXY_APP)
    # list(APPEND), not set(): conan_toolchain.cmake already holds its config folder in CMAKE_PREFIX_PATH.
    list(APPEND CMAKE_PREFIX_PATH /usr/local/asio)
    find_package(Boost REQUIRED)

    add_executable(${PROJECT_NAME} src/main.cpp)
    target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_imp ${Boost_LIBRARIES})
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${PROJECT_NAME} PRIVATE -Wfatal-errors -Wall -Werror)
    endif()
endif()

if(BUILD_TESTING)
    enable_testing()

    # An archive and its hash, not a git clone: nothing here can ask for a credential on the runner.
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.tar.gz
        URL_HASH SHA256=ad7fdba11ea011c1d925b3289cf4af2c66a352e18d4c7264392fead75e919363
        SYSTEM
    )
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(${PROJECT_NAME}_tests tests/headers_test.cpp tests/main.cpp)
    target_link_libraries(${PROJECT_NAME}_tests PRIVATE ${PROJECT_NAME}_imp GTest::gtest)
    warn_as_errors(${PROJECT_NAME}_tests)

    add_test(NAME AsyncHttpProxy_Tests COMMAND ${PROJECT_NAME}_tests)
    # The manual list is judged where every other file is judged (section 9).
    add_test(
        NAME manual_list
        COMMAND python -m pytest tests/test_manual_list.py -q -p no:cacheprovider
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()
