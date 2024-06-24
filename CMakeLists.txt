cmake_minimum_required(VERSION 3.7)

project(cbdraw LANGUAGES C VERSION 0.1.0)

# Compile and link flags
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")

# Modules
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(SDL_STATIC ON)
add_subdirectory(SDL)

set(BUILD_SHARED_LIBS OFF)
add_subdirectory(SDL_image)

# Environment defines
if (CMAKE_BUILD_TYPE MATCHES "Debug")
	set(DEBUG 1)
endif ()

file(GLOB SOURCE_FILES CONFIGURE_DEPENDS src/*.c)

add_executable(cbdraw ${SOURCE_FILES})
target_link_libraries(cbdraw
	SDL3-static
	SDL3_image-static
)
target_include_directories(cbdraw PRIVATE
	SDL/include/SDL3
	SDL_image/include/SDL3_image
)
