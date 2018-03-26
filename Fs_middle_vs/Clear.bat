
del /s /q *.ncb
del /s /q .\Release\*.exp
;del /s /q .\Release\*.lib
del /s /q .\Release\*.pdb
del /s /q .\Release\*.log

rmdir /s /q .\debug\
rmdir /s /q .\Fs_middle\debug\
rmdir /s /q .\Fs_middle\Release\
rmdir /s /q .\release\*.bin

mkdir debug
copy .\release\*.bin .\debug /b
copy .\release\*.vec .\debug /b
copy .\release\*.ini .\debug /b
