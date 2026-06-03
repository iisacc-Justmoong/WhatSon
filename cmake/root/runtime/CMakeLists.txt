if (TARGET WhatSon)
    add_custom_target(whatson_run_app
            COMMAND "$<TARGET_FILE:WhatSon>"
            DEPENDS WhatSon
            USES_TERMINAL
            COMMENT "Run WhatSon app on current host"
    )
endif ()

if (TARGET whatson_cli AND DEFINED WHATSON_CLI_BINARY_PATH AND NOT WHATSON_CLI_BINARY_PATH STREQUAL "")
    add_custom_target(whatson_run_cli
            COMMAND "${WHATSON_CLI_BINARY_PATH}"
            DEPENDS whatson_cli
            USES_TERMINAL
            COMMENT "Run Rust CLI launcher (whatson)"
    )
endif ()

if (TARGET WhatSon_daemon)
    add_custom_target(whatson_run_daemon
            COMMAND "$<TARGET_FILE:WhatSon_daemon>"
            DEPENDS WhatSon_daemon
            USES_TERMINAL
            COMMENT "Run WhatSon daemon on current host"
    )

    add_custom_target(whatson_healthcheck_daemon
            COMMAND "$<TARGET_FILE:WhatSon_daemon>" --healthcheck
            DEPENDS WhatSon_daemon
            USES_TERMINAL
            COMMENT "Run daemon healthcheck"
    )
endif ()

if (TARGET launch_WhatSon_wasm)
    add_custom_target(whatson_launch_wasm DEPENDS launch_WhatSon_wasm)
endif ()

if (TARGET export_WhatSon_wasm_site)
    add_custom_target(whatson_export_wasm_site DEPENDS export_WhatSon_wasm_site)
endif ()
