cmake_minimum_required(VERSION 3.1)

project(SBFspot)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules
        CACHE STRING "CMake module path")

find_package(PkgConfig REQUIRED)
find_package(Boost REQUIRED COMPONENTS system date_time)
find_package(CURL REQUIRED)
pkg_check_modules (BLUEZ REQUIRED bluez)

set(SRC_NOSQL
        ArchData.cpp
        Bluetooth.cpp
        CSVexport.cpp
        Ethernet.cpp
        EventData.cpp
        SBFNet.cpp
        SBFspot.cpp
        TagDefs.cpp
        boost_ext.cpp
        influxdb.cpp
        misc.cpp
        mqtt.cpp
        sunrise_sunset.cpp
        )

set(SRC_SQLITE
        ${SRC_NOSQL}
        db_SQLite.cpp
        db_SQLite_Export.cpp
        )

set(SRC_MYSQL
        ${SRC_NOSQL}
        db_MySQL.cpp
        db_MySQL_Export.cpp
        )

set(SRC_MARIADB
        ${SRC_MYSQL}
        )


add_executable(
        ${PROJECT_NAME}
        ${SRC_NOSQL}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} ${CURL_LIBRARIES} ${BLUEZ_LINK_LIBRARIES})

add_executable(influxdb-test
        influxdb.cpp
        influxdb.test.cpp
)

set_property(TARGET influxdb-test PROPERTY CXX_STANDARD 11)
target_link_libraries(influxdb-test ${CURL_LIBRARIES})

include(GNUInstallDirs)
set(bin_dir "${CMAKE_INSTALL_BINDIR}/sbfspot.3")
file(GLOB txt_files TagList*.txt)
install(TARGETS ${PROJECT_NAME} DESTINATION ${bin_dir})
install(FILES
        date_time_zonespec.csv
        ${txt_files}
        DESTINATION ${bin_dir})