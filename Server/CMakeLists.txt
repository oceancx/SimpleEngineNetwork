# CMake entry point
cmake_minimum_required (VERSION 3.1)
project (Server)


# adding asio
add_definitions(-DASIO_STANDALONE)
add_definitions(-DASIO_HAS_STD_ATOMIC)
add_definitions(-DASIO_HAS_STD_SHARED_PTR) 
add_definitions(-DASIO_HAS_STD_FUNCTION) 
add_definitions(-DASIO_HAS_STD_ADDRESSOF) 
add_definitions(-DASIO_HAS_STD_ARRAY) 
add_definitions(-DASIO_HAS_VARIADIC_TEMPLATES)
add_definitions(-DASIO_HAS_STD_CHRONO)
add_definitions(-DASIO_HAS_CSTDINT)
add_definitions(-DASIO_HAS_STD_TYPE_TRAITS)
add_definitions(-std=c++11)
include_directories(SYSTEM /Users/oceancx/SimpleEngine/external/asio/asio/include)


add_executable(${PROJECT_NAME}
	main.cpp
	common.h
)

target_link_libraries(${PROJECT_NAME}
)
