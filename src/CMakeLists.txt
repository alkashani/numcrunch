set(SOURCE
    main.c
    clenshaw.c
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/_bin)
add_executable(clenshaw ${SOURCE})
