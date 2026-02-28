find_package(Qt6 6.5 REQUIRED COMPONENTS Core)

qt_add_executable(WhatSon_daemon
        main.cpp
)

target_link_libraries(WhatSon_daemon
        PRIVATE
        Qt6::Core
)
