cmake_minimum_required(VERSION 3.18)
project(geometric_dungeon C CXX)

# Do Gradle truyền vào: thư mục raylib 5.5 (được workflow tải về) và file nguồn của game
set(RAYLIB_DIR "" CACHE PATH "Thư mục gốc của raylib")
set(GAME_SOURCE "" CACHE FILEPATH "File main.cpp của game")
if(NOT RAYLIB_DIR OR NOT EXISTS "${RAYLIB_DIR}/src/raylib.h")
    message(FATAL_ERROR "Không tìm thấy raylib tại '${RAYLIB_DIR}'. Hãy chạy: git clone --depth 1 --branch 5.5 https://github.com/raysan5/raylib.git android/raylib")
endif()
if(NOT GAME_SOURCE OR NOT EXISTS "${GAME_SOURCE}")
    message(FATAL_ERROR "Không tìm thấy mã nguồn game tại '${GAME_SOURCE}'")
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(RL "${RAYLIB_DIR}/src")
set(NATIVE_APP_GLUE_DIR "${CMAKE_ANDROID_NDK}/sources/android/native_app_glue")

# android_native_app_glue của NDK
add_library(native_app_glue STATIC "${NATIVE_APP_GLUE_DIR}/android_native_app_glue.c")
target_include_directories(native_app_glue PUBLIC "${NATIVE_APP_GLUE_DIR}")

# raylib cho Android (OpenGL ES 2.0)
add_library(raylib_android STATIC
    "${RL}/rcore.c" "${RL}/rshapes.c" "${RL}/rtextures.c" "${RL}/rtext.c"
    "${RL}/rmodels.c" "${RL}/raudio.c" "${RL}/utils.c")
target_compile_definitions(raylib_android PUBLIC PLATFORM_ANDROID GRAPHICS_API_OPENGL_ES2)
target_include_directories(raylib_android PUBLIC "${RL}" "${RL}/external")
target_link_libraries(raylib_android PUBLIC native_app_glue)
target_compile_options(raylib_android PRIVATE -ffunction-sections -funwind-tables -fstack-protector-strong -Wno-unused-value -Wno-pointer-sign)

# Game: thư viện chia sẻ "main" được NativeActivity nạp (xem AndroidManifest.xml)
add_library(main SHARED "${GAME_SOURCE}")
target_link_libraries(main PRIVATE raylib_android native_app_glue android log EGL GLESv2 OpenSLES dl m)
set_target_properties(main PROPERTIES LINK_FLAGS "-u ANativeActivity_onCreate")
