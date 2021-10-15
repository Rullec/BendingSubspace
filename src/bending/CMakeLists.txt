set(
    bending_src
    BendingGui.cpp
    load/BendingData.cpp
    load/BendingDataCloth.cpp
    load/BendingDataLoader.cpp
)

add_library(bending_lib ${bending_src})