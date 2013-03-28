/*****************************************************************************
 *     FullFAT - High Performance, Thread-Safe Embedded FAT File-System      *
 *                                                                           *
 *        Copyright(C) 2009  James Walmsley  <james@fullfat-fs.co.uk>        *
 *        Copyright(C) 2011  Hein Tibosch    <hein_tibosch@yahoo.es>         *
 *                                                                           *
 *    See RESTRICTIONS.TXT for extra restrictions on the use of FullFAT.     *
 *                                                                           *
 *    WARNING : COMMERCIAL PROJECTS MUST COMPLY WITH THE GNU GPL LICENSE.    *
 *                                                                           *
 *  Projects that cannot comply with the GNU GPL terms are legally obliged   *
 *    to seek alternative licensing. Contact James Walmsley for details.     *
 *                                                                           *
 *****************************************************************************
 *           See http://www.fullfat-fs.co.uk/ for more information.          *
 *****************************************************************************
 *  This program is free software: you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation, either version 3 of the License, or        *
 *  (at your option) any later version.                                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License        *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *                                                                           *
 *  The Copyright of Hein Tibosch on this project recognises his efforts in  *
 *  contributing to this project. The right to license the project under     *
 *  any other terms (other than the GNU GPL license) remains with the        *
 *  original copyright holder (James Walmsley) only.                         *
 *                                                                           *
 *****************************************************************************
 *  Modification/Extensions/Bugfixes/Improvements to FullFAT must be sent to *
 *  James Walmsley for integration into the main development branch.         *
 *****************************************************************************/

/**
 *	@file		ff_error.c
 *	@author		James Walmsley
 *	@ingroup	ERROR
 *
 *	@defgroup	ERR Error Message
 *	@brief		Used to return pretty strings for FullFAT error codes.
 *
 **/
#include <stdio.h>

#include "ff_config.h"
#include "ff_types.h"
#include "ff_error.h"

#define ARRAY_SIZE(x)	(int)(sizeof(x)/sizeof(x)[0])

#ifdef FF_DEBUG
const struct _FFMODULETAB
{
	const FF_T_INT8 * const strModuleName;
	const FF_T_UINT8		ucModuleID;
} gcpFullFATModuleTable[] =
{
	{"Unknown Module",		1},									// 1 here is ok, as the GetError functions start at the end of the table.
	{"ff_ioman.c",			FF_GETMODULE(FF_MODULE_IOMAN)},
	{"ff_dir.c",			FF_GETMODULE(FF_MODULE_DIR)},
	{"ff_file.c",			FF_GETMODULE(FF_MODULE_FILE)},
	{"ff_fat.c",			FF_GETMODULE(FF_MODULE_FAT)},
	{"ff_crc.c",			FF_GETMODULE(FF_MODULE_CRC)},
	{"ff_format.c",			FF_GETMODULE(FF_MODULE_FORMAT)},
	{"ff_hash.c",			FF_GETMODULE(FF_MODULE_HASH)},
	{"ff_memory.c",			FF_GETMODULE(FF_MODULE_MEMORY)},
	{"ff_string.c",			FF_GETMODULE(FF_MODULE_STRING)},
	{"ff_unicode.c",		FF_GETMODULE(FF_MODULE_UNICODE)},
	{"ff_safety.c",			FF_GETMODULE(FF_MODULE_SAFETY)},
	{"ff_time.c",			FF_GETMODULE(FF_MODULE_TIME)},
	{"Platform Driver",		FF_GETMODULE(FF_MODULE_DRIVER)},
};

