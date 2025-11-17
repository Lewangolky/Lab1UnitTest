cmake_minimum_required(VERSION 3.16)
project(RPGDemo CXX)

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(gtest_build_shared_libs OFF CACHE INTERNAL "")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Налаштування для MSVC
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# --- Налаштування GoogleTest ---
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
)
set(GTEST_DONT_DEFINE_TEST_MACRO_BY_DEFAULT ON)
FetchContent_MakeAvailable(googletest)
# --- Кінець Налаштування GoogleTest ---

enable_testing()

# 1. "Бібліотека" з вашим rpg_engine.h
# Це каже CMake, що файли в цьому каталозі (rpg_engine.h)
# потрібні іншим частинам проєкту.
add_library(RPGEngine INTERFACE)
target_include_directories(RPGEngine INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# 2. Основна програма (використовує lab1.cpp)
add_executable(RPGDemo lab1.cpp)
target_link_libraries(RPGDemo PRIVATE RPGEngine)

# 3. Додаємо підкаталог з тестами
add_subdirectory(tests)