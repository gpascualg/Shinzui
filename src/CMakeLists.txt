if (SHINZUI_USE_SYSTEM_BOOST)
    find_package(Boost 1.62 REQUIRED)
    set(BOOST_DEPENDENCIES "")

    AddDependency(
        TARGET Shinzui
        DEPENDENCY Boost
    )
else()
    set(BOOST_DEPENDENCIES
        # Lockfree dependencies
        lockfree core config assert static_assert type_traits mpl preprocessor
        array throw_exception functional align parameter atomic detail
        utility predef

        # Asio dependencies
        asio system date_time smart_ptr bind regex

        # Pool dependencies
        pool integer
    )

    foreach (dep ${BOOST_DEPENDENCIES})
        RequireExternal(
            TARGET Shinzui
            EXCLUDE ON
            MODULE "boostorg/${dep}:develop"
            INC_PATH "include"
        )
    endforeach()
endif()

set(DEFINES
    BUILD_TESTS=${SHINZUI_BUILD_TESTS}
    SIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P}
)

set(SOURCE_FOLDERS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/defs
    ${CMAKE_CURRENT_SOURCE_DIR}/map
    ${CMAKE_CURRENT_SOURCE_DIR}/map-cluster
    ${CMAKE_CURRENT_SOURCE_DIR}/server
    ${CMAKE_CURRENT_SOURCE_DIR}/threadpool
)

foreach(folder ${SOURCE_FOLDERS})
    AddToSources(
        TARGET Shinzui
        SRC_PATH ${folder}
        GLOB_SEARCH ".hpp;.cpp"
    )
endforeach()

if (UNIX AND SHINZUI_COVERAGE)
    AddDependency(
        TARGET Shinzui
        DEPENDENCY gcov
    )
endif()

BuildNow(
    TARGET Shinzui
    DEFINES ${DEFINES}
    STATIC_LIB ON
    OUTPUT_NAME Shinzui
)
