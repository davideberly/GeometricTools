cmake_minimum_required(VERSION 3.30...4.4)

project(gtapplications
	VERSION ${GTE_VERSION_MAJOR}.${GTE_VERSION_MINOR}
	DESCRIPTION "GTE Applications library"
	LANGUAGES CXX
)

# Source file list for the core library
set(gtapplications_CPP_FILES
	Application.cpp
	CameraRig.cpp
	Command.cpp
	ConsoleApplication.cpp
	Environment.cpp
	GTApplications.cpp
	OnIdleTimer.cpp
	Timer.cpp
	TrackBall.cpp
	TrackCylinder.cpp
	TrackObject.cpp
	Window2.cpp
	Window3.cpp
	WindowApplication.cpp
)
# Source file list for windowing backend (MSW or GLX)
set(gtapplications_BACKEND_CPP_FILES "")
if(WIN32)
	list(APPEND gtapplications_BACKEND_CPP_FILES
		MSW/Console.cpp
		MSW/ConsoleSystem.cpp
		MSW/WICFileIO.cpp
		MSW/WICFileIONative.cpp
		MSW/Window.cpp
		MSW/WindowSystem.cpp
	)
else()
	list(APPEND gtapplications_BACKEND_CPP_FILES
		GLX/Console.cpp
		GLX/ConsoleSystem.cpp
		GLX/WICFileIO.cpp
		GLX/Window.cpp
		GLX/WindowSystem.cpp
	)
endif()

if(GTE_BUILD_SHARED_LIBS)
	add_library(${PROJECT_NAME} SHARED)
	set_target_properties(${PROJECT_NAME}
		PROPERTIES
			SOVERSION ${PROJECT_VERSION_MAJOR}
			VERSION ${PROJECT_VERSION}
	)
else()
	add_library(${PROJECT_NAME} STATIC)
	set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
endif()

target_sources(${PROJECT_NAME}
	PRIVATE
		${gtapplications_CPP_FILES}
		${gtapplications_BACKEND_CPP_FILES}
)

file(GLOB_RECURSE gtapplications_HEADER_FILES
	CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)
target_sources(${PROJECT_NAME}
	INTERFACE
	FILE_SET fs_gtapplications_headers
	TYPE
	HEADERS
	BASE_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/..
	FILES
	${gtapplications_HEADER_FILES}
)

add_library(GTE::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(${PROJECT_NAME} PUBLIC gte_usage_requirements)
target_link_libraries(${PROJECT_NAME} PRIVATE gte_warning_levels)

target_link_libraries(${PROJECT_NAME} PUBLIC GTE::gtmathematics GTE::gtgraphics)
target_link_libraries(${PROJECT_NAME}
	PRIVATE ${GTE_APPLICATIONS_BACKEND_LIBRARIES}
)