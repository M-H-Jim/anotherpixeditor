cmake_minimum_required(VERSION 4.0)
project(anotherpixeditor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")

find_package(FLTK REQUIRED)  

file(GLOB SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

include_directories(${FLTK_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME}
    ${FLTK_LIBRARIES}  
    comctl32
    user32
    gdi32
    ws2_32
    comdlg32
    ole32
    oleaut32
    uuid
    advapi32
    shell32
    gdiPlus
)


# cmake -G Ninja -B build -DCMAKE_PREFIX_PATH="E:/fltk-1.4.4/build"
