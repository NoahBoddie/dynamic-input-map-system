cmake_minimum_required(VERSION 3.21)
message("Using toolchain file ${CMAKE_TOOLCHAIN_FILE}.")

########################################################################################################################
## Define project
########################################################################################################################
project(
        DynamicInputMapSystem
        VERSION 1.0.0
        DESCRIPTION "An SKSE plugin to create custom control maps and unique input events."
        LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

include(GNUInstallDirs)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/version.rc
        @ONLY)

set(headers
     "include/ConditionData.h" "include/API_DIMS.h" "include/Var.h" "src/ControlInterface.h")

set(sources
        src/main.cpp

        src/Enums.h
        src/Parameter.h
        src/Argument.h
        src/Input.h
        src/TriggerInfo.h


        src/TestField.h
        src/TestField.cpp

        ${CMAKE_CURRENT_BINARY_DIR}/version.rc "src/Triggers/ITrigger.h" "src/Triggers/OnButton.h" "src/Triggers/OnControl.h" "src/Triggers/OnControl.cpp" "src/Triggers/OnButton.cpp" "src/Triggers/ITrigger.cpp" "src/TriggerNode.h" "src/InputCommand.h" "src/ActiveCommand.h" "src/EventData.h" "src/InputInterface.h" "src/Actions/IAction.h" "src/ActionNode.h" "src/ActionInfo.h" "src/Actions/InvokeFunction.h" "src/Actions/InvokeFunction.cpp" "src/Actions/IAction.cpp" "src/CommandEntry.h" "src/EventResult.h" "src/Settings.h" "src/InputNumber.h" "src/Utility.h" "src/ActiveData.h" "src/InputQueue.h" "src/Actions/InvokeInput.h" "src/Impl/IComponent.h" "src/Actions/InvokeInput.cpp" "src/Resources.h" "src/RE/Functions.h" "src/Actions/InvokeMode.h" "src/Actions/InvokeMode.cpp" "src/Impl/TypeAliases.h" "src/InputMatrix.h" "src/InputCommand.cpp" "src/Hooks/UserEvents.h" "src/Hooks.h" "src/HashLibrary.h" "src/Triggers/OnMouseMove.h" "src/VirtualEvent.h" "src/Triggers/OnAxis.h" "src/Triggers/OnAxis.cpp" "src/Triggers/OnThumbstick.h"  "src/Triggers/OnThumbstick.cpp" "src/Triggers/IHoldTrigger.h" "src/Triggers/IHoldTrigger.cpp" "src/Condition/ICondition.h" "src/Condition/ExternalCondition.h" "src/API_DIMS.cpp" "src/Condition/ConditionLibrary.h" "src/CommandEntry.cpp")

source_group(
        TREE ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
        ${headers}
        ${sources})

#########################################################################################################################
### Build options
#########################################################################################################################
message("Options:")
option(BUILD_TESTS "Build unit tests." OFF)
message("\tTests: ${BUILD_TESTS}")

########################################################################################################################
## Configure target DLL
########################################################################################################################
find_package(CommonLibSSE CONFIG REQUIRED)



add_library(
		${PROJECT_NAME}
		SHARED
		${headers}
		${sources}
	)

#source_group(
#	TREE
#		${CMAKE_CURRENT_SOURCE_DIR}
#	FILES
#		${headers}
#		${sources}
#)

target_commonlibsse_properties(${PROJECT_NAME} ${ARGN})
#add_commonlibsse_plugin(${PROJECT_NAME} SOURCES ${headers} ${sources})


add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS "${PROJECT_NAME}")
#message("FFFFFFFFFFFFFFFFFFFFFFF")


target_include_directories(${PROJECT_NAME}
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
        $<INSTALL_INTERFACE:src>
        ${ARTICUNO_INCLUDE_DIRS})

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_precompile_headers(${PROJECT_NAME}
        PRIVATE
        src/PCH.h)

install(TARGETS ${PROJECT_NAME}
        DESTINATION "${CMAKE_INSTALL_LIBDIR}")



if (NOT LexiconSKSEPath)
	cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH CMAKE_REPO_DIR)
	set(LexiconSKSEPath "${CMAKE_REPO_DIR}/lexicon-skse")
endif()

add_subdirectory(${LexiconSKSEPath} LexiconSKSE)
add_dependencies(${PROJECT_NAME} LexiconSKSE::LexiconSKSE)


target_include_directories(
	${PROJECT_NAME}
	PUBLIC
		${RoguePath}/include
)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
        RoguesGallery::RoguesGallery
        Lexicon::Lexicon
        LexiconSKSE::LexiconSKSE
)


########################################################################################################################
## Automatic plugin deployment
########################################################################################################################
# Automatic deployment to FOMOD directory.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_NAME "Debug")
else()
    set(BUILD_NAME "Release")
endif()
install(DIRECTORY DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Papyrus${BUILD_NAME}/")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Plugin${BUILD_NAME}/")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Plugin${BUILD_NAME}/")
if(${CMAKE_BUILD_TYPE} STREQUAL Debug OR ${CMAKE_BUILD_TYPE} STREQUAL RelWithDebInfo)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:${PROJECT_NAME}> "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Plugin${BUILD_NAME}/")
endif()
file(GLOB_RECURSE OUTPUT_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/**/*.dll")
file(GLOB_RECURSE OUTPUT_PDBS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/**/*.pdb")
file(GLOB_RECURSE OUTPUT_SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/**/*.pex")
set_property(TARGET ${PROJECT_NAME}
        APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_DLLS}")
set_property(TARGET ${PROJECT_NAME}
        APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_PDBS}")
set_property(TARGET ${PROJECT_NAME}
        APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_SCRIPTS}")

# Automatic deployment to Mod Organizer 2 mod directory.
foreach(DEPLOY_TARGET $ENV{CommonLibSSESamplePluginTargets})
    message("Adding deployment target ${DEPLOY_TARGET}.")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> "${DEPLOY_TARGET}/SKSE/Plugins/")
    if(${CMAKE_BUILD_TYPE} STREQUAL Debug OR ${CMAKE_BUILD_TYPE} STREQUAL RelWithDebInfo)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:${PROJECT_NAME}> "${DEPLOY_TARGET}/SKSE/Plugins/")
    endif()
endforeach()