const struct _FFFUNCTIONTAB
{
	const FF_T_INT8 * const strFunctionName;
	const FF_T_UINT16		ucFunctionID;
} gcpFullFATFunctionTable[] =
{
	{"Unknown Function",	1},
//----- FF_IOMAN - The FullFAT I/O Manager
	{"FF_CreateIOMAN",           FF_GETMOD_FUNC(FF_CREATEIOMAN) },
	{"FF_DestroyIOMAN",          FF_GETMOD_FUNC(FF_DESTROYIOMAN) },
	{"FF_RegisterBlkDevice",     FF_GETMOD_FUNC(FF_REGISTERBLKDEVICE) },
	{"FF_UnregisterBlkDevice",   FF_GETMOD_FUNC(FF_UNREGISTERBLKDEVICE) },
	{"FF_MountPartition",        FF_GETMOD_FUNC(FF_MOUNTPARTITION) },
	{"FF_UnmountPartition",      FF_GETMOD_FUNC(FF_UNMOUNTPARTITION) },
	{"FF_FlushCache",            FF_GETMOD_FUNC(FF_FLUSHCACHE) },
	{"FF_GetPartitionBlockSize", FF_GETMOD_FUNC(FF_GETPARTITIONBLOCKSIZE) },
	{"FF_BlockRead",             FF_GETMOD_FUNC(FF_BLOCKREAD) },
	{"FF_BlockWrite",            FF_GETMOD_FUNC(FF_BLOCKWRITE) },
	{"FF_DetermineFatType",      FF_GETMOD_FUNC(FF_DETERMINEFATTYPE) },
	{"FF_GetEfiPartitionEntry",  FF_GETMOD_FUNC(FF_GETEFIPARTITIONENTRY) },
	{"FF_UserDriver",            FF_GETMOD_FUNC(FF_USERDRIVER) },

//----- FF_DIR - The FullFAT directory handling routines
	{"FF_FindNextInDir",         FF_GETMOD_FUNC(FF_FINDNEXTINDIR) },
	{"FF_FetchEntryWithContext", FF_GETMOD_FUNC(FF_FETCHENTRYWITHCONTEXT) },
	{"FF_PushEntryWithContext",  FF_GETMOD_FUNC(FF_PUSHENTRYWITHCONTEXT) },
	{"FF_GetEntry",              FF_GETMOD_FUNC(FF_GETENTRY) },
	{"FF_FindFirst",             FF_GETMOD_FUNC(FF_FINDFIRST) },
	{"FF_FindNext",              FF_GETMOD_FUNC(FF_FINDNEXT) },
	{"FF_RewindFind",            FF_GETMOD_FUNC(FF_REWINDFIND) },
	{"FF_FindFreeDirent",        FF_GETMOD_FUNC(FF_FINDFREEDIRENT) },
	{"FF_PutEntry",              FF_GETMOD_FUNC(FF_PUTENTRY) },
	{"FF_CreateShortName",       FF_GETMOD_FUNC(FF_CREATESHORTNAME) },
	{"FF_CreateLFNs",            FF_GETMOD_FUNC(FF_CREATELFNS) },
	{"FF_ExtendDirectory",       FF_GETMOD_FUNC(FF_EXTENDDIRECTORY) },
	{"FF_MkDir",                 FF_GETMOD_FUNC(FF_MKDIR) },

//----- FF_FILE - The FullFAT file handling routines
	{"FF_GetModeBits",           FF_GETMOD_FUNC(FF_GETMODEBITS) },
	{"FF_Open",                  FF_GETMOD_FUNC(FF_OPEN) },
	{"FF_isDirEmpty",            FF_GETMOD_FUNC(FF_ISDIREMPTY) },
	{"FF_RmDir",                 FF_GETMOD_FUNC(FF_RMDIR) },
	{"FF_RmFile",                FF_GETMOD_FUNC(FF_RMFILE) },
	{"FF_Move",                  FF_GETMOD_FUNC(FF_MOVE) },
	{"FF_isEOF",                 FF_GETMOD_FUNC(FF_ISEOF) },
	{"FF_GetSequentialClusters", FF_GETMOD_FUNC(FF_GETSEQUENTIALCLUSTERS) },
	{"FF_ReadClusters",          FF_GETMOD_FUNC(FF_READCLUSTERS) },
	{"FF_ExtendFile",            FF_GETMOD_FUNC(FF_EXTENDFILE) },
	{"FF_WriteClusters",         FF_GETMOD_FUNC(FF_WRITECLUSTERS) },
	{"FF_Read",                  FF_GETMOD_FUNC(FF_READ) },
	{"FF_GetC",                  FF_GETMOD_FUNC(FF_GETC) },
	{"FF_GetLine",               FF_GETMOD_FUNC(FF_GETLINE) },
	{"FF_Tell",                  FF_GETMOD_FUNC(FF_TELL) },
	{"FF_Write",                 FF_GETMOD_FUNC(FF_WRITE) },
	{"FF_PutC",                  FF_GETMOD_FUNC(FF_PUTC) },
	{"FF_Seek",                  FF_GETMOD_FUNC(FF_SEEK) },
	{"FF_Invalidate",            FF_GETMOD_FUNC(FF_INVALIDATE) },
	{"FF_CheckValid",            FF_GETMOD_FUNC(FF_CHECKVALID) },
	{"FF_Close",                 FF_GETMOD_FUNC(FF_CLOSE) },
	{"FF_SetTime",               FF_GETMOD_FUNC(FF_SETTIME) },
	{"FF_BytesLeft",             FF_GETMOD_FUNC(FF_BYTESLEFT) },
	{"FF_SetFileTime",           FF_GETMOD_FUNC(FF_SETFILETIME) },

//----- FF_FAT - The FullFAT FAT handling routines
	{"FF_getFatEntry",           FF_GETMOD_FUNC(FF_GETFATENTRY) },
	{"FF_ClearCluster",          FF_GETMOD_FUNC(FF_CLEARCLUSTER) },
	{"FF_putFatEntry",           FF_GETMOD_FUNC(FF_PUTFATENTRY) },
	{"FF_FindFreeCluster",       FF_GETMOD_FUNC(FF_FINDFREECLUSTER) },
	{"FF_CountFreeClusters",     FF_GETMOD_FUNC(FF_COUNTFREECLUSTERS) },

//----- FF_HASH - The FullFAT hashing routines
	{"FF_ClearHashTable",        FF_GETMOD_FUNC(FF_CLEARHASHTABLE) },
	{"FF_SetHash",               FF_GETMOD_FUNC(FF_SETHASH) },
	{"FF_ClearHash",             FF_GETMOD_FUNC(FF_CLEARHASH) },
	{"FF_DestroyHashTable",      FF_GETMOD_FUNC(FF_DESTROYHASHTABLE) },

//----- FF_UNICODE - The FullFAT hashing routines
	{"FF_Utf8ctoUtf16c",         FF_GETMOD_FUNC(FF_UTF8CTOUTF16C) },
	{"FF_Utf16ctoUtf8c",         FF_GETMOD_FUNC(FF_UTF16CTOUTF8C) },
	{"FF_Utf32ctoUtf16c",        FF_GETMOD_FUNC(FF_UTF32CTOUTF16C) },
	{"FF_Utf16ctoUtf32c",        FF_GETMOD_FUNC(FF_UTF16CTOUTF32C) },

//----- FF_FORMAT - The FullFAT format routine
	{"FF_FormatPartition",       FF_GETMOD_FUNC(FF_FORMATPARTITION) },
};

