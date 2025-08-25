workspace "Compily"
	platforms { "x86", "x64" }
	configurations { "Debug", "Release" }
	startproject "MyCompiler"
	buildoptions { "/utf-8" }

	configurations 
	{
		"Debug",
		"Release"
	}

	outputdir = "%{cfg.buildcfg}-%{cfg.architecture}";

	filter "system:windows"
    	systemversion "latest"
    	defines { "COMPILY_WINDOWS" }

	filter "system:linux"
   		defines { "COMPILY_LINUX" }

	filter "platforms:x86"
		architecture "x86"

	filter "platforms:x64"
		architecture "x64"

	filter "configurations:Debug"
		defines "COMPILY_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "COMPILY_RELEASE"
		runtime "Release"
		optimize "On"

	IncludeDirs = {}
	IncludeDirs["spdlog"] = "Utilities/vendor/spdlog/include"

group "Dependencies"
	project "Utilities"
		location "Utilities"
		kind "StaticLib"
		language "C++"
		cppdialect "C++17"

		targetdir  ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

		files
		{
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp",
		}

		includedirs
		{
			"%{prj.name}/src",
			"%{IncludeDirs.spdlog}"
		}

	project "Lexy"
		location "Lexy"
		kind "StaticLib"
		language "C++"
		cppdialect "C++17"

		targetdir  ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

		files
		{
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp",
		}

		includedirs
		{
			"%{wks.location}/Utilities/src",
			"%{prj.name}/src",
			"%{IncludeDirs.spdlog}"
		}

		links
		{
			"Utilities"
		}

	project "Parsy"
		location "Parsy"
		kind "StaticLib"
		language "C++"
		cppdialect "C++17"

		targetdir  ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

		files
		{
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp",
		}

		includedirs
		{
			"%{wks.location}/Utilities/src",
			"%{wks.location}/Lexy/src",
			"%{prj.name}/src",
			"%{IncludeDirs.spdlog}"
		}

		links
		{
			"Utilities",
			"Lexy"
		}

	project "Iry"
		location "Iry"
		kind "StaticLib"
		language "C++"
		cppdialect "C++17"

		targetdir  ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

		files
		{
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp",
		}

		includedirs
		{
			"%{wks.location}/Utilities/src",
			"%{wks.location}/Lexy/src",
			"%{wks.location}/Parsy/src",
			"%{prj.name}/src",
			"%{IncludeDirs.spdlog}"
		}

		links
		{
			"Utilities",
			"Lexy",
			"Parsy"
		}

group ""

project "MyCompiler"
	location "MyCompiler"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"

	targetdir  ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"%{wks.location}/Utilities/src",
		"%{wks.location}/Lexy/src",
		"%{wks.location}/Parsy/src",
		"%{wks.location}/Iry/src",
		"%{prj.name}/src",
		"%{IncludeDirs.spdlog}"
	}

	links
	{
		"Lexy",
		"Parsy",
		"Iry"
	}

project "TestCompiler"
	location "TestCompiler"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"

	targetdir  ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"%{wks.location}/Utilities/src",
		"%{wks.location}/Lexy/src",
		"%{wks.location}/Lexy/src",
		"%{wks.location}/Parsy/src",
		"%{wks.location}/Iry/src",
		"%{prj.name}/src",
		"%{IncludeDirs.spdlog}"
	}

	links
	{
		"Lexy",
		"Parsy",
		"Iry"
	}