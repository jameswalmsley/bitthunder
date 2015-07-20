DBUILD_OS:=UNKNOWN

ifeq ($(OS),Windows_NT)
	DBUILD_OS:=WIN32
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        DBUILD_OS:=LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
        DBUILD_OS:=OSX
    endif
endif


OS_EXT:=
ifeq ($(DBUILD_OS), WIN32)
OS_EXT:=.exe
endif
ifeq ($(DBUILD_OS), OSX)
OS_EXT:=.osx
endif
