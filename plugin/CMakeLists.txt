# ---- Rename here only ----
set(PLUGIN_TARGET_NAME       "Ripple")
set(PLUGIN_PRODUCT_NAME      "Ripple")
set(PLUGIN_COMPANY_NAME      "Milo Maurer")
set(PLUGIN_BUNDLE_ID         "com.milomaurer.ripple")   # placeholder scoped to Milo; revisit once
                                                          # he has an Apple dev account / picks a
                                                          # permanent reverse-DNS root
set(PLUGIN_MANUFACTURER_CODE "Milo")                     # 4 chars, >=1 uppercase
set(PLUGIN_CODE              "Drop")                     # 4 chars, exactly 1 uppercase
# ---------------------------

juce_add_plugin(${PLUGIN_TARGET_NAME}
    PRODUCT_NAME                 "${PLUGIN_PRODUCT_NAME}"
    COMPANY_NAME                 "${PLUGIN_COMPANY_NAME}"
    BUNDLE_ID                    "${PLUGIN_BUNDLE_ID}"
    PLUGIN_MANUFACTURER_CODE     "${PLUGIN_MANUFACTURER_CODE}"
    PLUGIN_CODE                  "${PLUGIN_CODE}"
    FORMATS                      VST3 AU
    IS_SYNTH                     FALSE
    NEEDS_MIDI_INPUT             FALSE
    NEEDS_MIDI_OUTPUT            FALSE
    IS_MIDI_EFFECT                FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS  FALSE
    COPY_PLUGIN_AFTER_BUILD      TRUE      # installs to ~/Library/Audio/Plug-Ins/{VST3,Components}
    VST3_CATEGORIES              Fx Modulation
    AU_MAIN_TYPE                 kAudioUnitType_Effect
    NEEDS_WEB_BROWSER            FALSE
    NEEDS_CURL                   FALSE)

juce_generate_juce_header(${PLUGIN_TARGET_NAME})

target_sources(${PLUGIN_TARGET_NAME} PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/RippleGenerator.cpp
    Source/RippleView.cpp)

target_compile_definitions(${PLUGIN_TARGET_NAME} PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(${PLUGIN_TARGET_NAME}
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

# ---- pluginval ----
find_program(PLUGINVAL_EXECUTABLE pluginval
    PATHS /opt/homebrew/bin /usr/local/bin
          /Applications/pluginval.app/Contents/MacOS
          $ENV{HOME}/Applications/pluginval.app/Contents/MacOS)

if (PLUGINVAL_EXECUTABLE)
    add_custom_target(validate
        COMMAND ${PLUGINVAL_EXECUTABLE} --strictness-level 5
                --validate "$<TARGET_BUNDLE_DIR:${PLUGIN_TARGET_NAME}_VST3>"
        COMMAND ${PLUGINVAL_EXECUTABLE} --strictness-level 5
                --validate "$<TARGET_BUNDLE_DIR:${PLUGIN_TARGET_NAME}_AU>"
        DEPENDS ${PLUGIN_TARGET_NAME}_VST3 ${PLUGIN_TARGET_NAME}_AU
        COMMENT "pluginval: validating ${PLUGIN_TARGET_NAME} VST3 + AU"
        VERBATIM)
else()
    message(STATUS "pluginval not found — 'validate' target skipped. Install it (see scripts/pluginval.sh) to enable.")
endif()
