cmake_minimum_required (VERSION 3.8)

project ("P2P_Downloader")

set(SOURCES
	src/main.cpp
	src/Server2.cpp
)

if (MSVC_VERSION GREATER_EQUAL "1900")
	include(CheckCXXCompilerFlag)
		CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
		if (_cpp_latest_flag_supported)
			add_compile_options("/std:c++latest")
		endif()
endif()

add_executable(${PROJECT_NAME} ${SOURCES})


target_include_directories(${PROJECT_NAME}
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
)

if(WIN32)
	target_link_libraries(${PROJECT_NAME} wsock32 ws2_32)
endif()

