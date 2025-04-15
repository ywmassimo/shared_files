function get_git_revision()
	local handle = io.popen("git describe --tags --always")
	local result = handle:read("*a")
	handle:close()
	return string.gsub(result, "%s+", "")
end

workspace "source"
	platforms {
		"x32"
	}

	warnings "Default"

	configurations {
		"Debug",
		"Release",
		"Distribute"
	}

	staticruntime "on"
	language "C++"
	cppdialect "C++20"
	largeaddressaware "on"
	location "../ccache"

	ROOT = _WORKING_DIR

	filter "platforms:*32"
		architecture "x86"
	filter "platforms:*64"
		architecture "x64"


	filter {}

	includedirs {
		"../thirdparty/include",
		"../thirdparty/include/granny",
		"../thirdparty/include/directx",
		"../thirdparty/include/others",
		"../thirdparty/include/python",
	}

	libdirs {
		"../thirdparty/library",
		"../thirdparty/library/%{cfg.system}/%{cfg.platform}",
		"../thirdparty/library/%{cfg.system}/%{cfg.platform}/%{cfg.runtime}"
	}

	local os_ver = os.getversion()
	local hostname = string.format("__HOSTNAME__=\" %d.%d.%d (%s)\"", os_ver.majorversion, os_ver.minorversion, os_ver.revision, os_ver.description)
	local target_os = string.format("__TARGET_OS__=\"%s\"", os.target())
	local working_dir = string.format("__WORKING_DIR__=\"%s\"", _WORKING_DIR)
	local premake_ver = string.format("__PREMAKE_VERSION__=\"%s\"", _PREMAKE_VERSION)
	local git_revision = string.format("__GIT_REVISION_HASH__=\"%s\"", get_git_revision())
	defines {
		"__BUILD_TYPE__=\"%{cfg.buildcfg}\"",
		"__TOOLSET__=\"%{cfg.toolset}\"",
		"__ARCHITECTURE__=\"%{cfg.architecture}\"",
		hostname,
		target_os,
		working_dir,
		premake_ver,
		git_revision,
		"FMT_HEADER_ONLY",
	}

	filter "configurations:Debug"
	   defines {
		   "_DEBUG"
		}
	   runtime "Debug"
	   symbols "full"

	filter "configurations:Release"
		defines {
			"NDEBUG"
		}
	   runtime "Release"
	   optimize "On"
	   symbols "On"

	filter "configurations:Distribute"
	   defines {
		   "NDEBUG",
		   "DISTRIBUTE",
	   }
	  runtime "Release"
	  optimize "Full"
	  symbols "Off"

	filter "action:vs*"
		systemversion "latest"
		atl ("Static")

		defines {
			"_CRT_SECURE_NO_WARNINGS",
			"_HAS_STD_BYTE=0",
		}

		disablewarnings {
			"4242",
			"4307",
			"4996",
			"4244",
			"4267",
		}

		characterset "MBCS"

		flags {
			"MultiProcessorCompile",
			"NoBufferSecurityCheck",
			"NoRuntimeChecks",
			"RelativeLinks",
		}

	filter { "platforms:*32", "action:vs*"}
		defines { "_USE_32BIT_TIME_T" }
	filter {}

	if os.istarget("windows") then
		group "client"
			project "CWebBrowser"
				kind "StaticLib"
				language "C"
				removeplatforms { "x64" }
				files {
					"../srcs/%{prj.group}/%{prj.name}/**.h",
					"../srcs/%{prj.group}/%{prj.name}/**.c"
				}
			project "EffectLib"
				kind "StaticLib"
				removeplatforms { "x64" }
				files {
					"../srcs/%{prj.group}/%{prj.name}/**.h",
					"../srcs/%{prj.group}/%{prj.name}/**.cpp"
				}
				pchheader "StdAfx.h"
				pchsource "../srcs/%{prj.group}/%{prj.name}/StdAfx.cpp"

			project "EterBase"
				kind "StaticLib"
				removeplatforms { "x64" }
				files {
					"../srcs/%{prj.group}/%{prj.name}/**.h",
					"../srcs/%{prj.group}/%{prj.name}/**.cpp"
				}
				pchheader "StdAfx.h"
				pchsource "../srcs/%{prj.group}/%{prj.name}/StdAfx.cpp"

			project "EterPack"
				kind "StaticLib"
				removeplatforms { "x64" }
				files {
					"../srcs/%{prj.group}/%{prj.name}/**.h",
					"../srcs/%{prj.group}/%{prj.name}/**.hpp",
					"../srcs/%{prj.group}/%{prj.name}/**.cpp",
					"../srcs/%{prj.group}/%{prj.name}/**.asm",
					"../srcs/%{prj.group}/%{prj.name}/**.inc"
				}
			project "EterGrnLib"
				kind "StaticLib"
				removeplatforms { "x64" }
				files {
					"../srcs/%{prj.group}/%{prj.name}/**.h",
					"../srcs/%{prj.group}/%{prj.name}/**.cpp"
				}
				pchheader "StdAfx.h"
				pchsource "../srcs/%{prj.group}/%{prj.name}/StdAfx.cpp"

			project "EterImageLib"
				kind "StaticLib"
				removeplatforms { "x64" }
				files {
					"../srcs/%{prj.group}/%{prj.name}/**.h",
					"../srcs/%{prj.group}/%{prj.name}/**.cpp"
				}
				pchheader "StdAfx.h"
				pchsource "../srcs/%{prj.group}/%{prj.name}/StdAfx.cpp"

			project "EterLib"
				kind "StaticLib"
				removeplatforms { "x64" }
				files {
					"../srcs/%{prj.group}/%{prj.name}/**.h",
					"../srcs/%{prj.group}/%{prj.name}/**.cpp"
				}
				pchheader "StdAfx.h"
				pchsource "../srcs/%{prj.group}/%{prj.name}/StdAfx.cpp"

			project "EterLocale"
				kind "StaticLib"
				removeplatforms { "x64" }
				files {
					"../srcs/%{prj.group}/%{prj.name}/**.h",
					"../srcs/%{prj.group}/%{prj.name}/**.cpp"
				}
				pchheader "StdAfx.h"
				pchsource "../srcs/%{prj.group}/%{prj.name}/StdAfx.cpp"

			project "EterPythonLib"
				kind "StaticLib"
				removeplatforms { "x64" }
				files {
					"../srcs/%{prj.group}/%{prj.name}/**.h",
					"../srcs/%{prj.group}/%{prj.name}/**.cpp"
				}
				pchheader "StdAfx.h"
				pchsource "../srcs/%{prj.group}/%{prj.name}/StdAfx.cpp"

			project "GameLib"
				kind "StaticLib"
				removeplatforms { "x64" }
				files {
					"../srcs/%{prj.group}/%{prj.name}/**.h",
					"../srcs/%{prj.group}/%{prj.name}/**.cpp"
				}
				pchheader "StdAfx.h"
				pchsource "../srcs/%{prj.group}/%{prj.name}/StdAfx.cpp"

			project "MilesLib"
				kind "StaticLib"
				removeplatforms { "x64" }
				files {
					"../srcs/%{prj.group}/%{prj.name}/**.h",
					"../srcs/%{prj.group}/%{prj.name}/**.cpp"
				}
				pchheader "Stdafx.h"
				pchsource "../srcs/%{prj.group}/%{prj.name}/Stdafx.cpp"

			project "PRTerrainLib"
				kind "StaticLib"
				removeplatforms { "x64" }
				files {
					"../srcs/%{prj.group}/%{prj.name}/**.h",
					"../srcs/%{prj.group}/%{prj.name}/**.cpp"
				}
				pchheader "StdAfx.h"
				pchsource "../srcs/%{prj.group}/%{prj.name}/StdAfx.cpp"

			project "ScriptLib"
				kind "StaticLib"
				removeplatforms { "x64" }
				files {
					"../srcs/%{prj.group}/%{prj.name}/**.h",
					"../srcs/%{prj.group}/%{prj.name}/**.cpp"
				}
				pchheader "StdAfx.h"
				pchsource "../srcs/%{prj.group}/%{prj.name}/StdAfx.cpp"

			project "SpeedTreeLib"
				kind "StaticLib"
				removeplatforms { "x64" }
				files {
					"../srcs/%{prj.group}/%{prj.name}/**.h",
					"../srcs/%{prj.group}/%{prj.name}/**.cpp"
				}
				pchheader "StdAfx.h"
				pchsource "../srcs/%{prj.group}/%{prj.name}/StdAfx.cpp"

			project "SphereLib"
				kind "StaticLib"
				removeplatforms { "x64" }
				files {
					"../srcs/%{prj.group}/%{prj.name}/**.h",
					"../srcs/%{prj.group}/%{prj.name}/**.cpp"
				}
				pchheader "StdAfx.h"
				pchsource "../srcs/%{prj.group}/%{prj.name}/StdAfx.cpp"

			project "UserInterface"
				kind "WindowedApp"
				targetdir (ROOT .. "/../client/")
				targetname "m2"
				targetsuffix "_%{cfg.architecture}_%{cfg.buildcfg}"
				removeplatforms { "x64" }

				files {
					"../srcs/%{prj.group}/%{prj.name}/**.h",
					"../srcs/%{prj.group}/%{prj.name}/**.cpp",
					"../srcs/%{prj.group}/%{prj.name}/**.rc",
					"../srcs/%{prj.group}/%{prj.name}/**.ico",
					"../srcs/%{prj.group}/%{prj.name}/**.c",
					"../srcs/%{prj.group}/%{prj.name}/**.cur"
				}

				filter { "files:**/minIni.c" }
					flags { "NoPCH" }
				filter {}

				pchheader "StdAfx.h"
				pchsource "../srcs/%{prj.group}/%{prj.name}/StdAfx.cpp"

				ignoredefaultlibraries { "libci" }

				links {
					"CWebBrowser",
					"EffectLib",
					"EterBase",
					"EterPack",
					"EterGrnLib",
					"EterImageLib",
					"EterLib",
					"EterLocale",
					"EterPythonLib",
					"GameLib",
					"MilesLib",
					"PRTerrainLib",
					"ScriptLib",
					"SpeedTreeLib",
					"SphereLib"
				}

				linkoptions {
					"/ignore:4098",
					"/ignore:4099",
				}

				configuration {
					"Release or Distribute",
					"vs*"
				}
					linkoptions {
						"/SAFESEH:NO"
					}

					flags {
						"LinkTimeOptimization",
						"NoIncrementalLink",
						"NoImplicitLink",
					}

				configuration {
					"Distribute",
					"vs*"
				}
					linkoptions {
						"/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\""
					}
	end

	group "tools"
		if os.istarget("windows") then
			project "archiver"
				kind "ConsoleApp"
				targetdir (ROOT .. "/bin")
				targetsuffix "_%{cfg.architecture}_%{cfg.buildcfg}"
				removeplatforms { "x64" }

				files {
					"../srcs/%{prj.group}/%{prj.name}/**.h",
					"../srcs/%{prj.group}/%{prj.name}/**.cpp"
				}

				defines {
					"TBB_SUPPRESS_DEPRECATED_MESSAGES"
				}

				links {
					"EterPack",
					"lzo2",
					"cryptlib"
				}

				linkoptions {
					"/ignore:4098",
					"/ignore:4099"
				}
				flags { "ExcludeFromBuild" }
			project "property_generate"
				kind "ConsoleApp"
				targetdir (ROOT .. "/bin")
				targetsuffix "_%{cfg.architecture}_%{cfg.buildcfg}"
				removeplatforms { "x64" }

				files {
					"../srcs/%{prj.group}/%{prj.name}/**.h",
					"../srcs/%{prj.group}/%{prj.name}/**.cpp"
				}

				links {
					"EterPack",
					"lzo2",
					"cryptlib"
				}

				linkoptions {
					"/ignore:4098",
					"/ignore:4099"
				}
				flags { "ExcludeFromBuild" }
			project "dmp_proto"
				kind "ConsoleApp"
				targetdir (ROOT .. "/bin")
				targetsuffix "_%{cfg.architecture}_%{cfg.buildcfg}"
				removeplatforms { "x64" }

				files {
					"../srcs/%{prj.group}/%{prj.name}/**.h",
					"../srcs/%{prj.group}/%{prj.name}/**.cpp"
				}

				links {
					"EterBase",
					"lzo2",
				}

				linkoptions {
					"/ignore:4098",
					"/ignore:4099"
				}

				flags { "ExcludeFromBuild" }

			project "wikipedia_generate"
				kind "ConsoleApp"
				targetdir (ROOT .. "/bin")
				targetsuffix "_%{cfg.architecture}_%{cfg.buildcfg}"
				removeplatforms { "x64" }

				files {
					"../srcs/%{prj.group}/%{prj.name}/**.h",
					"../srcs/%{prj.group}/%{prj.name}/**.cpp"
				}

				linkoptions {
					"/ignore:4098",
					"/ignore:4099"
				}

				flags { "ExcludeFromBuild" }
		end