cmake_minimum_required (VERSION 2.8)

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O2 -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -g")
#SET(CMAKE_C_COMPILER g++)

add_compile_options(-std=c++11)

project (gameserver)
if (NOT SET_OUTPUT_DIRECTORY)
    set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "../bin/server")
    set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "./Share/llib")
    set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "./Share/llib")
    set (SET_OUTPUT_DIRECTORY ON)
endif (NOT SET_OUTPUT_DIRECTORY)

SET(SRC_LIST
GameServer/Main.cpp
GameServer/GameServer.cpp
GameServer/Config/GameServerConfig.cpp
GameServer/ClientConnection/ClientConnection
)

include_directories (
        "GameServer"
        "Share/Header"
        "Share/Header/Base"
        "Share/Header/GameServer"
)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/Share/llib)

ADD_EXECUTABLE(gameserver ${SRC_LIST})

add_subdirectory ("GameServerLogic")

target_link_libraries (gameserver
	daemon
	filelog
	filereader
	network
	ringbuffer
	gameserverlogic
	pthread
)

