cmake_minimum_required(VERSION 3.7)

project(indicate LANGUAGES C CXX VERSION 0.1.0)

# Compile and link flags
# set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -fsanitize=address -fno-omit-frame-pointer")
# set(CMAKE_C_LINKER_FLAGS_DEBUG "${CMAKE_C_LINKER_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -fsanitize=address -fno-omit-frame-pointer")
# set(CMAKE_CXX_LINKER_FLAGS_DEBUG "${CMAKE_CXX_LINKER_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")

# Modules
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(SDL_STATIC ON)
add_subdirectory(SDL)

set(BUILD_SHARED_LIBS OFF)
add_subdirectory(SDL_image)

set(IMGUI_STATIC ON CACHE STRING "Build as a static library" FORCE)
add_subdirectory(cimgui)

# Environment defines
if (CMAKE_BUILD_TYPE MATCHES "Debug")
	set(DEBUG 1)
endif ()

add_definitions(-DIMGUI_DISABLE_OBSOLETE_KEYIO)

# Build the imgui backend (SDL3/sdl_renderer) lib
file(GLOB IMGUI_SRC CONFIGURE_DEPENDS cimgui/imgui/*.cpp)
add_library(imgui_impl STATIC
	${IMGUI_SRC}
	cimgui/imgui/backends/imgui_impl_sdlrenderer3.cpp
	cimgui/imgui/backends/imgui_impl_sdl3.cpp
)
target_include_directories(imgui_impl PRIVATE
	SDL/include
	cimgui/imgui
)
target_compile_definitions(imgui_impl PRIVATE IMGUI_IMPL_API=extern\ \"C\")

# Build the main project
add_executable(${PROJECT_NAME}
	src/main.c
	src/gui.c
	src/draw.c
)
target_link_libraries(${PROJECT_NAME}
	SDL3-static
	SDL3_image-static
	cimgui
	imgui_impl
)
target_include_directories(${PROJECT_NAME} PRIVATE
	SDL/include/SDL3
	SDL_image/include/SDL3_image
	cimgui
	cimgui/imgui/backends
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
	PROGRAM_NAME="${PROJECT_NAME}"
	PROGRAM_VERSION="${PROJECT_VERSION}"
	_DEBUG=${DEBUG}
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
