function wx_config()
    if os.get() == "windows" then
        defines { "_CRTDBG_MAPALLOC", "_CRT_SECURE_NO_WARNINGS" }

        wx = os.getenv("WXWIN")

        flags { "Unicode" }
        defines { "__WXMSW__", "__WXDEBUG__", "WXUSINGDLL" }

        includedirs { wx .. "/include", wx .. "/lib/vc_dll/mswud" }
        libdirs { wx .. "/lib/vc_dll" }

        links { "wxbase30ud", "wxmsw30ud_core", "wxmsw30ud_adv",
                "wxbase30ud_net", "wxbase30ud_xml", "wxmsw30ud_richtext",
                "wxpngd", "wxjpegd", }
    else
        configuration { "Debug" }
            buildoptions { "`wx-config --debug=yes --cflags`" }
            linkoptions { "`wx-config --debug=yes --libs std`" }

        configuration { "Release" }
            buildoptions { "`wx-config --debug=no --cflags`" }
            linkoptions { "`wx-config --libs std`" }
    end
end


local ROOT = ".."

workspace "M4Player"
    configurations { "Debug", "Release" }

    filter "configurations:Debug"
        defines { "_DEBUG", "DEBUG" }
        flags { "Symbols" }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

    project "VdkControls"
        language "C++"
        targetdir "bin/%{cfg.buildcfg}"

        local home = ROOT .. "/Libs/VdkControls"

        includedirs { home .. "/Inc" }
        files { home .. "/Inc/**.h", home .. "/Inc/**.inl", 
                home .. "/Src/**.cpp" }

        includedirs { home .. "/Src" }
        pchheader ( "StdAfx.h" )
        pchsource ( home .. "/Src/StdAfx.cpp" )

        wx_config()

        filter "system:Windows"
            kind "StaticLib"

        filter "system:Linux"
            kind "StaticLib"
            targetname "vdk"

            removefiles { home .. "/Src/MSW/*"  }

    project "MyTagLib"
        kind "StaticLib"
        language "C++"
        targetdir "bin/%{cfg.buildcfg}"

        local home = ROOT .. "/Libs/MyTagLib"

        includedirs { home }
        files { home .. "/*.h", home .. "/Src/*.h", home .. "/Src/*.cpp" }

        includedirs { home .. "/Src" }
        pchheader ( "StdAfx.h" )
        pchsource ( home .. "/Src/StdAfx.cpp" )

        flags { "Unicode" }

        filter "system:Windows"
            removefiles { home .. "/CharsetIconvImpl.h", 
                          home .. "/Src/CharsetIconvImpl.cpp" }

        filter "system:Linux"
            targetname "mytag"
            removefiles { home .. "/CharsetMswImpl.h", 
                          home .. "/Src/CharsetMswImpl.cpp" }

    project "Audio"
        language "C++"
        targetdir "bin/%{cfg.buildcfg}"

        local home = ROOT .. "/Libs/Audio"

        includedirs { home }
        files { home .. "/*.hpp", home .. "/Src/*.h", home .. "/Src/*.cpp" }

        includedirs { home .. "/Src" }
        pchheader ( "StdAfx.h" )
        pchsource ( home .. "/Src/StdAfx.cpp" )

        defines { "__AUDIO_HOST__" }

        wx_config()

        filter "system:Windows"
            kind "StaticLib"
            defines { "WIN32_LEAN_AND_MEAN" }

        filter "system:Linux"
            kind "StaticLib"
            targetname "audio"

    project "AudioSDK"
        language "C++"
        targetdir "bin/%{cfg.buildcfg}"

        local home = ROOT .. "/Libs/Audio"

        includedirs { home }

        files { home .. "/Config.hpp" }
        files { home .. "/Export.hpp" }
        files { home .. "/Service.hpp" }
        files { home .. "/Client.hpp" }
        files { home .. "/Codec.hpp" }
        files { home .. "/Outdev.hpp" }
        files { home .. "/Time.hpp" }
        files { home .. "/Debug.hpp" }

        files { home .. "/Src/Time.cpp" }
        files { home .. "/Src/Service.cpp" }
        files { home .. "/Src/Outdev.cpp" }

        filter "system:Windows"
            kind "StaticLib"
            defines { "WIN32_LEAN_AND_MEAN" }

        filter "system:Linux"
            kind "StaticLib"
            targetname "audiosdk"

    if os.get() == "windows" then
        project "OpenAL"
            kind "SharedLib"
            language "C++"
            targetdir "bin/%{cfg.buildcfg}"

            local home = ROOT .. "/Libs/Audio/Plugins/Outdev/OpenAL"
            includedirs { home }
            includedirs { ROOT .. "/Libs/Audio" }
            links { "AudioSDK" }

            files { home .. "/*.hpp" }
            files { home .. "/*.cpp" }

            vpaths { ["Headers/*"] = home .. "/*.hpp" }
            vpaths { ["Sources/*"] = home .. "/*.cpp" }

            filter "system:Windows"
                defines { "WIN32_LEAN_AND_MEAN" }

                includedirs { home .. "/Vendor" }
                libdirs { home .. "/Vendor" }
                links { "OpenAL32" }

            filter "system:Linux"
                targetname "openal"

        project "Mpg123"
            kind "SharedLib"
            language "C++"
            targetdir "bin/%{cfg.buildcfg}"

            local home = ROOT .. "/Libs/Audio/Plugins/Codec/mpg123"
            includedirs { home }
            includedirs { ROOT .. "/Libs/Audio" }
            links { "AudioSDK" }

            files { home .. "/mpg123.*" }

            filter "system:Windows"
                defines { "WIN32_LEAN_AND_MEAN" }

                includedirs { home .. "/Vendor" }
                libdirs { home .. "/Vendor" }
                links { "libmpg123-0" }

            filter "system:Linux"
                targetname "mpg123"

    else
        project "PulseAudio"
            kind "SharedLib"
            language "C++"
            targetdir "bin/%{cfg.buildcfg}"
            targetname "m4pulseaudio"

            local home = ROOT .. "/Libs/Audio/Plugins/Outdev/PulseAudio"
            includedirs { home }
            includedirs { ROOT .. "/Libs/Audio" }
            links { "AudioSDK" }

            files { home .. "/PulseAudio.*" }

            buildoptions { "`pkg-config libpulse-simple --cflags`" }
            linkoptions { "`pkg-config libpulse-simple --libs`" }
    end

    project "FFmpeg"
        kind "SharedLib"
        language "C++"
        targetdir "bin/%{cfg.buildcfg}"

        local home = ROOT .. "/Libs/Audio/Plugins/Codec/FFmpeg"
        includedirs { home }
        includedirs { ROOT .. "/Libs/Audio" }
        links { "AudioSDK" }

        files { home .. "/FFmpeg.*" }

        filter "system:Windows"
            includedirs { home .. "/Vendor/include" }
            libdirs { home .. "/Vendor/lib" }
            links { "avutil", "avformat", "avcodec", "swresample" }

        filter "system:Linux"
            targetname "m4ffmpeg"

            buildoptions { "`pkg-config libavutil libavformat libavcodec libswresample --cflags`" }
            linkoptions { "`pkg-config libavutil libavformat libavcodec libswresample --libs`" }

    project "M4Player"
        kind "WindowedApp"
        language "C++"
        targetdir "bin/%{cfg.buildcfg}"

        local home = ROOT .. "/M4Player"

        includedirs { home }
        files { home .. "/*.h", home .. "/*.cpp", 
                home .. "/Lyric/*.h", home .. "/Lyric/*.cpp", 
                home .. "/LyricGrabber/*.h", 
                home .. "/LyricGrabber/*.cpp", }

        vpaths { ["Headers/*"] = home .. "/**.h",
                 ["Sources/*"] = home .. "/**.cpp",
                 ["Resources/*"] = home .. "/Resources/*.*" }

        pchheader ( "StdAfx.h" )
        pchsource ( home .. "/StdAfx.cpp" )

        includedirs { home .. "/../Libs/Loki/include" }
        includedirs { home .. "/../Libs/VdkControls/Inc" }
        includedirs { home .. "/../Libs/MyTagLib" }
        includedirs { home .. "/../Libs/Audio" }

        libdirs { "bin/%{cfg.buildcfg}" }
        links { "VdkControls", "MyTagLib", "Audio" }

        defines { "__AUDIO_HOST__" }

        wx_config()

        filter "system:Windows"
            files { home .. "/Lyric/MSW/*.cpp" }
            files { home .. "/Resources/*.*" }

            flags { "WinMain" }

        filter "system:Linux"
            targetname "m4"

            files { home .. "/Lyric/GTK/*.cpp" }
            links { "wx_baseu_net-3.0" }

    project "TT2OO"
        kind "ConsoleApp"
        targetdir "bin/%{cfg.buildcfg}"

        local home = "../TT2OO"
        files { home .. "/**.h", home .. "/**.cpp" }

        defines { "_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1", 
                  "_CRT_SECURE_NO_WARNINGS" }

        includedirs { home .. "/TinyXML" }

        filter "configurations:Debug"
            defines { "DEBUG" }
            flags { "Symbols" }

        filter "configurations:Release"
            defines { "NDEBUG" }
            optimize "On"
