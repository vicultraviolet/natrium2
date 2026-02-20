IncludeDirectories = {}
LibraryDirectories = {}
Libraries = {}

VulkanSdk = os.getenv("VULKAN_SDK")
IncludeDirectories["vk"] = "%{VulkanSdk}/Include/"
LibraryDirectories["vk"] = "%{VulkanSdk}/Lib/"

group "Dependencies"
    include "dependencies/fmt-Premake.lua"
    include "dependencies/glfw-Premake.lua"
group ""

IncludeDirectories["nlohmann_json"] = "dependencies/nlohmann_json/include/"
IncludeDirectories["stduuid"] = "dependencies/stduuid/include/"
IncludeDirectories["glm"] = "dependencies/glm/"

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
        "%{IncludeDirectories.glfw}",
        "%{IncludeDirectories.nlohmann_json}",
        "%{IncludeDirectories.stduuid}",
        "%{IncludeDirectories.glm}",
        "dependencies/",
        "include/",
        "src/"
    }

    links {
        "%{Libraries.fmt}",
        "%{Libraries.glfw}",
    }

    filter "system:linux"
        links {
            "vulkan",
        }

        defines {
            "NA2_PLATFORM_LINUX",
            "NA2_USE_GLFW"
        }

    filter "system:windows"
        includedirs {
            "%{IncludeDirectories.vk}",
        }

        libdirs {
            "%{LibraryDirectories.vk}",
        }

        links {
            "vulkan-1",
            "winmm",            
            "avrt",
            "user32",
            "ole32"
        }

        defines {
            "NA2_PLATFORM_WINDOWS",
            "NA2_USE_GLFW"
        }

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
