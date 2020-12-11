# cmake-format: off
function(apx_module)

    apx_parse_function_args(
        NAME apx_module
        MULTI_VALUE
            SRCS
            INCLUDES
            DEPENDS
            COMPILE_FLAGS
            LINK_FLAGS
            GENSRC
        OPTIONS
            INTERFACE
        ARGN ${ARGN}
    )
# cmake-format: on

    # guess module name
    set(path ${CMAKE_CURRENT_SOURCE_DIR})
    if(${path} MATCHES "^${APX_SHARED_DIR}")
        file(RELATIVE_PATH path ${APX_SHARED_DIR} ${path})
        set(path "shared.${path}")
        get_filename_component(GROUP_DIR ${APX_SHARED_DIR} ABSOLUTE)
    elseif(${path} MATCHES "^${APX_MODULES_DIR}")
        file(RELATIVE_PATH path ${APX_MODULES_DIR} ${path})
        get_filename_component(GROUP_DIR ${APX_SHARED_DIR} ABSOLUTE)
    else()
        message(FATAL_ERROR "Can't guess module name: ${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
    string(REPLACE "/" "." MODULE ${path})

    set(MODULE
        ${MODULE}
        PARENT_SCOPE
    )
    set_property(GLOBAL APPEND PROPERTY APX_MODULES ${module})

    # glob SRCS when needed
    set(srcs)
    if(NOT SRCS)
        if(INTERFACE)
            set(SRCS "*.h*")
        else()
            set(SRCS "*.[ch]*")
        endif()
    endif()
    foreach(src ${SRCS})
        if(NOT EXISTS ${src})
            file(
                GLOB src_exp
                RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
                ${src}
            )
            if(src_exp)
                set(src ${src_exp})
            endif()
        endif()
        list(APPEND srcs ${src})
    endforeach()

    # add library
    if(INTERFACE)
        add_library(${MODULE} INTERFACE ${srcs})
    else()
        add_library(${MODULE} STATIC EXCLUDE_FROM_ALL ${srcs})
    endif()

    # includes
    if(NOT INCLUDES)
        set(INCLUDES "${GROUP_DIR}")
    endif()

    if(INTERFACE)
        target_include_directories(${MODULE} INTERFACE ${INCLUDES})
    else()
        target_include_directories(${MODULE} PUBLIC ${INCLUDES})
    endif()

    # compile flags
    if(COMPILE_FLAGS)
        target_compile_options(${MODULE}_original PRIVATE ${COMPILE_FLAGS})
    endif()
    if(LINK_FLAGS)
        set_target_properties(${MODULE} PROPERTIES LINK_FLAGS ${LINK_FLAGS})
    endif()

    # generated sources
    if(GENSRC)
        foreach(gensrc ${GENSRC})
            if(INTERFACE)
                add_dependencies(${MODULE} ${gensrc})
            else()
                get_target_property(gensrc_targets ${gensrc} INTERFACE_SOURCES)
                get_target_property(gensrc_dir ${gensrc} INCLUDE_DIRECTORIES)

                add_dependencies(${MODULE} ${gensrc})
                target_include_directories(${MODULE} PUBLIC ${gensrc_dir})
                target_sources(${MODULE} PRIVATE ${gensrc_targets})
            endif()
        endforeach()
    endif()

    # dependencies (other modules)
    if(DEPENDS)
        foreach(dep ${DEPENDS})

            apx_use_module(${dep})

            get_target_property(dep_type ${dep} TYPE)
            if(${dep_type} STREQUAL "STATIC_LIBRARY" OR ${dep_type} STREQUAL "INTERFACE_LIBRARY")
                if(INTERFACE)
                    target_link_libraries(${MODULE} INTERFACE ${dep})
                else()
                    target_link_libraries(${MODULE} PRIVATE ${dep})
                endif()
            else()
                add_dependencies(${MODULE} ${dep})
            endif()
        endforeach()
    endif()

endfunction()