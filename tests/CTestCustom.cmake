file (STRINGS "@CMAKE_BINARY_DIR@/CTestTestfile.cmake" LINES)

# overwrite the file....
file(WRITE "@CMAKE_BINARY_DIR@/CTestTestfile.cmake" "")

# loop through the lines,
foreach(LINE IN LISTS LINES)
  # remove unwanted parts
  string(REGEX REPLACE ".*directory_to_remove.*" "" STRIPPED "${LINE}")
  # and write the (changed) line ...
  file(APPEND "@CMAKE_BINARY_DIR@/CTestTestfile.cmake" "${STRIPPED}\n")
endforeach()