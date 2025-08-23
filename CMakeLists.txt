cmake_minimum_required (VERSION 3.6)

project (Toool)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/.)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

#set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE STRING "Configuration types" FORCE)

file(GLOB SRC_FILES
    src/*.h
    src/*.cpp
    src/*.c
)

file(GLOB SYS_FILES
    src/sys/*.h
    src/sys/*.cpp
)

if(WIN32)
    set(PLATFORM_FILES
        src/sys/_windows/msys_threadOS.cpp
        src/sys/_windows/msys_soundOS.cpp
    )
    set(PLATFORM_FILES ${PLATFORM_FILES} "src/sys/_windows/main_deb.cpp" "src/sys/_windows/msys_debugOS.cpp")
    add_definitions(-DWINDOWS=1)
    add_definitions(-DA64BITS=1)
    add_definitions(-DDEBUG=1)
    add_definitions(-DNOMINMAX)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(EXT_LIBRARIES "opengl32.lib" "winmm.lib" "Dsound.lib")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
elseif(APPLE)
    find_package(OpenGL REQUIRED)
    file(GLOB PLATFORM_FILES
        src/sys/_macos/*.h
        src/sys/_macos/*.cpp
        src/sys/_macos/*.mm
    )
    include_directories(${OPENGL_INCLUDE_DIRS})
else()
    file(GLOB PLATFORM_FILES
        src/sys/_linux/*.h
        src/sys/_linux/*.cpp
    )
endif()

include_directories("src/edit")

set(EXE_NAME "Toool")

include_directories("src/tools")
file(GLOB TOOLS_FILES
    src/tools/*.h
    src/tools/*.cpp
)
file(GLOB EDIT_FILES
    src/edit/*.h
    src/edit/*.cpp
)
if(WIN32)
    include_directories("src/tools/_windows")
    file(GLOB TOOLS_PLATFORM_FILES
        src/tools/_windows/*.h
        src/tools/_windows/*.cpp
    )
elseif(APPLE)
    include_directories("src/tools/_macos")
    file(GLOB TOOLS_PLATFORM_FILES
        src/tools/_macos/*.h
        src/tools/_macos/*.cpp
        src/tools/_macos/*.mm
    )
else()
    file(GLOB TOOLS_PLATFORM_FILES
        src/tools/_linux/*.h
        src/tools/_linux/*.cpp
    )
endif()
add_definitions(-DTOOLS=1)

include_directories("src")
include_directories("src/sys")
set(SOURCES ${SRC_FILES} ${SYS_FILES} ${PLATFORM_FILES} ${TOOLS_FILES} ${TOOLS_PLATFORM_FILES} ${EDIT_FILES})

if(WIN32)
    set(WIN32_EXECUTABLE TRUE)
    add_executable(${EXE_NAME} WIN32 ${SOURCES})
else()
    add_executable(${EXE_NAME} ${SOURCES})
endif()

target_link_libraries(${EXE_NAME} ${EXT_LIBRARIES})

set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin )
set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin )
set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin )
set_target_properties(${EXE_NAME} PROPERTIES DEBUG_POSTFIX "_d")
set_target_properties(${EXE_NAME} PROPERTIES RELWITHDEBINFO_POSTFIX "RelWithDebInfo")
set_target_properties(${EXE_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

if(APPLE)
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

#if(WIN32)
#    add_compile_options(/W4 /Zi)  # Add /Zi for debug information
#endif()

if(APPLE)
    target_link_libraries(${EXE_NAME} "-framework CoreFoundation")
    target_link_libraries(${EXE_NAME} "-framework AppKit")
    target_link_libraries(${EXE_NAME} "-framework OpenGL")
    target_link_libraries(${EXE_NAME} "-framework GameController")
target_link_libraries(${EXE_NAME} "-framework AudioToolbox")
else()
    #foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
    #                 CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    #  string(REPLACE /GR /GR- "${flag_var}" "${${flag_var}}")
    #  string(REPLACE /EHsc /EHsc- "${flag_var}" "${${flag_var}}")
    #endforeach()

    #add_definitions(-D_HAS_EXCEPTIONS=0)
    #set(CMAKE_CXX_FLAGS_DEBUG "-g")
    #set(CMAKE_CXX_FLAGS_RELEASE "-O1 -Os -GL")
    target_link_options(${EXE_NAME} PRIVATE /machine:x64)
endif()

source_group(msys FILES ${SYS_FILES} ${TOOLS_PLATFORM_FILES} ${PLATFORM_FILES})
source_group(edit FILES ${EDIT_FILES})
source_group(demo FILES ${SRC_FILES})
