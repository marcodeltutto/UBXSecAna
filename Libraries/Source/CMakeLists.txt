# Notice name prefix of this variable, set by CMake according
# to value given with "project()" in the root CMakeLists.txt.
include_directories(${UBXSecAna_SOURCE_DIR}/Libraries/Include)

ROOT_GENERATE_DICTIONARY(BootstrapTH1DClassDict 
                         ${UBXSecAna_SOURCE_DIR}/Libraries/Include/BootstrapTH1D.h 
                         MODULE BootstrapTH1DClass
                         LINKDEF ${UBXSecAna_SOURCE_DIR}/Libraries/Include/BootstrapTH1D_LinkDef.h)

add_library(BootstrapTH1DClass SHARED BootstrapTH1D.cxx BootstrapTH1DClassDict)

target_link_libraries(BootstrapTH1DClass ${ROOT_LIBRARIES})

#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libBootstrapTH1DClass_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libBootstrapTH1DClassDict.rootmap DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

if (${ROOT_VERSION} VERSION_GREATER "6.0")
   install(
      FILES
      ${CMAKE_CURRENT_BINARY_DIR}/libBootstrapTH1DClass_rdict.pcm
      ${CMAKE_CURRENT_BINARY_DIR}/libBootstrapTH1DClass.rootmap
      DESTINATION lib/)
endif (${ROOT_VERSION} VERSION_GREATER "6.0")

install(TARGETS BootstrapTH1DClass DESTINATION lib/)

add_library(Libraries   MigrationMatrix2D.cxx 
                        MigrationMatrix4D.cxx 
                        CrossSectionCalculator1D.cxx
                        CrossSectionCalculator2D.cxx
                        BootstrapTH1D.cxx
                        ReweightingPlotter.cxx
                        CorrelationCalculator2D.cxx
                        UBXSecEvent.C)
