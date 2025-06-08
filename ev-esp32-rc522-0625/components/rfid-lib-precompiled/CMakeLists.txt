idf_component_register(
        INCLUDE_DIRS "include"
)

#target_link_libraries(${component_lib} "${CMAKE_CURRENT_SOURCE_DIR}/lib/librfid.a")
set_property(TARGET ${COMPONENT_LIB} APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/lib/librfid.a"
)