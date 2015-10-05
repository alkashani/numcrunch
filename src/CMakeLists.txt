set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/_bin)

add_library(clenshaw_scalar_lib clenshaw_scalar.c)
add_library(clenshaw_vector_lib clenshaw_vector.c)

add_executable(clenshaw_scalar main.c)
target_link_libraries(clenshaw_scalar clenshaw_scalar_lib)
add_executable(clenshaw_vector main.c)
target_link_libraries(clenshaw_vector clenshaw_vector_lib)
add_executable(clenshaw_thread main_thread.c)
target_link_libraries(clenshaw_thread clenshaw_vector_lib ${CMAKE_THREAD_LIBS_INIT})
