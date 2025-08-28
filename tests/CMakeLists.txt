enable_testing()

add_executable(client_tests
  client_tests.cpp
)
target_link_libraries(client_tests PRIVATE
  byoredis_core
  GTest::gtest_main
)
enable_sanitizers(client_tests)

include(GoogleTest)
gtest_discover_tests(client_tests)
