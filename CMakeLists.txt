# This file configures the "build" of this SKSE plugin
# - how to compile the C++ plugin (in addition to settings found in CMakePresets.json)
# - where to output the resulting .dll file

# It's recommended to set a minimum CMake version.
# If you use CMake features from higher versions, update this to match.
cmake_minimum_required(VERSION 3.21)

# [BEGIN set_project_name]
# Set your project name. This will be the name of your SKSE .dll file.
project(HelloLua VERSION 0.0.1 LANGUAGES CXX C)
# [END set_project_name]

# #
# YOU DO NOT NEED TO EDIT ANYTHING BELOW HERE
# #

# If you're not using a mod manager, you probably want the SKSE plugin to go
# inside of your Skyrim "Data" folder.
#
# To do this automatically, set the `SKYRIM_FOLDER` environment variable
# to the path of your Skyrim Special Edition folder
if(DEFINED ENV{SKYRIM_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_FOLDER}/Data")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_FOLDER}/Data")
endif()

# Setup Lua paths
if(DEFINED ENV{LUA_HOME})
    set(LUA_HOME $ENV{LUA_HOME})
else()
    # Use the Lua submodule in the ext directory
    set(LUA_HOME "${CMAKE_CURRENT_SOURCE_DIR}/ext/lua")
endif()

message(STATUS "Lua Home: ${LUA_HOME}")
set(LUA_INCLUDE_DIR "${LUA_HOME}")
set(LUA_SRC_DIR "${LUA_HOME}")

# Check if source directory exists
if(NOT EXISTS "${LUA_SRC_DIR}")
    message(FATAL_ERROR "Lua source directory not found at ${LUA_SRC_DIR}. Please make sure the ext/lua submodule is properly initialized or the LUA_HOME environment variable points to a directory containing Lua source files.")
endif()

# List of Lua source files to compile - explicitly list core files to avoid issues
set(LUA_SOURCES
    "${LUA_SRC_DIR}/lapi.c"
    "${LUA_SRC_DIR}/lauxlib.c"
    "${LUA_SRC_DIR}/lbaselib.c"
    "${LUA_SRC_DIR}/lcode.c"
    "${LUA_SRC_DIR}/lcorolib.c" 
    "${LUA_SRC_DIR}/lctype.c"
    "${LUA_SRC_DIR}/ldblib.c"
    "${LUA_SRC_DIR}/ldebug.c"
    "${LUA_SRC_DIR}/ldo.c"
    "${LUA_SRC_DIR}/ldump.c"
    "${LUA_SRC_DIR}/lfunc.c"
    "${LUA_SRC_DIR}/lgc.c"
    "${LUA_SRC_DIR}/linit.c"
    "${LUA_SRC_DIR}/liolib.c"
    "${LUA_SRC_DIR}/llex.c"
    "${LUA_SRC_DIR}/lmathlib.c"
    "${LUA_SRC_DIR}/lmem.c"
    "${LUA_SRC_DIR}/loadlib.c"
    "${LUA_SRC_DIR}/lobject.c"
    "${LUA_SRC_DIR}/lopcodes.c"
    "${LUA_SRC_DIR}/loslib.c"
    "${LUA_SRC_DIR}/lparser.c"
    "${LUA_SRC_DIR}/lstate.c"
    "${LUA_SRC_DIR}/lstring.c"
    "${LUA_SRC_DIR}/lstrlib.c"
    "${LUA_SRC_DIR}/ltable.c"
    "${LUA_SRC_DIR}/ltablib.c"
    "${LUA_SRC_DIR}/ltm.c"
    "${LUA_SRC_DIR}/lundump.c"
    "${LUA_SRC_DIR}/lutf8lib.c"
    "${LUA_SRC_DIR}/lvm.c"
    "${LUA_SRC_DIR}/lzio.c"
)

message(STATUS "Lua source files: ${LUA_SOURCES}")

# Create a static library for Lua - name it lua_static to avoid conflicts
add_library(lua_static STATIC ${LUA_SOURCES})

# Configure the Lua static library
target_include_directories(lua_static PUBLIC "${LUA_SRC_DIR}")
target_compile_definitions(lua_static PRIVATE 
    _CRT_SECURE_NO_WARNINGS
    LUA_COMPAT_ALL
    # Removed LUA_USE_WINDOWS as it's unnecessary for Windows-only builds
)

# Apply C language settings to Lua source files
set_target_properties(lua_static PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Setup your SKSE plugin
find_package(CommonLibSSE CONFIG REQUIRED)

# Add the plugin target with updated source paths
add_commonlibsse_plugin(${PROJECT_NAME} 
    SOURCES
        src/Main.cpp
        src/Core/Papyrus.cpp
        src/Core/LuaManager.cpp
        src/Core/SKSEManager.cpp
        include/Core/Papyrus.h
        include/Core/LuaManager.h
        include/Core/SKSEManager.h
)

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${LUA_SRC_DIR}  # Use direct source directory for include
)

# Add definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    _CRT_SECURE_NO_WARNINGS
)

# Link against our Lua static library - it's important this comes first
target_link_libraries(${PROJECT_NAME} PRIVATE lua_static)

# Set C++ version and PCH
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_precompile_headers(${PROJECT_NAME} PRIVATE include/Core/PCH.h)

# Deployment settings for copying the plugin and scripts
if(DEFINED OUTPUT_FOLDER)
    # Copy the SKSE plugin .dll files into the SKSE/Plugins/ folder
    set(DLL_FOLDER "${OUTPUT_FOLDER}/SKSE/Plugins")

    message(STATUS "SKSE plugin output folder: ${DLL_FOLDER}")

    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${DLL_FOLDER}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        VERBATIM
    )

    # If you perform a "Debug" build, also copy .pdb file (for debug symbols)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_custom_command(
            TARGET "${PROJECT_NAME}"
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
            VERBATIM
        )
    endif()

    # Create scripts folder and copy example Lua scripts
    set(SCRIPTS_FOLDER "${OUTPUT_FOLDER}/SKSE/Plugins/Scripts")
    
    # Dynamically find all Lua script files in the Scripts directory
    file(GLOB LUA_SCRIPT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Scripts/*.lua")
    
    # First, create the scripts directory
    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${SCRIPTS_FOLDER}"
        VERBATIM
    )
    
    # Then, for each Lua script file found, add a command to copy it
    foreach(SCRIPT_FILE ${LUA_SCRIPT_FILES})
        get_filename_component(SCRIPT_FILENAME ${SCRIPT_FILE} NAME)
        add_custom_command(
            TARGET "${PROJECT_NAME}"
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different 
                    "${SCRIPT_FILE}"
                    "${SCRIPTS_FOLDER}/${SCRIPT_FILENAME}"
            VERBATIM
        )
    endforeach()
      # init.lua generation removed - package path handling is done in LuaManager.cpp
endif()
