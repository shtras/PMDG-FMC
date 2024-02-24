cmake_minimum_required (VERSION 3.24.2)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (FMC VERSION 0.1)

add_library(fmc_compiler_flags INTERFACE)
target_compile_features(fmc_compiler_flags INTERFACE cxx_std_20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/" ${CMAKE_MODULE_PATH})

find_package(spdlog CONFIG REQUIRED)
find_package(civetweb CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(SimConnect REQUIRED)

add_executable (FMC_server
				"FMC.cpp" 
				 "PMDG_NG3_SDK.h" "ScreenSource.h" "MSFSScreenSource.h" "MSFSScreenSource.cpp" "DummyScreenSource.h" "DummyScreenSource.cpp" "Finally.h")

target_link_libraries(FMC_server PUBLIC fmc_compiler_flags spdlog::spdlog civetweb::civetweb-cpp rapidjson nlohmann_json::nlohmann_json ${SIM_CONNECT_LIBRARY})
target_include_directories(FMC_server PRIVATE ${CMAKE_SOURCE_DIR} ${SIM_CONNECT_INCLUDE_DIR})
