cmake_minimum_required(VERSION 3.7)

project(indicate LANGUAGES C CXX VERSION 1.0.0)

# Compile and link flags
# set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -fsanitize=address -fno-omit-frame-pointer")
# set(CMAKE_C_LINKER_FLAGS_DEBUG "${CMAKE_C_LINKER_FLAGS_DEBUG} -lasan -fsanitize=address -fno-omit-frame-pointer")
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -fsanitize=address -fno-omit-frame-pointer")
# set(CMAKE_CXX_LINKER_FLAGS_DEBUG "${CMAKE_CXX_LINKER_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")

# Modules
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(SDL_STATIC ON)
add_subdirectory(SDL)

set(BUILD_SHARED_LIBS OFF)
add_subdirectory(SDL_image)

set(IMGUI_STATIC ON CACHE STRING "Build as a static library" FORCE)
add_subdirectory(cimgui EXCLUDE_FROM_ALL)

# Environment defines
if (CMAKE_BUILD_TYPE MATCHES "Debug")
	set(DEBUG 1)
endif ()

add_definitions(
	-DIMGUI_DISABLE_OBSOLETE_KEYIO
	-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS
)

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
	PRIVATE
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
install(
	TARGETS ${PROJECT_NAME}
	EXPORT "${PROJECT_NAME}Targets"
	#RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	COMPONENT ${PROJECT_NAME}
)

# Packaging options
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "Linus Probert")
set(CPACK_PACKAGE_CONTACT "Linus Probert <linus.probert@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_DIRECTORY ${PROJECT_BINARY_DIR}/package)
set(CPACK_PACKAGE_VERSION_MAJOR ${breakhack_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${breakhack_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${breakhack_PATCH_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
set(CPACK_PACKAGE_CHECKSUM "MD5")

# DEB options
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Linus Probert")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_COMPONENT_INSTALL ON)

include(CPack)
