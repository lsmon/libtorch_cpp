cmake_minimum_required(VERSION 3.18)
project(minimal_libtorch LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OCID_DSET_PATH "${CMAKE_SOURCE_DIR}/data/ocid")
set(CSV_FILES   ${OCID_DSET_PATH}/310.csv
                ${OCID_DSET_PATH}/312.csv
                ${OCID_DSET_PATH}/313.csv
                ${OCID_DSET_PATH}/314.csv)

include(${CMAKE_SOURCE_DIR}/cmake/depencency_install.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/fetch_mnist.cmake)

set(CASSANDRA_V "2.17.1")
install("Cassandra" "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/scripts/dependencies/cassandra-driver.py" ${CASSANDRA_V})

set(LIBTORCH_V "2.5.0")
dependency_install("libtorch" "${CMAKE_SOURCE_DIR}/libtorch" "libtorch-install.py" "${LIBTORCH_V}")

# Silences the Kineto warning
set(kineto_LIBRARY "")
set(KINETO_LIBRARY "")

cmake_policy(SET CMP0144 NEW)
set(Torch_ROOT "${CMAKE_SOURCE_DIR}/libtorch")
message(STATUS "System Architecre: " ${CMAKE_SYSTEM_PROCESSOR})
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    set(TORCH_LIBTORCH "${Torch_ROOT}/lib")
    find_package(Torch REQUIRED PATHS "${CMAKE_SOURCE_DIR}/libtorch")
elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    set(TORCH_LIBTORCH "${Torch_ROOT}/lib.torch")
    find_package(Torch REQUIRED PATHS "${CMAKE_SOURCE_DIR}/libtorch")
endif()

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


set(CASSANDRA_INC ${CMAKE_SOURCE_DIR}/include/cassandra.h)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(CASSANDRA_LIB
        ${CMAKE_SOURCE_DIR}/lib/libcassandra.dylib
        ${CMAKE_SOURCE_DIR}/lib/libcassandra.2.dylib
        ${CMAKE_SOURCE_DIR}/lib/libcassandra.${CASSANDRA_V}.dylib)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(CASSANDRA_LIB
        ${CMAKE_SOURCE_DIR}/lib/libcassandra.so
        ${CMAKE_SOURCE_DIR}/lib/libcassandra.so.2
        ${CMAKE_SOURCE_DIR}/lib/libcassandra.so.${CASSANDRA_V})
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

else()

endif()

find_library(CASSANDRA_LIB_PATH
    NAMES cassandra
    PATHS "${CMAKE_SOURCE_DIR}/lib"
    NO_DEFAULT_PATH
)

if(NOT CASSANDRA_LIB_PATH)
    message(FATAL_ERROR "Cassandra library not found in ${CMAKE_SOURCE_DIR}/lib")
endif()

# --- FIX: Detect if Cassandra is static or shared to avoid linker errors ---
if(CASSANDRA_LIB_PATH MATCHES "${CMAKE_STATIC_LIBRARY_SUFFIX}$")
    set(CASSANDRA_LIB_TYPE STATIC)
else()
    set(CASSANDRA_LIB_TYPE SHARED)
endif()

find_package(nlohmann_json 3.10 REQUIRED)

if (NOT nlohmann_json_FOUND)
    message(FATAL_ERROR "nlohmann_json library not found")
else()
    message(STATUS "nlohmann_json found: ${nlohmann_json_VERSION}")
    message(STATUS "nlohmann_json include dirs: ${nlohmann_json_INCLUDE_DIRS}")
    message(STATUS "nlohmann_json libraries: ${nlohmann_json_LIBRARIES}")
endif()

configure_file(${CMAKE_SOURCE_DIR}/include/config.h.in ${CMAKE_SOURCE_DIR}/include/config.hpp)

add_library(cassandra_lib ${CASSANDRA_LIB_TYPE} IMPORTED)
set_target_properties(cassandra_lib PROPERTIES 
    IMPORTED_LOCATION "${CASSANDRA_LIB_PATH}"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/include"
)