const struct _FFERRTAB
{
	const FF_T_INT8 * const strErrorString;
	const FF_T_UINT8		ucErrorCode;		// Currently there are less then 256 errors, so lets keep this table small.
} gcpFullFATErrorTable[] =
{
	{"Unknown or Generic Error! - Please contact james@fullfat-fs.co.uk",			1},
	{"No Error",																	FF_ERR_NONE},
	{"Null Pointer provided, (probably for IOMAN)",									FF_ERR_NULL_POINTER},
	{"Not enough memory (malloc() returned NULL)",									FF_ERR_NOT_ENOUGH_MEMORY},
	{"Device Driver returned a FATAL error!",										FF_ERR_DEVICE_DRIVER_FAILED},
	{"The blocksize is not 512 multiple",											FF_ERR_IOMAN_BAD_BLKSIZE},
	{"The memory size, is not a multiple of the blocksize. (Atleast 2 Blocks)",		FF_ERR_IOMAN_BAD_MEMSIZE},
	{"Device is already registered, use FF_UnregisterBlkDevice() first",			FF_ERR_IOMAN_DEV_ALREADY_REGD},
	{"No mountable partition was found on the specified device",					FF_ERR_IOMAN_NO_MOUNTABLE_PARTITION},
    {"The format of the MBR was unrecognised",										FF_ERR_IOMAN_INVALID_FORMAT},
    {"The provided partition number is out-of-range (0 - 3)",						FF_ERR_IOMAN_INVALID_PARTITION_NUM},
    {"The selected partition / volume doesn't appear to be FAT formatted",			FF_ERR_IOMAN_NOT_FAT_FORMATTED},
    {"Cannot register device. (BlkSize not a multiple of 512)",						FF_ERR_IOMAN_DEV_INVALID_BLKSIZE},
    {"Cannot unregister device, a partition is still mounted",						FF_ERR_IOMAN_PARTITION_MOUNTED},
    {"Cannot unmount the partition while there are active FILE handles",			FF_ERR_IOMAN_ACTIVE_HANDLES},
	{"The GPT partition header appears to be corrupt, refusing to mount",			FF_ERR_IOMAN_GPT_HEADER_CORRUPT},
	{"Disk full",                                                                   FF_ERR_IOMAN_NOT_ENOUGH_FREE_SPACE},
	{"Attempted to Read a sector out of bounds",									FF_ERR_IOMAN_OUT_OF_BOUNDS_READ},
	{"Attempted to Write a sector out of bounds",									FF_ERR_IOMAN_OUT_OF_BOUNDS_WRITE},
	{"I/O driver is busy",                                                          FF_ERR_IOMAN_DRIVER_BUSY},
	{"I/O driver returned fatal error",                                             FF_ERR_IOMAN_DRIVER_FATAL_ERROR},

    {"Cannot open the file, file already in use",									FF_ERR_FILE_ALREADY_OPEN},
    {"The specified file could not be found",										FF_ERR_FILE_NOT_FOUND},
    {"Cannot open a Directory",														FF_ERR_FILE_OBJECT_IS_A_DIR},
	{"Cannot open for writing: File is marked as Read-Only",						FF_ERR_FILE_IS_READ_ONLY},
    {"Path not found",																FF_ERR_FILE_INVALID_PATH},
	{"File operation failed - the file was not opened for writing",					FF_ERR_FILE_NOT_OPENED_IN_WRITE_MODE},
	{"File operation failed - the file was not opened for reading",					FF_ERR_FILE_NOT_OPENED_IN_READ_MODE},
	{"File operation failed - could not extend file",								FF_ERR_FILE_EXTEND_FAILED},
	{"Destination file already exists",												FF_ERR_FILE_DESTINATION_EXISTS},
	{"Source file was not found",													FF_ERR_FILE_SOURCE_NOT_FOUND},
	{"Destination path (dir) was not found",										FF_ERR_FILE_DIR_NOT_FOUND},
	{"Failed to create the directory Entry",										FF_ERR_FILE_COULD_NOT_CREATE_DIRENT},
	{"A file handle was invalid",													FF_ERR_FILE_BAD_HANDLE},
#ifdef FF_REMOVABLE_MEDIA
	{"File handle got invalid because media was removed",							FF_ERR_FILE_MEDIA_REMOVED},
#endif
    {"A file or folder of the same name already exists",							FF_ERR_DIR_OBJECT_EXISTS},
    {"FF_ERR_DIR_DIRECTORY_FULL",													FF_ERR_DIR_DIRECTORY_FULL},
    {"FF_ERR_DIR_END_OF_DIR",														FF_ERR_DIR_END_OF_DIR},
    {"The directory is not empty",													FF_ERR_DIR_NOT_EMPTY},
	{"Could not extend File or Folder - No Free Space!",							FF_ERR_FAT_NO_FREE_CLUSTERS},
	{"Could not find the directory specified by the path",							FF_ERR_DIR_INVALID_PATH},
	{"The Root Dir is full, and cannot be extended on Fat12 or 16 volumes",			FF_ERR_DIR_CANT_EXTEND_ROOT_DIR},
	{"Not enough space to extend the directory.",									FF_ERR_DIR_EXTEND_FAILED},
	{"Name exceeds the number of allowed charachters for a filename",				FF_ERR_DIR_NAME_TOO_LONG},

#ifdef FF_UNICODE_SUPPORT
	{"An invalid Unicode charachter was provided!",									FF_ERR_UNICODE_INVALID_CODE},
	{"Not enough space in the UTF-16 buffer to encode the entire sequence",			FF_ERR_UNICODE_DEST_TOO_SMALL},
	{"An invalid UTF-16 sequence was encountered",									FF_ERR_UNICODE_INVALID_SEQUENCE},
	{"Filename exceeds MAX long-filename length when converted to UTF-16",			FF_ERR_UNICODE_CONVERSION_EXCEEDED},
#endif
};

