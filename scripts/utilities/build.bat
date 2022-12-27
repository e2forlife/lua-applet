..\xLua.exe compiler.lua --ofile=compiler.c --def=LCOMPILE --app=compiler ..\modules\app.lua ..\modules\progress.lua compiler.lua
del /Q /F ..\..\src\compiler.c
copy compiler.c ..\..\src\compiler.c
del /Q /F compiler.c

..\xLua.exe compiler.lua --ofile=encbin.c --def=BROOKS_ENCBIN --app=encbin ..\modules\app.lua ..\modules\progress.lua ..\modules\base64.lua encbin.lua
del /Q /F ..\..\src\encbin.c
copy encbin.c ..\..\src\encbin.c
del /Q /F encbin.c
