include(FindPackageHandleStandardArgs)

set(sim_connect_default_path 
	to_cmake_path(
		"C:\\MSFS SDK\\SimConnect SDK"
	)
)

find_path(SIM_CONNECT_INCLUDE_DIR
	NAMES SimConnect.h
	PATHS ${sim_connect_default_path}
	PATH_SUFFIXES include)

find_library(SIM_CONNECT_LIBRARY 
	NAMES SimConnect.lib
	PATHS ${sim_connect_default_path}
	PATH_SUFFIXES lib
)

find_package_handle_standard_args(SimConnect DEFAULT_MSG 
	SIM_CONNECT_INCLUDE_DIR
	SIM_CONNECT_LIBRARY)
