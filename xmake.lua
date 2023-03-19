-- project
set_project("dfint-hook")
set_languages("cxxlatest")


-- allowed
set_allowedarchs("windows|x64")
set_allowedmodes("debug", "release")
-- default
set_defaultarchs("windows|x64")
set_defaultmode("release")

output_dir = "C:/Users/aka/Desktop/Game/"


-- rules
add_rules("plugin.vsxmake.autoupdate")
add_rules("mode.release")

-- deps
--3rd party local libs
add_linkdirs("deps/lib")
add_linkdirs("C:\\Users\\aka\\scoop\\apps\\vcpkg\\current\\installed\\x64-windows\\lib")
add_includedirs("deps/include")
--3rd party remote libs
add_requires("spdlog")
add_requires("vcpkg::detours")
add_requires("toml++")
add_requires("vcpkg::rapidfuzz")
-- add_requires("libomp", {optional = true})
add_linkdirs("C:\\Users\\aka\\scoop\\apps\\vcpkg\\current\\installed\\x64-windows\\lib")
add_linkdirs("C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.22000.0\\um\\x64")
add_includedirs("C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.22000.0\\um")
add_includedirs("C:\\Users\\aka\\scoop\\apps\\vcpkg\\current\\installed\\x64-windows\\include")
add_includedirs("C:\\Users\\aka\\scoop\\apps\\vcpkg\\current\\installed\\x64-windows\\bin")
add_requires("libcurl")
add_requires("vcpkg::curl")
add_packages("openmp")
add_requires("vcpkg::jsoncpp")

option("hook_version")
    set_default("not-defined")

target("dfint_hook")
    set_default(true)
    set_kind("shared")
    set_basename("dfint_hook")
    set_targetdir("C:/Users/aka/Desktop/Game/dfint_data")  -- build to DF dir, handy for testing
    set_pcxxheader("src/hook/pch.h")
    add_files("src/hook/*.cpp")
    add_packages("spdlog", "vcpkg::detours", "toml++","vcpkg::rapidfuzz","openmp")
    add_defines("HOOK_VERSION=\"$(hook_version)\"")
    add_links("libcurl","jsoncpp","libomp")
    add_packages("vcpkg::curl","libcurl","vcpkg::jsoncpp")


target("dfint_launcher")
    set_default(true)
    set_kind("binary")
    set_basename("dfint_launcher")
    set_targetdir("C:/Users/aka/Desktop/Game/") -- build to DF dir, handy for testing
    add_files("src/launcher/*.cpp")
    add_packages("vcpkg::detours")



after_build(function (target)
    os.cp("C:/Users/aka/scoop/apps/vcpkg/current/installed/x64-windows/bin/libcurl.dll","C:/Users/aka/Desktop/Game/")
    os.cp("C:/Program Files (x86)/Buzz/PyQt6/Qt6/bin/MSVCP140.dll","C:/Users/aka/Desktop/Game/")
    os.cp("C:/Program Files (x86)/Buzz/VCRUNTIME140.dll","C:/Users/aka/Desktop/Game/")
    os.cp("C:/Program Files (x86)/Buzz/vcruntime140_1.dll","C:/Users/aka/Desktop/Game/")
end)