cmake_minimum_required (VERSION 3.23)

set(CMAKE_CXX_STANDARD 17)

project(stackable-functor-allocation)

add_executable(test_Interface)
target_include_directories(test_Interface PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_sources(test_Interface PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test/sfa.cpp)

add_library(${PROJECT_NAME} INTERFACE)
target_sources(${PROJECT_NAME}
    PUBLIC
      FILE_SET public_headers
      TYPE HEADERS
      FILES
        $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
)
install(TARGETS ${PROJECT_NAME} FILE_SET public_headers)
