find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
    add_definitions(-D EnableOMP)
    message(STATUS "OpenMP enabled")
endif()

set(
    bending_src
    BendingGui.cpp
    load/BendingData.cpp
    load/BendingDataCloth.cpp
    load/BendingDataLoader.cpp
    bezier/BezierCurve.cpp
    bezier/BezierCurvePhysics.cpp
    bezier/BendingStiffness.cpp
)

add_library(bending_lib ${bending_src})