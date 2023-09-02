cmake_minimum_required(VERSION 3.27.4)

project(portico)

set(CMAKE_CXX_COMPILER g++)

if(DEBUG)
    set(CMAKE_BUILD_TYPE Debug)
else()
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(libmysql REQUIRED)

add_executable(${PROJECT_NAME} core/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE ${MYSQL_LIBRARIES})

install (TARGETS ${PROJECT_NAME} DESTINATION bin)