add_executable(minimal ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_link_directories(minimal PRIVATE "${CMAKE_PREFIX_PATH}/torch.libs" "${CMAKE_PREFIX_PATH}/torch/libs")
target_link_libraries(minimal "${TORCH_LIBRARIES}")
target_include_directories(minimal PUBLIC ${TORCH_INCLUDE} ${CMAKE_SOURCE_DIR}/include)

add_executable(pca ${CMAKE_SOURCE_DIR}/src/unsupervised/pca.cpp)
target_link_directories(pca PRIVATE "${CMAKE_PREFIX_PATH}/torch.libs" "${CMAKE_PREFIX_PATH}/torch/libs")
target_link_libraries(pca "${TORCH_LIBRARIES}")
target_include_directories(pca PUBLIC ${TORCH_INCLUDE} ${CMAKE_SOURCE_DIR}/include)

add_executable(pca_k-means ${CMAKE_SOURCE_DIR}/src/unsupervised/pca_k-means.cpp)
target_link_directories(pca_k-means PRIVATE "${CMAKE_PREFIX_PATH}/torch.libs" "${CMAKE_PREFIX_PATH}/torch/libs")
target_link_libraries(pca_k-means "${TORCH_LIBRARIES}")
target_include_directories(pca_k-means PUBLIC ${TORCH_INCLUDE} ${CMAKE_SOURCE_DIR}/include)

fetch_mnist("${CMAKE_SOURCE_DIR}/data")
add_executable(No01_libtorch_basics ${CMAKE_SOURCE_DIR}/src/basics/libtorch.cpp)
target_link_directories(No01_libtorch_basics PRIVATE "${CMAKE_PREFIX_PATH}/torch.libs" "${CMAKE_PREFIX_PATH}/torch/libs")
target_link_libraries(No01_libtorch_basics "${TORCH_LIBRARIES}")
target_include_directories(No01_libtorch_basics PUBLIC ${TORCH_INCLUDE} ${CMAKE_SOURCE_DIR}/include)

add_library(cass_con ${CMAKE_SOURCE_DIR}/include/db/connector.hpp
                     ${CMAKE_SOURCE_DIR}/src/db/connector.cpp)
target_include_directories(cass_con PUBLIC ${CMAKE_SOURCE_DIR}/include ${CASSANDRA_INC})
target_link_libraries(cass_con ${CASSANDRA_LIB})

add_executable(db_test ${CMAKE_SOURCE_DIR}/test/db/db_test.cpp)
target_link_libraries(db_test cass_con)

add_executable(insert_test  ${CMAKE_SOURCE_DIR}/test/db/access/insert_test.cpp
                            ${CMAKE_SOURCE_DIR}/include/db/access/measurement.hpp
                            ${CMAKE_SOURCE_DIR}/src/db/access/measurement.cpp
                            ${CMAKE_SOURCE_DIR}/include/db/connector.hpp
                            ${CMAKE_SOURCE_DIR}/src/db/connector.cpp)
target_include_directories(insert_test PUBLIC ${CMAKE_SOURCE_DIR}/include ${CASSANDRA_INC})
target_link_libraries(insert_test PRIVATE cass_con nlohmann_json::nlohmann_json uv)

add_executable(insert_csv   ${CMAKE_SOURCE_DIR}/test/db/access/insert_csv.cpp
                            ${CMAKE_SOURCE_DIR}/include/db/access/measurement.hpp
                            ${CMAKE_SOURCE_DIR}/src/db/access/measurement.cpp
                            ${CMAKE_SOURCE_DIR}/include/db/connector.hpp
                            ${CMAKE_SOURCE_DIR}/src/db/connector.cpp)
target_include_directories(insert_csv PUBLIC ${CMAKE_SOURCE_DIR}/include ${CASSANDRA_INC})
target_link_libraries(insert_csv cass_con)