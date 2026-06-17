#I HATE CMAKE I HATE CMAKE I HATE CMAKE I HATE CMAKE I HATE CMAKE I HATE CMAKE I HATE CMAKE I HATE CMAKE I HATE CMAKE I HATE CMAKE I HATE CMAKE I HATE CMAKE I HATE CMAKE I HATE CMAKE I HATE CMAKE I HATE CMAKE I HATE CMAKE 
cmake_minimum_required(VERSION 3.10.0)
project(winmain VERSION 0.1.0 LANGUAGES C CXX ASM ASM_MASM)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
set(ASM src/utils/asm_gcc.S)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(ASM src/utils/asm_msvc.asm) 
endif()

add_library(
            IMGUI STATIC
            external/imgui/imgui.cpp
            external/imgui/imgui_demo.cpp
            external/imgui/imgui_stdlib.cpp
            external/imgui/imgui_draw.cpp
            external/imgui/imgui_tables.cpp
            external/imgui/imgui_widgets.cpp
            external/imgui/imgui_impl_win32.cpp
            external/imgui/imgui_impl_dx11.cpp)

target_include_directories(IMGUI PUBLIC external/imgui)
target_link_libraries(IMGUI PUBLIC d3d11 dxgi d3dcompiler dwmapi)
target_compile_options(IMGUI PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/O2>
    $<$<CXX_COMPILER_ID:GNU>:-O2>
)

set(
    SRC 
    src/utils/utils.cpp
    src/utils/vars.cpp
    src/utils/sys.cpp
    src/core/mem.cpp
    src/core/inj.cpp
    src/menu/menu.cpp
    src/menu/draw.cpp
    src/main.cpp)

add_executable(winbs WIN32 ${SRC} ${ASM})

target_include_directories(winbs PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(winbs PRIVATE IMGUI)
target_compile_features(winbs PRIVATE cxx_std_17)

target_compile_options(winbs PRIVATE
    # MSVC Debug
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:MSVC>>:/Od /Zi /RTC1>

    # MSVC Release
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/O2 /GL>

    # GCC Debug
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU>>:-O0 -g>

    # GCC Release
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU>>:-O2 -flto>
)

target_link_options(winbs PRIVATE
    # Windows subsystem
    $<$<CXX_COMPILER_ID:MSVC>:/SUBSYSTEM:WINDOWS>
    $<$<CXX_COMPILER_ID:GNU>:-mwindows>

    # MSVC LTO
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/LTCG>

    # GCC LTO
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU>>:-flto>

    # Static runtime (GCC)
    $<$<CXX_COMPILER_ID:GNU>:-static>
    $<$<CXX_COMPILER_ID:GNU>:-static-libgcc>
    $<$<CXX_COMPILER_ID:GNU>:-static-libstdc++>
)

target_compile_options(winbs PRIVATE
    $<$<AND:$<CONFIG:RelWithDebInfo>,$<CXX_COMPILER_ID:GNU>>:
        -O1
        -g
        -fsanitize=undefined
    >
)

target_link_options(winbs PRIVATE
    $<$<AND:$<CONFIG:RelWithDebInfo>,$<CXX_COMPILER_ID:GNU>>:
        -fsanitize=undefined
    >
)