cmake_minimum_required(VERSION 3.18)
project(minimal_libtorch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

configure_file(${CMAKE_SOURCE_DIR}/include/config.h.in ${CMAKE_SOURCE_DIR}/include/config.hpp)


include(${CMAKE_SOURCE_DIR}/cmake/depencency_install.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/fetch_mnist.cmake)

dependency_install("libtorch" "${CMAKE_SOURCE_DIR}/libtorch" "libtorch-install.py" "2.5.0")
# Silences the Kineto warning
set(kineto_LIBRARY "")
set(KINETO_LIBRARY "")
find_package(Torch REQUIRED PATHS "${CMAKE_SOURCE_DIR}/libtorch")

set(TORCH_ROOT "${CMAKE_SOURCE_DIR}/libtorch")
set(TORCH_LIBTORCH "${TORCH_ROOT}/lib.torch")

if (Torch_FOUND)
    set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/libtorch")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
    message(STATUS "${TORCH_INSTALL_PREFIX} Found")
    message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
    message(STATUS "Torch compiler flags: ${TORCH_CXX_FLAGS}")
    message(STATUS "Torch libraries: ${TORCH_LIBRARIES}")
    message(STATUS "Torch include dirs: ${TORCH_INCLUDE_DIRS}")
    message(STATUS "Compiler flags: ${CMAKE_CXX_FLAGS}")
    
    set(TORCH_INCLUDE "${TORCH_INSTALL_PREFIX}/include")
    link_directories("${CMAKE_PREFIX_PATH}/torch.libs")
endif()

add_executable(minimal ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_link_directories(minimal PRIVATE "${CMAKE_PREFIX_PATH}/torch.libs" "${CMAKE_PREFIX_PATH}/torch/libs")
target_link_libraries(minimal "${TORCH_LIBRARIES}")
target_include_directories(minimal PUBLIC ${TORCH_INCLUDE} ${CMAKE_SOURCE_DIR}/include)
set_property(TARGET minimal PROPERTY CXX_STANDARD 17)

add_executable(pca ${CMAKE_SOURCE_DIR}/src/unsupervised/pca.cpp)
target_link_directories(pca PRIVATE "${CMAKE_PREFIX_PATH}/torch.libs" "${CMAKE_PREFIX_PATH}/torch/libs")
target_link_libraries(pca "${TORCH_LIBRARIES}")
target_include_directories(pca PUBLIC ${TORCH_INCLUDE} ${CMAKE_SOURCE_DIR}/include)
set_property(TARGET pca PROPERTY CXX_STANDARD 17)

add_executable(pca_k-means ${CMAKE_SOURCE_DIR}/src/unsupervised/pca_k-means.cpp)
target_link_directories(pca_k-means PRIVATE "${CMAKE_PREFIX_PATH}/torch.libs" "${CMAKE_PREFIX_PATH}/torch/libs")
target_link_libraries(pca_k-means "${TORCH_LIBRARIES}")
target_include_directories(pca_k-means PUBLIC ${TORCH_INCLUDE} ${CMAKE_SOURCE_DIR}/include)
set_property(TARGET pca_k-means PROPERTY CXX_STANDARD 17)

fetch_mnist("${CMAKE_SOURCE_DIR}/data")
add_executable(No01_libtorch_basics ${CMAKE_SOURCE_DIR}/src/basics/libtorch.cpp)
target_link_directories(No01_libtorch_basics PRIVATE "${CMAKE_PREFIX_PATH}/torch.libs" "${CMAKE_PREFIX_PATH}/torch/libs")
target_link_libraries(No01_libtorch_basics "${TORCH_LIBRARIES}")
target_include_directories(No01_libtorch_basics PUBLIC ${TORCH_INCLUDE} ${CMAKE_SOURCE_DIR}/include)
set_property(TARGET No01_libtorch_basics PROPERTY CXX_STANDARD 17)
