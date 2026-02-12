IncludeDirectories = {}
LibraryDirectories = {}
Libraries = {}

VulkanSdk = os.getenv("VULKAN_SDK")
IncludeDirectories["vk"] = "%{VulkanSdk}/Include/"
LibraryDirectories["vk"] = "%{VulkanSdk}/Lib/"

include "dependencies/fmt-Premake.lua"

group "Dependencies"

group ""

project "Natrium2"
    location "./"
    targetname "%{prj.name}-bin"
    kind "StaticLib"
    staticruntime "Off"

    language "C++"
    cppdialect "C++20"
    systemversion "latest"

    pchheader "Pch.hpp"
    pchsource "src/Pch.cpp"

    files {
        "include/**.hpp",
        "src/**.hpp",
        "src/**.cpp"
    }

    includedirs {
        "%{IncludeDirectories.fmt}",
        "dependencies/",
        "include/",
        "src/Natrium/"
    }

    links {
        "fmt"
    }

    filter "system:linux"
        links {

        }

        defines { "NA2_PLATFORM_LINUX" }

    filter "system:windows"
        includedirs {

        }

        libdirs {

        }

        links {
            "winmm",
            "avrt",
            "user32",
            "ole32"
        }

        defines { "NA2_PLATFORM_WINDOWS" }

        buildoptions { "/utf-8" }

    filter "toolset:msc"
        defines { "_CRT_SECURE_NO_WARNINGS" }

    filter "toolset:clang"
        buildoptions { "-Wno-switch" }

    filter "configurations:dbg"
        symbols "On"
        defines { "NA2_CONFIG_DEBUG" }

    filter "configurations:rel"
        optimize "speed"
        symbols "Off"
        defines { "NA2_CONFIG_RELEASE" }

    filter "configurations:dist"
        optimize "speed"
        symbols "Off"
        defines { "NA2_CONFIG_DIST" }

    filter { "system:windows", "configurations:dbg" }
        links {

        }
    filter { "system:windows", "configurations:rel or dist" }
        links {

        }
