# Build ChucK's official embedding core only; the standalone host and audio
# device backends are not used by these DAW-hosted plug-ins.
set(core "${chuck_SOURCE_DIR}/src/core")
set(chuck_cpp
    chuck chuck_absyn chuck_parse chuck_errmsg chuck_frame chuck_symbol
    chuck_table chuck_vm chuck_instr chuck_scan chuck_type chuck_emit
    chuck_compile chuck_dl chuck_oo chuck_lang chuck_ugen chuck_otf
    chuck_stats chuck_shell chuck_io chuck_carrier chuck_globals
    midiio_rtmidi rtmidi hidio_sdl ugen_osc ugen_filter ugen_stk ugen_xxx
    ulib_ai ulib_doc ulib_machine ulib_math ulib_std ulib_opsc
    util_buffers util_console util_math util_platforms util_string
    util_thread util_opsc util_serial util_hid uana_xform uana_extract)
set(chuck_c chuck_yacc util_network util_raw util_xforms util_sndfile)
set(chuck_lo address blob bundle message method pattern_match send server
    server_thread timetag)
set(chuck_sources)
foreach(name IN LISTS chuck_cpp)
    list(APPEND chuck_sources "${core}/${name}.cpp")
endforeach()
foreach(name IN LISTS chuck_c)
    list(APPEND chuck_sources "${core}/${name}.c")
endforeach()
foreach(name IN LISTS chuck_lo)
    list(APPEND chuck_sources "${core}/lo/${name}.c")
endforeach()

add_library(chuck_core STATIC ${chuck_sources})
target_include_directories(chuck_core PUBLIC "${core}" "${core}/lo")
target_compile_definitions(chuck_core PRIVATE HAVE_CONFIG_H)
set_source_files_properties("${core}/chuck_yacc.c"
    PROPERTIES COMPILE_DEFINITIONS "__USE_CHUCK_YACC__")
find_package(Threads REQUIRED)
target_link_libraries(chuck_core PUBLIC Threads::Threads ${CMAKE_DL_LIBS})
if(WIN32)
    target_compile_definitions(chuck_core PUBLIC
        __WINDOWS_DS__ __WINDOWS_MODERN__ __PLATFORM_WIN32__ NOMINMAX)
    target_compile_definitions(chuck_core PRIVATE
        HAVE_WIN32_THREADS WIN32 _CRT_SECURE_NO_WARNINGS)
    target_link_libraries(chuck_core PUBLIC
        dsound dinput8 dxguid wsock32 ws2_32 winmm iphlpapi user32)
elseif(APPLE)
    target_compile_definitions(chuck_core PUBLIC __MACOSX_CORE__)
    target_link_libraries(chuck_core PUBLIC
        "-framework CoreMIDI" "-framework CoreFoundation")
else()
    target_link_libraries(chuck_core PUBLIC m)
endif()
if(MSVC)
    target_compile_options(chuck_core PRIVATE /bigobj /utf-8)
endif()
