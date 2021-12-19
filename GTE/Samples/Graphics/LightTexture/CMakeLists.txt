if(COMMAND cmake_policy)
    # Allow VERSION in the project() statement.
    cmake_policy(SET CMP0048 NEW)
endif()

project(LightTexture)

cmake_minimum_required(VERSION 3.8)
option(BUILD_RELEASE_LIB "Build release library" OFF)
option(BUILD_SHARED_LIB "Build shared library" OFF)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_definitions(-DGTE_USE_LINUX -DGTE_USE_ROW_MAJOR -DGTE_USE_MAT_VEC -DGTE_USE_OPENGL -DGTE_DISABLE_PCH)
add_compile_options(-c -Wall -Werror)
if(BUILD_RELEASE_LIB)
    add_compile_definitions(NDEBUG)
    add_compile_options(-O3)
else()
    add_compile_definitions(_DEBUG)
    add_compile_options(-g)
endif()

set(GTE_ROOT ${PROJECT_SOURCE_DIR}/../../..)
set(GTE_INC_DIR ${GTE_ROOT})
set(GTE_LIB_PREFIX ${GTE_ROOT}/lib/${CMAKE_BUILD_TYPE})
set(GTE_EXE_PREFIX ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE})
if(BUILD_SHARED_LIB)
    set(GTE_LIB_DIR ${GTE_LIB_PREFIX}Shared)
    set(GTE_EXE_DIR ${GTE_EXE_PREFIX}Shared)
else()
    set(GTE_LIB_DIR ${GTE_LIB_PREFIX}Static)
    set(GTE_EXE_DIR ${GTE_EXE_PREFIX}Static)
endif()
set(EXECUTABLE_OUTPUT_PATH ${GTE_EXE_DIR} CACHE PATH "Executable directory" FORCE)
SET(EXECUTABLE_OUTPUT_PATH ${GTE_EXE_DIR})

include_directories(${GTE_INC_DIR})

add_executable(${PROJECT_NAME}
${PROJECT_NAME}Main.cpp
${PROJECT_NAME}Window3.cpp)

find_package(PNG REQUIRED)
find_package(Threads REQUIRED)
target_link_directories(${PROJECT_NAME} PUBLIC ${GTE_LIB_DIR})
target_link_libraries(${PROJECT_NAME}
gtapplications
gtmathematicsgpu
gtgraphics
GL
EGL
X11
PNG::PNG
Threads::Threads)
