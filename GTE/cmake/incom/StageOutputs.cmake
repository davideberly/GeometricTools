#=============================================================================
# StageOutputs.cmake
#
# 1. Performs include(GNUInstallDirs) on inclusion
#
# 2. Provides a function to configure staged build output directories
# for both single-config and multi-config generators.
#
# Usage:
#   include(StageOutputs)
#   stageOutputDirsFor(<NAME>)
#
# Example:
#   stageOutputDirsFor(OTFCCXX)
#
# This will define:
#   <NAME>_STAGEDIR
#
# And conditionally set:
#   CMAKE_RUNTIME_OUTPUT_DIRECTORY
#   CMAKE_LIBRARY_OUTPUT_DIRECTORY
#   CMAKE_ARCHIVE_OUTPUT_DIRECTORY
#   plus their per-config equivalents when applicable.
#=============================================================================

include(GNUInstallDirs)

# The NAME is only used to name the [NAME]_STAGEDIR variable.
# The NAME has no relation to any other name or variable in CMake or elsewhere and can be anything.
# A sensible idea is to have the NAME the same as the name of the project.
function(stageOutputDirsFor NAME SUBDIR)
	if(NOT NAME)
		message(FATAL_ERROR
			"stageOutputDirsFor(): NAME argument is required."
		)
	endif()
	if(NOT SUBDIR)
		message(FATAL_ERROR
			"stageOutputDirsFor(): SUBDIR argument is required."
		)
	endif()

	# Define the staging directory
	set(_stagedir "${CMAKE_CURRENT_BINARY_DIR}/${SUBDIR}")

	# Expose stagedir to the caller
	set(${NAME}_STAGEDIR "${_stagedir}" PARENT_SCOPE)

	# Helper macro to set output directories conditionally
	macro(_set_output_dir var subdir)
		if(NOT DEFINED ${var})
			set(
				${var}
				"${_stagedir}/${subdir}" PARENT_SCOPE
			)
		endif()
	endmacro()

	# Baseline (single-config and fallback for multi-config)
	_set_output_dir(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_INSTALL_BINDIR}")
	_set_output_dir(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_INSTALL_LIBDIR}")
	_set_output_dir(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_INSTALL_LIBDIR}")

	# Multi-config generators (Visual Studio, Xcode, Ninja Multi-Config)
	if(CMAKE_CONFIGURATION_TYPES)
		foreach(_cfg IN LISTS CMAKE_CONFIGURATION_TYPES)
			string(TOUPPER "${_cfg}" _cfg_uc)

			if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_cfg_uc})
				set(
					CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_cfg_uc}
					"${_stagedir}/${CMAKE_INSTALL_BINDIR}" PARENT_SCOPE
				)
			endif()

			if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_cfg_uc})
				set(
					CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_cfg_uc}
					"${_stagedir}/${CMAKE_INSTALL_LIBDIR}" PARENT_SCOPE
				)
			endif()

			if(NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_cfg_uc})
				set(
					CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_cfg_uc}
					"${_stagedir}/${CMAKE_INSTALL_LIBDIR}" PARENT_SCOPE
				)
			endif()
		endforeach()
	endif()
endfunction()
