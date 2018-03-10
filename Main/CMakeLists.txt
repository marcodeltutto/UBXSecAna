
include_directories(${UBXSecAna_SOURCE_DIR}/Libraries/Include)

link_directories(${UBXSecAna_SOURCE_DIR}/Libraries/Source)

add_executable(Maker Maker.cpp)
target_link_libraries(Maker Libraries)
target_link_libraries(Maker ${ROOT_LIBRARIES})

add_executable(Analyse Analyse.cpp)
target_link_libraries(Analyse Libraries)
target_link_libraries(Analyse ${ROOT_LIBRARIES})
