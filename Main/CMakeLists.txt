
include_directories(${UBXSecAna_SOURCE_DIR}/Libraries/Include)

link_directories(${UBXSecAna_SOURCE_DIR}/Libraries/Source)

add_executable(Maker Maker.cpp)
target_link_libraries(Maker Libraries)
target_link_libraries(Maker BootstrapTH1DClass ${ROOT_LIBRARIES})
#target_link_libraries(Maker BootstrapTH1DClass)

add_executable(Analyse Analyse.cpp)
target_link_libraries(Analyse Libraries)
target_link_libraries(Analyse BootstrapTH1DClass ${ROOT_LIBRARIES})
#target_link_libraries(Maker BootstrapTH1DClass)


install (TARGETS Maker Analyse DESTINATION bin)

