cmake_minimum_required(VERSION 3.10.0)
project(BendSubspace VERSION 0.1.0)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(./cmake/configure_movecmd.cmake)
set(CMAKE_CXX_STANDARD 20)


set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(./includes)


if(WIN32)
	include_directories(includes)
	link_directories(lib)
    add_definitions(-D _USE_MATH_DEFINES)
    add_definitions(-D _CRT_NO_VA_START_VALIDATION)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 /bigobj")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4061 /wd4244 /wd4246 /wd4305 /wd4267 /wd4711 /wd4710 /wd4514 /wd4477")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

include_directories(./src)
include_directories(./imgui)

find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
    include_directories(${OpenGL_INCLUDE_DIRS})
    link_directories(${OpenGL_LIBRARY_DIRS})
    add_definitions(${OpenGL_DEFINITIONS})
endif()

find_package(GLFW3 REQUIRED)

# find static glew
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/includes) # for glew configuration
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib) # for glew configuration

set(GLEW_USE_STATIC_LIBS True)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
find_package(GLEW REQUIRED glew)
if(GLEW_FOUND)
	link_directories(${GLEW_LIBRARY_DIRS})
	include_directories(${GLEW_INCLUDE_DIRS})
endif()
set(GL_LIBS ${GLFW3_LIBRARY} ${GLEW_STATIC_LIBRARIES} ${OPENGL_LIBRARIES})
add_subdirectory(./src/utils)
# add_subdirectory(./src/cameras)
add_subdirectory(./src/render)
add_subdirectory(./src/bending)
add_subdirectory(./imgui)

set(libs utils_lib ${OpenCV_LIBS} imgui_lib render_lib bending_lib)


add_executable(main ./main.cpp)
target_link_libraries(main ${GL_LIBS} ${libs})

if(WIN32)
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        add_custom_command(
            TARGET main POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rename
                ${CMAKE_CURRENT_BINARY_DIR}/Debug/main.exe
                ${CMAKE_SOURCE_DIR}/main.exe
        )
    else()
        add_custom_command(
            TARGET main POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rename
                ${CMAKE_CURRENT_BINARY_DIR}/Release/main.exe
                ${CMAKE_SOURCE_DIR}/main.exe
        )
    endif()
else()
    add_custom_command(
        TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rename
            ${CMAKE_CURRENT_BINARY_DIR}/main        
            ${CMAKE_SOURCE_DIR}/main    
    )
endif()