cmake_minimum_required (VERSION 3.8)
project (NetworkProject)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_DEBUG_LIBS ON)  
set(Boost_USE_MULTITHREADED ON)



find_package(Boost 1.87 REQUIRED COMPONENTS thread filesystem system)
if(Boost_FOUND)
    message(STATUS "Boost version: ${Boost_VERSION}")
    message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
endif()

add_executable ( Frends2 "Frends2.cpp" "Frends2.h")

target_link_libraries(Frends2 PRIVATE 
    Boost::thread 
    Boost::filesystem 
    Boost::chrono
    Boost::system
)

add_executable(Cerver "Cerver.cpp")
target_link_libraries(Cerver PRIVATE
    Boost::system)

if (MSVC)
    target_compile_options(Frends2 PRIVATE /std:c++17 /permissive-)
    target_compile_options(Cerver PRIVATE /std:c++17 /permissive-)
else()
    target_compile_options(Frends2 PRIVATE -std=c++17)
    target_compile_options(Cerver PRIVATE -std=c++17)
endif()




