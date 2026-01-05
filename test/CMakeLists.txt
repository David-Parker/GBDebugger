# Data structures test
add_executable(DataStructuresTest DataStructuresTest.cpp)
target_link_libraries(DataStructuresTest GBDebugger)
target_include_directories(DataStructuresTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

add_test(NAME DataStructuresTest COMMAND DataStructuresTest)

# API layer test
add_executable(APILayerTest APILayerTest.cpp)
target_link_libraries(APILayerTest GBDebugger)
target_include_directories(APILayerTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

add_test(NAME APILayerTest COMMAND APILayerTest)
