
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
  app.c
#  adc_dac.c
#  ring_buffer.c
)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})


target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
  # RSW - uncomment this when using the test pulse generator on PC0 for verification
  # Enables a state machine in app_loop_bottom to drive pin
  USE_TEST_PULSE
)


# Serial debug monitor 
add_subdirectory(dbgmon)


