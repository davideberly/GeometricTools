if(COMMAND cmake_policy)
	# Allow VERSION in the project() statement.
	cmake_policy(SET CMP0048 NEW)
endif()

cmake_minimum_required(VERSION 3.14...4.4)

project(RoughPlaneSolidBox)

file(GLOB ${PROJECT_NAME}_CPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(${PROJECT_NAME}
	${${PROJECT_NAME}_CPP_FILES}
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
		GTE::gtapplications
		GTE::gtmathematicsgpu
		GTE::gtgraphics
)
