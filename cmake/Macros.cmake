include(CMakeParseArguments)
include(ExternalProject)

function(RequireBoost)
    cmake_parse_arguments(
        ARG
        ""
        "TARGET;MODULE;WHERE"
        ""
        ${ARGN}
    )

    if (NOT ARG_MODULE)
        message(FATAL_ERROR "Boost module not specified")
    endif()

    ExternalProject_Add(Boost_${ARG_MODULE}
        GIT_REPOSITORY https://github.com/boostorg/${ARG_MODULE}
        GIT_TAG develop
        PREFIX ${CMAKE_BINARY_DIR}/third_party
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        TEST_COMMAND ""
    )

    add_dependencies(${ARG_TARGET} Boost_${ARG_MODULE})
    AddToSources(
        TARGET ${ARG_TARGET}
        INC_PATH ${CMAKE_BINARY_DIR}/third_party/src/Boost_${ARG_MODULE}/include
    )
endfunction()

function(AddDependency)
    cmake_parse_arguments(
        ARG
        ""
        "TARGET;DEPENDENCY"
        ""
        ${ARGN}
    )

    set(${ARG_TARGET}_DEPENDENCIES ${${ARG_TARGET}_DEPENDENCIES} ${ARG_DEPENDENCY} CACHE INTERNAL "")
endfunction()

function(AddToSources)
    cmake_parse_arguments(
        ARG
        ""
        "TARGET;SRC_PATH;INC_PATH;"
        "GLOB_SEARCH"
        ${ARGN}
    )

    if (ARG_SRC_PATH)
        # Add each file and extension
        foreach (ext ${ARG_GLOB_SEARCH})
            file(GLOB TMP_SOURCES ${ARG_SRC_PATH}/*${ext})

            if(ARG_INC_PATH)
                file(GLOB TMP_INCLUDES ${ARG_INC_PATH}/*${ext})
            else()
                set(TMP_INCLUDES "")
            endif()

            set(${ARG_TARGET}_SOURCES ${${ARG_TARGET}_SOURCES} ${TMP_SOURCES} ${TMP_INCLUDES} CACHE INTERNAL "")
        endforeach()
    endif()

    # Add include dirs
    if(NOT ARG_INC_PATH)
        set(ARG_INC_PATH ${ARG_SRC_PATH})
    endif()
    set(${ARG_TARGET}_INCLUDE_DIRECTORIES ${${ARG_TARGET}_INCLUDE_DIRECTORIES} ${ARG_INC_PATH} CACHE INTERNAL "")
endfunction()

function(BuildNow)
    cmake_parse_arguments(
        ARG
        "EXECUTABLE;STATIC_LIB;"
        "TARGET;BUILD_FUNC;OUTPUT_NAME;"
        "BOOST_DEPENDENCIES"
        ${ARGN}
    )

    if (ARG_EXECUTABLE)
        add_executable(${ARG_TARGET} ${${ARG_TARGET}_SOURCES})
    elseif (ARG_STATIC_LIB)
        add_library(${ARG_TARGET} STATIC ${${ARG_TARGET}_SOURCES})
    endif()

    foreach(dep ${ARG_BOOST_DEPENDENCIES})
        RequireBoost(
            TARGET ${ARG_TARGET}
            MODULE ${dep}
        )
    endforeach()

    foreach (dir ${${ARG_TARGET}_INCLUDE_DIRECTORIES})
        target_include_directories(${ARG_TARGET}
            PUBLIC ${dir}
        )
    endforeach()

    foreach (dep ${${ARG_TARGET}_DEPENDENCIES})
        target_link_libraries(${ARG_TARGET}
            PUBLIC ${dep}
        )
    endforeach()

    if (UNIX)
        if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
            set_target_properties(${ARG_TARGET} PROPERTIES
                COMPILE_DEFINITIONS DEBUG
            )
        else()
            set_target_properties(${ARG_TARGET} PROPERTIES
                COMPILE_DEFINITIONS NDEBUG
            )
        endif()
    endif()

    set_target_properties(${ARG_TARGET} PROPERTIES
        OUTPUT_NAME ${ARG_OUTPUT_NAME}
    )

    set(ALL_TARGETS ${ALL_TARGETS} ${ARG_TARGET} CACHE INTERNAL "")
endfunction()

function(ResetAllTargets)
    foreach(target ${ALL_TARGETS})
        set(${${ARG_TARGET}_DEPENDENCIES} "" CACHE INTERNAL "")
        set(${ARG_TARGET}_SOURCES "" CACHE INTERNAL "")
        set(${ARG_TARGET}_INCLUDE_DIRECTORIES "" CACHE INTERNAL "")
    endforeach()

    set(ALL_TARGETS "" CACHE INTERNAL "")
endfunction()
