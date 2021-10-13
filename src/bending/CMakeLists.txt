set(
    bending_src
    BendingGui.cpp
    load/BendingData.cpp
    load/BendingDataLoader.cpp
)

add_library(bending_lib ${bending_src})