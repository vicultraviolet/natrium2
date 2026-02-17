IncludeDirectories["fmt"] = "dependencies/fmt/include/"
Libraries["fmt"] = "fmt"

project "fmt"
    location "./"
    targetname "%{prj.name}-bin"
    kind "StaticLib"
    staticruntime "Off"

    language "C++"
    cppdialect "C++20"
    systemversion "latest"

    files {
        "fmt/include/**.h",
        "fmt/src/format.cc"
    }

    includedirs {
        "fmt/include/"
    }

	filter "system:linux"
        pic "On"

    filter "system:windows"
        buildoptions {
            "/utf-8"
        }

    filter "configurations:dbg"
        symbols "On"

    filter "configurations:rel"
        optimize "speed"
        symbols "Off"

    filter "configurations:dist"
        optimize "speed"
        symbols "Off"