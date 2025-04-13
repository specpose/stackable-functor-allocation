cmake_minimum_required (VERSION 3.23.5)

set(CMAKE_CXX_STANDARD 17)

project(stackable-functor-allocation LANGUAGES CXX)

option(SFA_BACKEND "CPU")
message("Backend is ${SFA_BACKEND}")

if (SFA_BACKEND STREQUAL "GOOPAX")
  find_package(goopax REQUIRED PATHS ../)
  add_executable(test_MOLE)
  target_link_libraries(test_MOLE PRIVATE goopax::goopax)
  target_sources(test_MOLE PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test/mole_gpx.cpp)
  add_executable(test_INV)
  target_include_directories(test_INV PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/)
  target_compile_definitions(test_INV PRIVATE GOOPAX=1)
  target_link_libraries(test_INV PRIVATE goopax::goopax)
  target_sources(test_INV PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test/invertable.cpp)
elseif(SFA_BACKEND STREQUAL "INTELDPCPP")
  find_package(IntelDPCPP REQUIRED)
  set(CMAKE_C_COMPILER "icx" CACHE "STRING" "")
  if (WIN32)
    set(CMAKE_CXX_COMPILER "icx" CACHE "STRING" "")
  else()
    set(CMAKE_CXX_COMPILER "icpx" CACHE "STRING" "")
  endif()
  add_executable(test_MOLE)
  target_sources(test_MOLE PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test/mole_sycl.cpp)
elseif(SFA_BACKEND STREQUAL "INTELSYCL")
  find_package(IntelSYCL REQUIRED)
  add_executable(test_MOLE)
  add_sycl_to_target(TARGET test_MOLE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test/mole_sycl.cpp)
  target_include_directories(test_MOLE PRIVATE ${SYCL_INCLUDE_DIR})
  target_link_libraries(test_MOLE PRIVATE ${SYCL_LIBRARY})
  target_sources(test_MOLE PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test/mole_sycl.cpp)
else()
  add_executable(test_SFA)
  target_include_directories(test_SFA PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/)
  target_sources(test_SFA PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test/sfa.cpp)
  add_executable(test_MOLE)
  target_sources(test_MOLE PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test/mole.cpp)
  add_executable(test_INV)
  target_include_directories(test_INV PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/)
  target_sources(test_INV PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test/invertable.cpp)
endif()
target_include_directories(test_MOLE PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/)

add_library(${PROJECT_NAME} INTERFACE)
target_sources(${PROJECT_NAME}
    PUBLIC
      FILE_SET public_headers
      TYPE HEADERS
      FILES
        $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
)
install(TARGETS ${PROJECT_NAME} FILE_SET public_headers)
