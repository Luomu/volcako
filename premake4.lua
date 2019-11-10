solution "volcako"
    targetdir "bin"
    configurations { "Debug", "Release" }

project "volcako"
    kind "WindowedApp"
    language "C++"
    files { "source/**.h", "source/**.cpp", "source/**.c", "libs/**.cpp", "libs/**.c", "libs/**.h", "premake4.lua" }
    includedirs { "source/imgui", "libs", "libs/glfw/include", "libs/gl3w", "libs/imgui" }
    libdirs { "libs/glfw/lib-vc2010-32" }
    links { "glfw3", "opengl32", "gdi32", "shell32", "winmm" }
    --ignoredefaultlibraries { "msvcrt" }

    configuration "Debug"
        defines { "DEBUG" }
        flags { "Symbols" }
        debugdir "bin"

    configuration "Release"
        defines { "NDEBUG" }
        debugdir "bin"

--replace with your actual version, genie can't detect this
premake.action.current().vstudio.windowsTargetPlatformVersion = "10.0.17763.0"
