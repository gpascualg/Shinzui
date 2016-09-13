set(LINT_DIR ${CMAKE_CURRENT_LIST_DIR})


function(setup_target_for_lint _targetname _sources)

    find_program(PYTHON_PATH python)

    if (NOT PYTHON_PATH)
        message(FATAL_ERROR "Could not find Python")
    endif()

    string (REPLACE ";" ":" SOURCES_STR "${_sources}")

	# Setup target
	add_custom_target(${_targetname}
        ${PYTHON_PATH} cpplint_wrapper.py "${SOURCES_STR}"

		WORKING_DIRECTORY ${LINT_DIR}
		COMMENT "Checking lint for project."
	)
endfunction()
