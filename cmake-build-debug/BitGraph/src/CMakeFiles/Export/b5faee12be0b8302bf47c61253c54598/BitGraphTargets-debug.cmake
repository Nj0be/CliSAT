#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "BitGraph::utils" for configuration "Debug"
set_property(TARGET BitGraph::utils APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(BitGraph::utils PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libutils.a"
  )

list(APPEND _cmake_import_check_targets BitGraph::utils )
list(APPEND _cmake_import_check_files_for_BitGraph::utils "${_IMPORT_PREFIX}/lib/libutils.a" )

# Import target "BitGraph::bitscan" for configuration "Debug"
set_property(TARGET BitGraph::bitscan APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(BitGraph::bitscan PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libbitscan.a"
  )

list(APPEND _cmake_import_check_targets BitGraph::bitscan )
list(APPEND _cmake_import_check_files_for_BitGraph::bitscan "${_IMPORT_PREFIX}/lib/libbitscan.a" )

# Import target "BitGraph::graph" for configuration "Debug"
set_property(TARGET BitGraph::graph APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(BitGraph::graph PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libgraph.a"
  )

list(APPEND _cmake_import_check_targets BitGraph::graph )
list(APPEND _cmake_import_check_files_for_BitGraph::graph "${_IMPORT_PREFIX}/lib/libgraph.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
