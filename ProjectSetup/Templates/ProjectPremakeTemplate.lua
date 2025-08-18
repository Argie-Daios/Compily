project "#projectName#"
	location "#projectName#"
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
		"%{prj.name}/src",
		"%{IncludeDirs.spdlog}"
	}

	links
	{
		"Lexy",
		"Parsy"
	}