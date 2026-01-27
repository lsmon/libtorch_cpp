function(dependency_install NAME CHECK_PATH SCRIPT_NAME VERSION)
    message(STATUS "python3 ${CMAKE_SOURCE_DIR}/scripts/dependencies/${SCRIPT_NAME} ${CMAKE_SOURCE_DIR} ${VERSION}")
    if(NOT EXISTS ${CHECK_PATH})
        message(STATUS "[INSTALL] ${NAME} not found. Running script: ${SCRIPT_NAME}...")    
        execute_process(
            COMMAND python3 "${CMAKE_SOURCE_DIR}/scripts/dependencies/${SCRIPT_NAME}" "${CMAKE_SOURCE_DIR}" "${VERSION}"
            RESULT_VARIABLE RESULT
        )
        if(NOT RESULT EQUAL 0)
            message(FATAL_ERROR "[ERROR] Failed to install ${NAME}")
        endif()
    else()
        message(STATUS "[FOUND] ${NAME} already installed at ${CHECK_PATH}")
    endif()
endfunction()