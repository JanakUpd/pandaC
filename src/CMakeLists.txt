add_library(compiler ${CMAKE_SOURCE_DIR}/src/compiler/compiler.cpp
        ${CMAKE_SOURCE_DIR}/src/compiler/codeconvertion.cpp
        ${CMAKE_SOURCE_DIR}/src/compiler/preprocessor.cpp
)
add_library(notifier ${CMAKE_SOURCE_DIR}/src/notifier/notifier.cpp)

add_library(clock ${CMAKE_SOURCE_DIR}/src/clock/clock.cpp)

target_include_directories(compiler PUBLIC ${CMAKE_SOURCE_DIR}/include/compiler)
target_include_directories(notifier PUBLIC ${CMAKE_SOURCE_DIR}/include/notifier)
target_include_directories(clock PUBLIC ${CMAKE_SOURCE_DIR}/include/clock)

target_link_libraries(compiler PUBLIC notifier)

