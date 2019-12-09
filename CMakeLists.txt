cmake_minimum_required(VERSION 3.10)

project(sbfspot)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules
	CACHE STRING "CMake module path")

find_package(Boost REQUIRED COMPONENTS system date_time)
find_package(BLUETOOTH REQUIRED)
find_package(GTest)

set(SRC_NOSQL
        SBFspot/boost_ext.cpp
        SBFspot/misc.cpp
        SBFspot/sunrise_sunset.cpp
        SBFspot/SBFNet.cpp
        SBFspot/CSVexport.cpp
        SBFspot/Ethernet.cpp
        SBFspot/EventData.cpp
        SBFspot/ArchData.cpp
        SBFspot/SBFspot.cpp
        SBFspot/TagDefs.cpp
        SBFspot/Bluetooth.cpp
        SBFspot/mqtt.cpp
)

set(SRC_SQLITE
        ${SRC_NOSQL}
        SBFspot/db_SQLite.cpp
        SBFspot/db_SQLite_Export.cpp
)

set(SRC_MYSQL
        ${SRC_NOSQL}
        SBFspot/db_MySQL.cpp
        SBFspot/db_MySQL_Export.cpp
)

set(SRC_MARIADB
        ${SRC_MYSQL}
)


add_executable(
        ${PROJECT_NAME}
        ${SRC_NOSQL}
)

target_link_libraries(${PROJECT_NAME} ${LIBBLUETOOTH_LIBRARIES} ${Boost_LIBRARIES})
