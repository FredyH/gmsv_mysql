solution "gmsv_mysql"
	language "C++"
	location "project"
	targetdir "bin"

	flags "StaticRuntime"

	includedirs {
		"include",
	}

	libdirs {
		"lib"
	}

	configurations {
		"Debug",
		"Release"
	}

	configuration "Release"
		optimize "On"

	configuration "Debug"
		flags "symbols"

	project "gmsv_mysql"
		kind "SharedLib"
		defines "GMMODULE"

		links {
			"libmysql"
		}

		files {
			"src/**.cpp",
			"src/**.h"
		}