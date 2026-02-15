find_package(Qt6 6.5 REQUIRED COMPONENTS Core)

qt_add_executable(whats_on_daemon
        main.cpp
)

target_link_libraries(whats_on_daemon
        PRIVATE
        Qt6::Core
)
