function(enable_sanitizers target)
  if(MSVC)
    return()
  endif()

  if(ENABLE_ASAN)
    target_compile_options(${target} PRIVATE -fsanitize=address)
    target_link_options(${target} PRIVATE -fsanitize=address)
  endif()

  if(ENABLE_UBSAN)
    target_compile_options(${target} PRIVATE -fsanitize=undefined)
    target_link_options(${target} PRIVATE -fsanitize=undefined)
  endif()

  # Helpful for debug builds
  target_compile_definitions(${target} PRIVATE
    $<$<CONFIG:Debug>:BYOREDIS_DEBUG=1>
  )
endfunction()
