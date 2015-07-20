DBUILD_OS:=UNKNOWN

ifeq ($(OS),Windows_NT)
	DBUILD_OS:=WIN32
else
    UNAME_S := $(shell uname -s)
    UNAME_M := $(shell uname -m)
    ifeq ($(UNAME_S),Linux)
	ifeq ($(UNAME_M),x86_64)
	DBUILD_OS:=LINUX_64
	else
        DBUILD_OS:=LINUX_32
	endif
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
