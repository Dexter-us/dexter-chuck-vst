cmake_minimum_required(VERSION 3.22)
project(DexterChuckVST VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
include(FetchContent)

# Pinned releases: keep these independent of the unlicensed Chuck Racks app source.
FetchContent_Declare(juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 3fb5cbc0340758596dac39f7c8de47d562b4896e)
FetchContent_Declare(chuck
    GIT_REPOSITORY https://github.com/ccrma/chuck.git
    GIT_TAG 9d36c8dd7f45a75f375adff781f4cf7deb0ad910)
FetchContent_MakeAvailable(juce chuck)

include(cmake/ChucKCore.cmake)

juce_add_binary_data(ChuckExamples SOURCES
    Examples/Instruments/Welcome.ck
    Examples/Effects/SoftLowPass.ck)

function(add_dexter_chuck_plugin target product plugin_code is_effect)
    if(is_effect)
        set(plugin_is_synth FALSE)
        set(plugin_category Fx)
        set(plugin_description "ChucK audio effect by Dexter U.S.")
    else()
        set(plugin_is_synth TRUE)
        set(plugin_category Instrument)
        set(plugin_description "ChucK instrument by Dexter U.S.")
    endif()

    juce_add_plugin(${target}
        COMPANY_NAME "Dexter U.S."
        BUNDLE_ID "us.dexter.chuck.${plugin_code}"
        PLUGIN_MANUFACTURER_CODE DxUS
        PLUGIN_CODE ${plugin_code}
        PRODUCT_NAME "${product}"
        DESCRIPTION "${plugin_description}"
        FORMATS VST3
        IS_SYNTH ${plugin_is_synth}
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS TRUE
        COPY_PLUGIN_AFTER_BUILD FALSE
        VST3_CATEGORIES ${plugin_category})

    target_sources(${target} PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/ChuckRuntime.cpp
        Source/ChuckTokeniser.cpp)
    target_include_directories(${target} PRIVATE Source)
    target_compile_definitions(${target} PRIVATE
        DEXTER_CHUCK_IS_EFFECT=${is_effect}
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_MODAL_LOOPS_PERMITTED=1
        JUCE_VST3_CAN_REPLACE_VST2=0)
    target_link_libraries(${target} PRIVATE
        chuck_core
        ChuckExamples
        juce::juce_audio_utils
        juce::juce_gui_extra
        juce::juce_recommended_config_flags)
    if(MSVC)
        target_compile_options(${target} PRIVATE /bigobj /utf-8 /permissive- /Zc:__cplusplus)
    endif()

    # Keep the requested script folders beside the plug-in in its VST3 bundle.
    add_custom_command(TARGET ${target}_VST3 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${target}_VST3>/../Resources/Instruments"
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${target}_VST3>/../Resources/Effects"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/Examples/Instruments"
            "$<TARGET_FILE_DIR:${target}_VST3>/../Resources/Instruments"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/Examples/Effects"
            "$<TARGET_FILE_DIR:${target}_VST3>/../Resources/Effects")
endfunction()

add_dexter_chuck_plugin(DexterChuckInstrument
    "Dexter U.S. Chuck Instrument" DChI FALSE)
add_dexter_chuck_plugin(DexterChuckEffect
    "Dexter U.S. Chuck Effect" DChE TRUE)

option(DEXTER_CHUCK_BUILD_TESTS "Build ChucK audio smoke tests" ON)
if(DEXTER_CHUCK_BUILD_TESTS)
    enable_testing()
    add_executable(DexterChuckRuntimeSmoke
        Source/ChuckRuntime.cpp
        tests/runtime_smoke.cpp)
    target_include_directories(DexterChuckRuntimeSmoke PRIVATE
        Source "${chuck_SOURCE_DIR}/src/core" "${chuck_SOURCE_DIR}/src/core/lo")
    target_link_libraries(DexterChuckRuntimeSmoke PRIVATE chuck_core)
    target_compile_definitions(DexterChuckRuntimeSmoke PRIVATE
        DEXTER_CHUCK_EXAMPLES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Examples")
    add_test(NAME dexter-chuck-audio COMMAND DexterChuckRuntimeSmoke)

    foreach(plugin IN ITEMS DexterChuckInstrument DexterChuckEffect)
        add_executable(${plugin}Smoke tests/processor_smoke.cpp)
        target_include_directories(${plugin}Smoke PRIVATE Source)
        target_link_libraries(${plugin}Smoke PRIVATE
            ${plugin}
            chuck_core
            ChuckExamples
            juce::juce_audio_utils
            juce::juce_gui_extra)
        if(MSVC)
            target_compile_options(${plugin}Smoke PRIVATE
                /bigobj /utf-8 /permissive- /Zc:__cplusplus)
            target_compile_definitions(${plugin}Smoke PRIVATE
                JUCE_WEB_BROWSER=0
                JUCE_USE_CURL=0
                JUCE_MODAL_LOOPS_PERMITTED=1
                JUCE_VST3_CAN_REPLACE_VST2=0)
        endif()
        add_test(NAME ${plugin}-processor COMMAND ${plugin}Smoke)
    endforeach()
endif()
