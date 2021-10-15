set(
    bending_src
    BendingGui.cpp
    load/BendingData.cpp
    load/BendingDataCloth.cpp
    load/BendingDataLoader.cpp
    bezier/BezierCurve.cpp
    bezier/BezierCurvePhysics.cpp
)

add_library(bending_lib ${bending_src})