OutputDirectory = "%{cfg.buildcfg}-%{cfg.system}"

workspace "Natrium"
    architecture "x64"
    configurations { "dbg", "rel", "dist" }

    targetdir "bin/%{OutputDirectory}/%{prj.name}/"
    objdir "bin-int/%{OutputDirectory}/%{prj.name}/"

    debugdir "%{wks.location}"

include "natrium-cpp/Natrium2-Premake.lua"
