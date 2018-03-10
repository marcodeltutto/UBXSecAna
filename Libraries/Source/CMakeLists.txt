# Notice name prefix of this variable, set by CMake according
# to value given with "project()" in the root CMakeLists.txt.
include_directories(${UBXSecAna_SOURCE_DIR}/Libraries/Include)
add_library(Libraries   MigrationMatrix2D.cxx 
                        MigrationMatrix4D.cxx 
                        CrossSectionCalculator2D.cxx
                        PlotHandler.cxx
                        AnaTree.C)
