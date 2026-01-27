cmake_minimum_required(VERSION 3.18)
project(minimal_libtorch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_SOURCE_DIR}/cmake/depencency_install.cmake)

dependency_install("libtorch" "${CMAKE_SOURCE_DIR}/libtorch" "libtorch-install.py" "2.5.0")

find_package(Torch REQUIRED PATHS "${CMAKE_SOURCE_DIR}/libtorch")
if (Torch_FOUND)
    message(STATUS "${TORCH_INSTALL_PREFIX} Found")
    set(TORCH_INCLUDE "${TORCH_INSTALL_PREFIX}/include")
endif()

add_executable(minimal ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_link_libraries(minimal "${TORCH_LIBRARIES}")
target_include_directories(minimal PUBLIC ${TORCH_INCLUDE})

add_executable(pca ${CMAKE_SOURCE_DIR}/src/pca.cpp)
target_link_libraries(pca "${TORCH_LIBRARIES}")
target_include_directories(pca PUBLIC ${TORCH_INCLUDE})

add_executable(pca_k-means ${CMAKE_SOURCE_DIR}/src/pca_k-means.cpp)
target_link_libraries(pca_k-means "${TORCH_LIBRARIES}")
target_include_directories(pca_k-means PUBLIC ${TORCH_INCLUDE})


