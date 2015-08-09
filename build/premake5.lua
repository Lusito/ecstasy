require('netbeans')

-- Prevent from script error when no action is given
if not _ACTION then
	printf "Error: No action defined!"
	return
end

-- Check for supported action
isNetbeans = _ACTION == "netbeans"
isVisualStudio = _ACTION == "vs2013"
isGMake = _ACTION == "gmake"
if not isNetbeans and not isVisualStudio and not isGMake then
	printf("Error: %s is not supported yet", _ACTION)
	return
end

-- Check for supported OS
isWindows = os.is( "windows" )
isLinux = os.is( "linux" )
isMacOSX = os.is( "macosx" )
if not isWindows and not isLinux then
	printf("Error: %s is not supported yet", os.get())
	return
end

-- Some Paths
rootDir			= ".."
objectDir		= rootDir .. "/temp"
libDir			= rootDir .. "/lib/" .. _ACTION
sourceDir		= rootDir .. "/source"
includeDir		= rootDir .. "/include"
binariesDir		= rootDir .. "/binaries"
testsDir		= rootDir .. "/tests"
benchmarksDir	= rootDir .. "/benchmarks"
buildDir		= (_ACTION)

-- [start] Settings that are true for all projects
configurations { "Debug", "Release" }

if isLinux and isNetbeans then
    location ("netbeans-linux")
else
    location (buildDir)
end

flags {"C++11", "nbProjectFolder"}

filter { "system:windows" }
	defines { "WIN32" }

filter { "action:vs*" }
	defines {"_CRT_SECURE_NO_DEPRECATE" }

filter { "Debug" }
	defines { isVisualStudio and "_DEBUG" or "DEBUG"}
	flags { "Symbols" }

filter { "Release" }
	flags {"ReleaseRuntime"}
	defines { "NDEBUG" }
	--flags { "Optimize" }
	optimize "Speed"

-- [end] Settings that are true for all projects

-- The Game Solution
solution "ECS-tasy"
	-- ECS-tasy Project
	project "ecstasy"
		kind "StaticLib"
		language "C++"
		objdir( objectDir .. "/ecstasy" )
		includedirs {
			includeDir
		}
		files {
			includeDir .."/**.h",
			sourceDir .."/**.h",
			sourceDir .."/**.cpp"
		}
		targetdir( libDir )
		filter { "Release" }
			targetsuffix  "-s"
		filter { "Debug" }
			targetsuffix  "-s-d"

	-- Test Project
	project "tests"
		kind "ConsoleApp"
		language "C++"
		objdir( objectDir .. "/Test" )
		defines { "USING_ECSTASY", "USING_SIGNAL11"}
		includedirs {
			includeDir
		}
		libdirs {
			libDir
		}
		files {
			testsDir .."/**.h",
			testsDir .."/**.cpp"
		}
		targetdir( binariesDir )
		
		filter { "Debug" }
			links { "ecstasy-s-d" }
		filter { "Release" }
			links { "ecstasy-s" }

	-- Test Project
	project "benchmarks"
		kind "ConsoleApp"
		language "C++"
		objdir( objectDir .. "/benchmarks" )
		defines { "USING_ECSTASY", "USING_SIGNAL11"}
		includedirs {
			includeDir,
			benchmarksDir
		}
		libdirs {
			libDir
		}
		files {
			benchmarksDir .."/**.h",
			benchmarksDir .."/**.hpp",
			benchmarksDir .."/**.cpp",
			benchmarksDir .."/**.cc"
		}
		targetdir( binariesDir )
		
		filter { "Debug" }
			links { "ecstasy-s-d" }
		filter { "Release" }
			links { "ecstasy-s" }

			