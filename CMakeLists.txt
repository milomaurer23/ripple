cmake_minimum_required(VERSION 3.22)

# Milo's "elemental" modulation-source plugin family. Plugin #1: Ripple (droplet/ripple
# amplitude shaper). Reuses the JUCE checkout that already lives in the sibling DAW
# project instead of vendoring a second ~1GB copy.
# FRAGILE: if DAW/ is ever moved or renamed, update JUCE_PATH below (or vendor/symlink
# a local JUCE checkout instead).
#
# Generate an Xcode project:   cmake -G Xcode -B build
#   (build/ is a symlink to ~/Library/Developer/ElementalLFO-build: this folder is inside
#    iCloud Documents, and the File Provider stamps files with Finder metadata that
#    codesign rejects.)
# Build from the CLI:          cmake --build build --config Release --target Ripple_VST3 Ripple_AU

set(JUCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../DAW/JUCE" CACHE PATH "Path to the JUCE checkout")

set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures")

project(ElementalLFO VERSION 0.1.0 LANGUAGES C CXX OBJC OBJCXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(${JUCE_PATH} JUCE)
add_subdirectory(plugin)
