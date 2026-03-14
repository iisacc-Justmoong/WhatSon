find_package(Qt6 6.5 REQUIRED COMPONENTS Core)

qt_add_executable(WhatSon_daemon
        main.cpp
)

target_link_libraries(WhatSon_daemon
        PRIVATE
        Qt6::Core
)

if (UNIX AND NOT APPLE)
    set_target_properties(WhatSon_daemon PROPERTIES
            INSTALL_RPATH "$ORIGIN/../lib")
endif ()