/**
 *	@public
 *	@brief	Returns a pointer to a string relating to a FullFAT error code.
 *
 *	@param	iErrorCode	The error code.
 *
 *	@return	Pointer to a string describing the error.
 *
 **/
const FF_T_INT8 *FF_GetErrMessage(FF_ERROR iErrorCode) {
    FF_T_UINT32 stCount = ARRAY_SIZE(gcpFullFATErrorTable);
    while (stCount--){
        if (((FF_T_UINT) gcpFullFATErrorTable[stCount].ucErrorCode) == FF_GETERROR(iErrorCode)) {
            return gcpFullFATErrorTable[stCount].strErrorString;
        }
    }
	return gcpFullFATErrorTable[0].strErrorString;
}

const FF_T_INT8 *FF_GetErrModule(FF_ERROR iErrorCode) {
	FF_T_UINT32 stCount = ARRAY_SIZE(gcpFullFATModuleTable);
	while (stCount--) {
		if(gcpFullFATModuleTable[stCount].ucModuleID == FF_GETMODULE(iErrorCode)) {
			return gcpFullFATModuleTable[stCount].strModuleName;
		}
	}
	return gcpFullFATModuleTable[0].strModuleName;
}

const FF_T_INT8 *FF_GetErrFunction(FF_ERROR iErrorCode) {
	FF_T_UINT32 stCount = ARRAY_SIZE(gcpFullFATFunctionTable);
	FF_T_UINT16 ModuleFunc = FF_GETMOD_FUNC(iErrorCode);
	while (stCount--) {
		if(gcpFullFATFunctionTable[stCount].ucFunctionID == ModuleFunc) {
			return gcpFullFATFunctionTable[stCount].strFunctionName;
		}
	}
	return gcpFullFATFunctionTable[0].strFunctionName;
}

const FF_T_INT8 *FF_GetErrDescription(FF_ERROR iErrorCode, char *apBuf, int aMaxlen) {
	if (FF_isERR(iErrorCode)) {
		snprintf (apBuf, aMaxlen, "%s::%s::%s",
			FF_GetErrModule(iErrorCode),
			FF_GetErrFunction(iErrorCode),
			FF_GetErrMessage(iErrorCode));
	} else {
		snprintf (apBuf, aMaxlen, "No error");
	}
	return apBuf;
}

#endif
