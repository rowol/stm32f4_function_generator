
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
  app.c
  dac_dma.c
  sig_gen.c
)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})


# Serial debug monitor 
add_subdirectory(dbgmon)


