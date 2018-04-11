
del /s /q *.ncb
del /s /q .\Release\*.exp
;del /s /q .\Release\*.lib
del /s /q .\Release\*.pdb
del /s /q .\Release\*.log

rmdir /s /q .\debug\
rmdir /s /q .\fsmiddle_DTU\debug\
rmdir /s /q .\fsmiddle_DTU\Release\
rmdir /s /q .\release\*.bin

mkdir debug
copy .\release\*.bin .\debug /b
copy .\release\*.ini .\debug /b
copy .\release\*.txt .\debug /b
