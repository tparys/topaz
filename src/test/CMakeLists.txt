if (CHECK_FOUND)

  # Build our unit test against check ...
  add_executable(unit_test unit_test.c)
  target_link_libraries(unit_test topaz ${CHECK_LDFLAGS})

  # Hook into cmake
  #enable_testing()
  #add_test(mytest unit_test)

endif (CHECK_FOUND)