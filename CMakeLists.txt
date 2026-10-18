cmake_minimum_required(VERSION 3.16)
project(nfsu2re CXX)

# Native 32-bit analysis/demo build. Recovered sources keep the original PC x86 layouts,
# so this target must be built as i386 code without position independence.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE OFF)
set(UG2_ARCH_FLAGS -m32 -fno-pie -mfpmath=387 -fno-strict-aliasing -fno-stack-protector)

file(GLOB UG2_RECOVERED_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/Speed/Indep/Src/Physics/*.cpp
    ${CMAKE_SOURCE_DIR}/src/Speed/Indep/Src/Physics/Collision/*.cpp
    ${CMAKE_SOURCE_DIR}/src/Speed/Indep/Src/World/*.cpp
    ${CMAKE_SOURCE_DIR}/src/Speed/Indep/Src/Camera/*.cpp
    ${CMAKE_SOURCE_DIR}/src/Speed/Indep/Src/Hud/*.cpp
    ${CMAKE_SOURCE_DIR}/src/Speed/Indep/Src/Frontend/*.cpp
    ${CMAKE_SOURCE_DIR}/src/Speed/Indep/Libs/bWare/Strings.cpp
    ${CMAKE_SOURCE_DIR}/src/Speed/Indep/Libs/bWare/Formatting.cpp
    ${CMAKE_SOURCE_DIR}/src/Speed/Indep/Libs/bWare/FormattingSink.cpp
    ${CMAKE_SOURCE_DIR}/src/Speed/Indep/Libs/bWare/FormattingCore.cpp
    ${CMAKE_SOURCE_DIR}/src/Speed/Indep/Libs/bWare/TimedAverage.cpp
    ${CMAKE_SOURCE_DIR}/src/Speed/Indep/Libs/bWare/bSlotPool.cpp
    ${CMAKE_SOURCE_DIR}/src/Speed/Indep/Libs/bWare/Compression.cpp)
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/src/Platform/Native/include)
add_library(ug2recovered STATIC ${UG2_RECOVERED_SOURCES})
target_compile_options(ug2recovered PRIVATE ${UG2_ARCH_FLAGS} -O1 -g -w
    -include ${CMAKE_SOURCE_DIR}/src/Platform/Native/NativeCompat.h)

add_library(ug2native STATIC
    src/Platform/Native/OriginalImage.cpp
    src/Platform/Native/NativeVehicle.cpp
    src/Platform/Native/NativeWorld.cpp
    src/Platform/Native/NativeCollision.cpp)
target_compile_options(ug2native PRIVATE ${UG2_ARCH_FLAGS} -O2 -Wall
    -include ${CMAKE_SOURCE_DIR}/src/Platform/Native/NativeCompat.h)

include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/src/Platform/Native/include)
set(UG2_LINK_FLAGS -m32 -no-pie ${CMAKE_SOURCE_DIR}/src/Platform/Native/OriginalSymbols.ld)

add_executable(ug2sim src/Platform/Native/SimMain.cpp)
target_compile_options(ug2sim PRIVATE ${UG2_ARCH_FLAGS} -O2 -Wall
    -include ${CMAKE_SOURCE_DIR}/src/Platform/Native/NativeCompat.h)
target_link_libraries(ug2sim -Wl,--start-group ug2native ug2recovered -Wl,--end-group m)
target_link_options(ug2sim PRIVATE ${UG2_LINK_FLAGS})

add_executable(ug2demo src/Platform/Native/DemoMain.cpp src/Platform/Native/NativeAssets.cpp
    src/Platform/Native/NativeScene.cpp src/Platform/Native/NativeFng.cpp src/Platform/Native/NativeHud.cpp
    src/Platform/Native/NativeFormatting.cpp
    src/Platform/Native/NativePause.cpp src/Platform/Native/NativePauseOptions.cpp src/Platform/Native/NativeControls.cpp)
target_compile_options(ug2demo PRIVATE ${UG2_ARCH_FLAGS} -O2 -Wall -D_REENTRANT
    -include ${CMAKE_SOURCE_DIR}/src/Platform/Native/NativeCompat.h)
target_include_directories(ug2demo BEFORE PRIVATE ${CMAKE_SOURCE_DIR}/src/Platform/Native/include /usr/include/SDL2)
target_link_directories(ug2demo PRIVATE /usr/lib/i386-linux-gnu)
target_link_libraries(ug2demo -Wl,--start-group ug2native ug2recovered -Wl,--end-group m -l:libSDL2-2.0.so.0 -l:libGL.so.1)
target_link_options(ug2demo PRIVATE ${UG2_LINK_FLAGS})

add_executable(ug2probe src/Platform/Native/CollisionProbe.cpp)
target_compile_options(ug2probe PRIVATE ${UG2_ARCH_FLAGS} -O1 -g -w
    -include ${CMAKE_SOURCE_DIR}/src/Platform/Native/NativeCompat.h)
target_link_libraries(ug2probe -Wl,--start-group ug2native ug2recovered -Wl,--end-group m)
target_link_options(ug2probe PRIVATE ${UG2_LINK_FLAGS})

include(CTest)
option(UG2_ENABLE_MODDING "Build the native mod loader and SDK examples" OFF)
if(UG2_ENABLE_MODDING)
    add_subdirectory(modding)
endif()

add_executable(ug2recordprobe src/Platform/Native/PhysicsRecordProbe.cpp src/Platform/Native/OriginalImage.cpp)
target_compile_options(ug2recordprobe PRIVATE ${UG2_ARCH_FLAGS} -O2 -Wall
    -include ${CMAKE_SOURCE_DIR}/src/Platform/Native/NativeCompat.h)
target_link_libraries(ug2recordprobe ug2recovered m)
target_link_options(ug2recordprobe PRIVATE ${UG2_LINK_FLAGS})

# Asset-free, display-free queue integration probe using the native thiscall bridge.
add_executable(ug2fengqueue_native_test src/Platform/Native/FEngQueueNativeTest.cpp)
target_compile_options(ug2fengqueue_native_test PRIVATE ${UG2_ARCH_FLAGS} -O1 -g
    -include ${CMAKE_SOURCE_DIR}/src/Platform/Native/NativeCompat.h)
target_link_libraries(ug2fengqueue_native_test ug2recovered)
target_link_options(ug2fengqueue_native_test PRIVATE -m32 -no-pie)

if(BUILD_TESTING)
    add_test(NAME feng_queue_native COMMAND ug2fengqueue_native_test)
endif()

# Synthetic FNG parser/list integration; unused rendering sections are discarded.
add_executable(ug2fenglist_native_test src/Platform/Native/FEngListNativeTest.cpp src/Platform/Native/NativeFng.cpp)
target_compile_options(ug2fenglist_native_test PRIVATE ${UG2_ARCH_FLAGS} -O1 -g -ffunction-sections -fdata-sections
    -include ${CMAKE_SOURCE_DIR}/src/Platform/Native/NativeCompat.h)
target_link_libraries(ug2fenglist_native_test ug2recovered m)
target_link_options(ug2fenglist_native_test PRIVATE ${UG2_LINK_FLAGS} -Wl,--gc-sections)
if(BUILD_TESTING)
    add_test(NAME feng_list_native COMMAND ug2fenglist_native_test)
endif()

# Asset-free recovered grid/callback integration. Fixed-address strings are synthetic.
add_executable(ug2fenggrid_native_test src/Platform/Native/FEngGridNativeTest.cpp)
target_compile_options(ug2fenggrid_native_test PRIVATE ${UG2_ARCH_FLAGS} -O1 -g
    -include ${CMAKE_SOURCE_DIR}/src/Platform/Native/NativeCompat.h)
target_link_libraries(ug2fenggrid_native_test ug2recovered)
target_link_options(ug2fenggrid_native_test PRIVATE -m32 -no-pie)
if(BUILD_TESTING)
    add_test(NAME feng_grid_native COMMAND ug2fenggrid_native_test)
endif()
