--
-- _netbeans.lua
-- Define the netbeans action(s).
-- Copyright (c) 2013 Santo Pfingsten
--

	_NETBEANS = { }
	premake.netbeans = { }
	
--
-- Register the "netbeans" action
--

	newaction {
		trigger         = "netbeans",
		shortname       = "NetBeans",
		description     = "Generate NetBeans project files",
	
		valid_kinds     = { "ConsoleApp", "WindowedApp", "StaticLib", "SharedLib" },
		
		valid_languages = { "C", "C++" },
		
		valid_tools     = {
			cc     = { "gcc" },
		},
		
		onproject = function(prj)
			premake.generate(prj, prj.name .. "/Makefile", premake.netbeans_cpp_makefile)
			premake.generate(prj, prj.name .. "/nbproject/project.xml", premake.netbeans_project)
			premake.generate(prj, prj.name .. "/nbproject/configurations.xml", premake.netbeans_cpp)
		end,
		
		oncleanproject = function(prj)
			premake.clean.directory(prj, prj.name)
		end
	}
