@echo off
pushd ..\ProjectSetup
g++ ProjectSetup.cpp -o ProjectSetup.exe
popd
pushd ..\ProjectSetup
ProjectSetup.exe ../premake5.lua Templates/ProjectPremakeTemplate.lua Templates/Main.cpp Templates/RuleAndTokenIDS.h Templates/LexerTemplate.h Templates/LexerTemplate.cpp Templates/ParserTemplate.h Templates/ParserTemplate.cpp
popd
call Win-GenProjects.bat
PAUSE