workspace "Compily"
	platforms { "x86", "x64" }
	configurations { "Debug", "Release" }

	configurations 
	{
		"Debug",
		"Release"
	}

	outputdir = "%{cfg.buildcfg}-%{cfg.architecture}";

	IncludeDir = {}

project "Lexy"
	location "Lexy"
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
		"%{prj.name}/src",
	}

	filter "system:windows"
    	systemversion "latest"
    	defines { "LEXY_WINDOWS" }

	filter "system:linux"
   		defines { "LEXY_LINUX" }

	filter "platforms:x86"
		architecture "x86"

	filter "platforms:x64"
		architecture "x64"

	filter "configurations:Debug"
		defines "LEXY_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "LEXY_RELEASE"
		runtime "Release"
		optimize "On"