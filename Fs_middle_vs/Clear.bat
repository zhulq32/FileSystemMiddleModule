
del /s /q *.ncb
del /s /q .\Release\*.exp
;del /s /q .\Release\*.lib
del /s /q .\Release\*.pdb
del /s /q .\Release\*.log

rmdir /s /q .\debug\
rmdir /s /q .\Fs_middle_A\debug\
rmdir /s /q .\Fs_middle_A\Release\
rmdir /s /q .\Fs_middle_S\debug\
rmdir /s /q .\Fs_middle_S\Release\
rmdir /s /q .\fsmiddle_DTU\debug\
rmdir /s /q .\fsmiddle_DTU\Release\
rmdir /s /q .\release\*.bin

mkdir debug
copy .\release\*.bin .\debug /b
copy .\release\*.ini .\debug /b
copy .\release\*.txt .\debug /b
