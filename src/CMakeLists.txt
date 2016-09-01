project(Shinzui)

option(SHINZUI_USE_SYSTEM_BOOST "Use system installed Boost libraries" OFF)

if (SHINZUI_USE_SYSTEM_BOOST)
    find_package(Boost 1.62 REQUIRED)
    set(BOOST_DEPENDENCIES "")
else()
    set(BOOST_DEPENDENCIES
        # Lockfree dependencies
        lockfree core config assert static_assert type_traits mpl preprocessor
        array throw_exception functional align parameter atomic detail
        utility predef

        # Asio dependencies
        asio system date_time smart_ptr bind regex
    )
endif()

set(SOURCE_FOLDERS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster
    ${CMAKE_CURRENT_SOURCE_DIR}/defs
    ${CMAKE_CURRENT_SOURCE_DIR}/map
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

BuildNow(
    TARGET Shinzui
    BOOST_DEPENDENCIES ${BOOST_DEPENDENCIES}
    STATIC_LIB ON
    OUTPUT_NAME Shinzui
)
