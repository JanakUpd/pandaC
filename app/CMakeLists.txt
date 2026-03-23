add_executable(pandaC main.cpp)
target_link_libraries(pandaC PRIVATE compiler)
target_link_libraries(pandaC PRIVATE notifier)
target_link_libraries(pandaC PRIVATE clock)