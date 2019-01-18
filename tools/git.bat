echo Pulling git

d:
cd "D:\program\protobuf"
git.exe pull --progress -v --no-rebase "origin"
git.exe submodule update --init -- "third_party/benchmark"
git.exe submodule update --init -- "third_party/googletest"

cd "D:\program\12306"
git.exe pull --progress -v --no-rebase "origin"

cd "D:\program\Stranded"
git.exe pull --progress -v --no-rebase "origin"

cd "D:\program\Stranded\ThirdParty\nedmalloc"
git.exe pull --progress -v --no-rebase "origin"

cd "D:\program\Stranded\ThirdParty\glfw"
git.exe pull --progress -v --no-rebase "origin"

cd "D:\program\Stranded\ThirdParty\glad"
git.exe pull --progress -v --no-rebase "origin"

cd "D:\program\Stranded\ThirdParty\rapidjson"
git.exe pull --progress -v --no-rebase "origin"
git.exe submodule update --progress --init -- "thirdparty/gtest"

cd "D:\program\Stranded\ThirdParty\tinyxml2"
git.exe pull --progress -v --no-rebase "origin"

cd "D:\program\Stranded\tools\decoda"
git.exe pull --progress -v --no-rebase "origin" master

cd "D:\program\Stranded\tools\doom3.gpl"
git.exe pull --progress -v --no-rebase "origin" master

cd "D:\program\CRYENGINE\vs-cryengine-mono-debugger"
git.exe pull --progress -v --no-rebase "origin" master

cd "D:\program\CRYENGINE\GameSDK"
git.exe pull --progress -v --no-rebase "origin" release

cd "D:\program\CRYENGINE\CRYENGINE"
git.exe pull --progress -v --no-rebase "origin"

cd "D:\program\qt5"
git.exe pull --progress -v --no-rebase "origin"
git.exe submodule sync
git.exe submodule update --recursive --progress

cd "D:\program\qt5\qt-creator"
git.exe pull --progress -v --no-rebase "origin" master
git.exe submodule update --progress --init -- "src/shared/qbs"

cd "D:\program\ucc"
TortoiseProc.exe /command:update /path:D:\program\ucc /closeonend:1

cd "D:\program\Unreal\protoc-gen-lua-bin"
git.exe pull --progress -v --no-rebase "origin"

cd "D:\program\Unreal\RuntimeMeshComponent"
git.exe pull --progress -v --no-rebase "origin"

cd "D:\program\Unreal\StreetMap"
git.exe pull --progress -v --no-rebase "origin"

cd "D:\program\Unreal\VaTexAtlas"
git.exe pull --progress -v --no-rebase "origin"

cd "D:\program\Unreal\ARTv2"
git.exe pull --progress -v --no-rebase "origin"

cd "D:\program\Unreal\ThirdParty"
git.exe pull --progress -v --no-rebase "origin"

cd "D:\program\Unreal\unreal.lua"
git.exe pull --progress -v --no-rebase "origin"

cd "D:\program\Unreal\UnrealTournament"
git.exe pull --progress -v --no-rebase "origin"

cd "D:\program\Unreal\UnrealEngine"
git.exe pull --progress -v --no-rebase "origin" master
call "./Setup.bat"
GenerateProjectFiles.bat

echo Finish