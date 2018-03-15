# Notice name prefix of this variable, set by CMake according
# to value given with "project()" in the root CMakeLists.txt.
include_directories(${UBXSecAna_SOURCE_DIR}/Libraries/Include)

ROOT_GENERATE_DICTIONARY(BootstrapTH1DClassDict ${UBXSecAna_SOURCE_DIR}/Libraries/Include/BootstrapTH1D.h LINKDEF ${UBXSecAna_SOURCE_DIR}/Libraries/Include/BootstrapTH1D_LinkDef.h)

add_library(BootstrapTH1DClass SHARED BootstrapTH1D.cxx BootstrapTH1DClassDict.cxx)

target_link_libraries(BootstrapTH1DClass ${ROOT_LIBRARIES})

add_library(Libraries   MigrationMatrix2D.cxx 
                        MigrationMatrix4D.cxx 
                        CrossSectionCalculator1D.cxx
                        CrossSectionCalculator2D.cxx
                        BootstrapTH1D.cxx
                        UBXSecEvent.C)
