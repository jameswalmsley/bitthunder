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
 *	@file		ff_dir.c
 *	@author		James Walmsley
 *	@ingroup	DIR
 *
 *	@defgroup	DIR Handles Directory Traversal
 *	@brief		Handles DIR access and traversal.
 *
 *	Provides FindFirst() and FindNext() Interfaces
 **/

#include "ff_dir.h"
#include "ff_string.h"
#include "ff_unicode.h"
#include <stdio.h>


#ifdef FF_UNICODE_SUPPORT
#include <wchar.h>
#endif

#ifdef WIN32
#define wcsicmp _wcsicmp
#else
#define wcsicmp wcscasecmp
#include <ctype.h>  // tolower()
#ifndef FF_NOSTRCASECMP
int strcasecmp(const char *s1, const char *s2)
{
  unsigned char c1,c2;
  do {
    c1 = *s1++;
    c2 = *s2++;
    c1 = (unsigned char) tolower( (unsigned char) c1);
    c2 = (unsigned char) tolower( (unsigned char) c2);
  }
  while((c1 == c2) && (c1 != '\0'));
  return (int) c1-c2;
}
#endif
#endif


#ifdef FF_UNICODE_SUPPORT
static void FF_ProcessShortName(FF_T_WCHAR *name);
#else
static void FF_ProcessShortName(FF_T_INT8 *name);
#endif

void FF_lockDIR(FF_IOMAN *pIoman) {
	FF_PendSemaphore(pIoman->pSemaphore);	// Use Semaphore to protect DIR modifications.
	{
		while((pIoman->Locks & FF_DIR_LOCK)) {
			FF_ReleaseSemaphore(pIoman->pSemaphore);
			FF_Yield();						// Keep Releasing and Yielding until we have the DIR protector.
			FF_PendSemaphore(pIoman->pSemaphore);
		}
		pIoman->Locks |= FF_DIR_LOCK;
	}
	FF_ReleaseSemaphore(pIoman->pSemaphore);
}

void FF_unlockDIR(FF_IOMAN *pIoman) {
	FF_PendSemaphore(pIoman->pSemaphore);
	{
		pIoman->Locks &= ~FF_DIR_LOCK;
	}
	FF_ReleaseSemaphore(pIoman->pSemaphore);
}

static FF_T_UINT8 FF_CreateChkSum(const FF_T_UINT8 *pa_pShortName) {
	FF_T_UINT8	cNameLen;
	FF_T_UINT8	ChkSum = 0;

	for(cNameLen = 11; cNameLen != 0; cNameLen--) {
		ChkSum = ((ChkSum & 1) ? 0x80 : 0) + (ChkSum >> 1) + *pa_pShortName++;
	}
	return ChkSum;
}


FF_ERROR FF_FindNextInDir(FF_IOMAN *pIoman, FF_DIRENT *pDirent, FF_FETCH_CONTEXT *pFetchContext) {

	FF_T_UINT8		EntryBuffer[32];
	FF_ERROR		Error;
#ifndef FF_LFN_SUPPORT
	FF_T_UINT8		numLFNs;
#endif
	if(!pIoman) {
		return FF_ERR_NULL_POINTER | FF_FINDNEXTINDIR;
	}

	for(; pDirent->CurrentItem < 0xFFFF; pDirent->CurrentItem += 1) {

		Error = FF_FetchEntryWithContext(pIoman, pDirent->CurrentItem, pFetchContext, EntryBuffer);

		if(FF_isERR(Error)) {
			return Error;
		}

		if(EntryBuffer[0] != 0xE5) {
			if(FF_isEndOfDir(EntryBuffer)){
				return FF_ERR_DIR_END_OF_DIR | FF_FINDNEXTINDIR;
			}
			pDirent->Attrib = FF_getChar(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB));
			if((pDirent->Attrib & FF_FAT_ATTR_LFN) == FF_FAT_ATTR_LFN) {

#ifdef FF_LFN_SUPPORT
				Error = FF_PopulateLongDirent(pIoman, pDirent, pDirent->CurrentItem, pFetchContext);
				if(FF_isERR(Error)) {
					return Error;
				}
				return FF_ERR_NONE;
#else
				// LFN Processing
				numLFNs = (FF_T_UINT8)(EntryBuffer[0] & ~0x40);
				//pDirent->NumLFNs = numLFNs;
				pDirent->CurrentItem += (numLFNs - 1);
#endif
			} else if((pDirent->Attrib & FF_FAT_ATTR_VOLID) == FF_FAT_ATTR_VOLID) {
				// Do Nothing

			} else {
				FF_PopulateShortDirent(pIoman, pDirent, EntryBuffer);
				pDirent->CurrentItem += 1;
				return FF_ERR_NONE;
			}
		}
	}

	return FF_ERR_DIR_END_OF_DIR | FF_FINDNEXTINDIR;
}

#ifdef FF_UNICODE_SUPPORT
static FF_T_BOOL FF_ShortNameExists(FF_IOMAN *pIoman, FF_T_UINT32 ulDirCluster, FF_T_WCHAR *szShortName, FF_ERROR *pError) {
#else
static FF_T_BOOL FF_ShortNameExists(FF_IOMAN *pIoman, FF_T_UINT32 ulDirCluster, FF_T_INT8 *szShortName, FF_ERROR *pError) {
#endif

    FF_T_UINT16 		i;
    FF_T_UINT8      	EntryBuffer[32];
    FF_T_UINT8     		Attrib;
	FF_FETCH_CONTEXT	FetchContext;

#ifdef FF_UNICODE_SUPPORT
	FF_T_WCHAR			UTF16EntryBuffer[32];
#endif

#ifdef FF_HASH_CACHE
	FF_T_UINT32			ulHash;
#endif

	*pError = FF_ERR_NONE;


#ifdef FF_HASH_CACHE
	if(!FF_DirHashed(pIoman, ulDirCluster)) {
		// Hash the directory
		FF_HashDir(pIoman, ulDirCluster);
	}

#if FF_HASH_FUNCTION == CRC16
	ulHash = (FF_T_UINT32) FF_GetCRC16((FF_T_UINT8 *) szShortName, strlen(szShortName));
#elif FF_HASH_FUNCTION == CRC8
	ulHash = (FF_T_UINT32) FF_GetCRC8((FF_T_UINT8 *) szShortName, strlen(szShortName));
#endif

	if(!FF_CheckDirentHash(pIoman, ulDirCluster, ulHash)) {
		return FF_FALSE;
	}

#endif

	*pError = FF_InitEntryFetch(pIoman, ulDirCluster, &FetchContext);
	if(FF_isERR(*pError)) {
		return FF_FALSE;
	}

	for(i = 0; i < 0xFFFF; i++) {
		*pError = FF_FetchEntryWithContext(pIoman, i, &FetchContext, EntryBuffer);
		if(FF_isERR(*pError)) {
			break;
		}
		Attrib = FF_getChar(EntryBuffer, FF_FAT_DIRENT_ATTRIB);
		if(FF_getChar(EntryBuffer, 0x00) != 0xE5) {
			if(Attrib != FF_FAT_ATTR_LFN) {
#ifdef FF_UNICODE_SUPPORT
				// Convert Entry Buffer into UTF16
				FF_cstrntowcs(UTF16EntryBuffer, (FF_T_INT8 *) EntryBuffer, 32);
				FF_ProcessShortName(UTF16EntryBuffer);
#else
				FF_ProcessShortName((FF_T_INT8 *)EntryBuffer);
#endif
				if(FF_isEndOfDir(EntryBuffer)) {
					*pError = FF_CleanupEntryFetch(pIoman, &FetchContext);
					return FF_FALSE;
				}
#ifdef FF_UNICODE_SUPPORT
				if(wcscmp(szShortName, UTF16EntryBuffer) == 0) {
#else
				if(strcmp(szShortName, (FF_T_INT8 *)EntryBuffer) == 0) {
#endif
					*pError = FF_CleanupEntryFetch(pIoman, &FetchContext);
					return FF_TRUE;
				}
			}
		}
	}

	*pError = FF_CleanupEntryFetch(pIoman, &FetchContext);
    return FF_FALSE;
}


#ifdef FF_UNICODE_SUPPORT
FF_T_UINT32 FF_FindEntryInDir(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, const FF_T_WCHAR *name, FF_T_UINT8 pa_Attrib, FF_DIRENT *pDirent, FF_ERROR *pError) {
#else
FF_T_UINT32 FF_FindEntryInDir(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, const FF_T_INT8 *name, FF_T_UINT8 pa_Attrib, FF_DIRENT *pDirent, FF_ERROR *pError) {
#endif

	FF_FETCH_CONTEXT FetchContext;
	FF_T_UINT8	*src;	// Pointer to read from pBuffer
	FF_T_UINT8	*lastSrc;
#ifdef FF_UNICODE_UTF8_SUPPORT
	FF_T_SINT32	utf8Error;
	//FF_T_UINT8	bSurrogate = FF_FALSE;
#endif


#ifdef FF_LFN_SUPPORT

#ifdef FF_UNICODE_SUPPORT
	FF_T_WCHAR	*ptr;		// Pointer to store a LFN
	FF_T_WCHAR	*lastPtr = pDirent->FileName + sizeof(pDirent->FileName);
#else
	FF_T_INT8	*ptr;		// Pointer to store a LFN
	FF_T_INT8	*lastPtr = pDirent->FileName + sizeof(pDirent->FileName);
#endif

	FF_T_UINT16	lfnItem = 0;
	FF_T_UINT8	CheckSum = 0;
	FF_T_INT8	numLFNs = 0;
	FF_T_INT8	totalLFNs = 0;
	FF_T_UINT8	lastAttrib;

	FF_T_INT32	i;

#endif


	if(pError) {
		*pError = FF_ERR_NONE;
	}

	pDirent->CurrentItem = 0;
	pDirent->Attrib = 0;

	FF_InitEntryFetch(pIoman, DirCluster, &FetchContext);

	while(pDirent->CurrentItem < 0xFFFF) {
		if (FF_FetchEntryWithContext(pIoman, pDirent->CurrentItem, &FetchContext, NULL)) {
			break;
		}
		lastSrc = FetchContext.pBuffer->pBuffer + pIoman->BlkSize;
		for (src = FetchContext.pBuffer->pBuffer; src < lastSrc; src += 32, pDirent->CurrentItem++) {
			if (FF_isEndOfDir(src)) {	// 0x00: end-of-dir
				*pError = FF_CleanupEntryFetch(pIoman, &FetchContext);
				return 0;
			}
			if (src[0] == 0xE5) {	// Entry not used
				pDirent->Attrib = 0;
				continue;
			}
#ifdef FF_LFN_SUPPORT
			lastAttrib = pDirent->Attrib;
#endif
			pDirent->Attrib = FF_getChar(src, FF_FAT_DIRENT_ATTRIB);
			if((pDirent->Attrib & FF_FAT_ATTR_LFN) == FF_FAT_ATTR_LFN) {
				// LFN Processing
#ifdef FF_LFN_SUPPORT
				if (numLFNs == 0 || (lastAttrib & FF_FAT_ATTR_LFN) != FF_FAT_ATTR_LFN) {
					totalLFNs = numLFNs = (FF_T_UINT8)(src[0] & ~0x40);
					lfnItem = pDirent->CurrentItem;
					CheckSum = FF_getChar(src, FF_FAT_LFN_CHECKSUM);
					lastPtr[-1] = '\0';
				}
				if (numLFNs) {
					numLFNs--;
					ptr = pDirent->FileName + (numLFNs * 13);

					/*
						This section needs to extract the name and do the comparison
						dependent on UNICODE settings in the ff_config.h file.
					*/
#ifdef FF_UNICODE_SUPPORT
					// Add UTF-16 Routine here
					memcpy(ptr, &src[FF_FAT_LFN_NAME_1], 10);	// Copy first 5 UTF-16 chars (10 bytes).
					ptr += 5;									// Increment Filename pointer 5 utf16 chars.

					memcpy(ptr, &src[FF_FAT_LFN_NAME_2], 12);	//Copy next 6 chars (12 bytes).
					ptr += 6;

					memcpy(ptr, &src[FF_FAT_LFN_NAME_3], 4);		// You're getting the idea by now!
					ptr += 2;

#endif
#ifdef FF_UNICODE_UTF8_SUPPORT
					// UTF-8 Routine here
					for(i = 0; i < 5 && ptr < lastPtr; i++) {
						// Was there a surrogate sequence? -- Add handling here.
						utf8Error = FF_Utf16ctoUtf8c((FF_T_UINT8 *) ptr, (FF_T_UINT16 *) &src[FF_FAT_LFN_NAME_1 + (2*i)], lastPtr - ptr);
						if(utf8Error > 0) {
							ptr += utf8Error;

						} else {
							if(FF_GETERROR(utf8Error) == FF_ERR_UNICODE_INVALID_SEQUENCE) {
								// Handle potential surrogate sequence across entries.

							}
						}
					}

					for(i = 0; i < 6 && ptr < lastPtr; i++) {
						// Was there a surrogate sequence? -- To add handling here.
						utf8Error = FF_Utf16ctoUtf8c((FF_T_UINT8 *) ptr, (FF_T_UINT16 *) &src[FF_FAT_LFN_NAME_2 + (2*i)], lastPtr - ptr);
						if(utf8Error > 0) {
							ptr += utf8Error;
						} else {
							if(FF_GETERROR(utf8Error) == FF_ERR_UNICODE_INVALID_SEQUENCE) {
								// Handle potential surrogate sequence across entries.
							}
						}
					}

					for(i = 0; i < 2 && ptr < lastPtr; i++) {
						// Was there a surrogate sequence? -- To add handling here.
						utf8Error = FF_Utf16ctoUtf8c((FF_T_UINT8 *) ptr, (FF_T_UINT16 *) &src[FF_FAT_LFN_NAME_3 + (2*i)], lastPtr - ptr);
						if(utf8Error > 0) {
							ptr += utf8Error;
						} else {
							if(FF_GETERROR(utf8Error) == FF_ERR_UNICODE_INVALID_SEQUENCE) {
								// Handle potential surrogate sequence across entries.
							}
						}
					}
#endif // FF_UNICODE_UTF8_SUPPORT

#if !defined(FF_UNICODE_SUPPORT) && !defined(FF_UNICODE_UTF8_SUPPORT)
					for(i = 0; i < 10 && ptr < lastPtr; i += 2)
						*(ptr++) = src[FF_FAT_LFN_NAME_1 + i];

					for(i = 0; i < 12 && ptr < lastPtr; i += 2)
						*(ptr++) = src[FF_FAT_LFN_NAME_2 + i];

					for(i = 0; i < 4 && ptr < lastPtr; i += 2)
						*(ptr++) = src[FF_FAT_LFN_NAME_3 + i];
#endif
					if (numLFNs == totalLFNs-1 && ptr < lastPtr)
						*ptr = '\0';	// Important when name len is multiple of 13

				}
#endif // FF_LFN_SUPPORT
				continue;
			}
			if ((pDirent->Attrib & FF_FAT_ATTR_VOLID) == FF_FAT_ATTR_VOLID) {
#ifdef FF_LFN_SUPPORT
				 totalLFNs = 0;
#endif
				continue;
			}
#ifdef FF_LFN_SUPPORT
			if(!totalLFNs || CheckSum != FF_CreateChkSum(src))
#endif
			{
#ifdef FF_UNICODE_SUPPORT
				for(i = 0; i < 11; i++) {
					pDirent->FileName[i] = (FF_T_WCHAR) src[i];
				}
				FF_ProcessShortName(pDirent->FileName);
#else
				memcpy(pDirent->FileName, src, 11);
				FF_ProcessShortName(pDirent->FileName);
#endif
#ifdef FF_LFN_SUPPORT
				totalLFNs = 0;
#endif
			}

			if((pDirent->Attrib & pa_Attrib) == pa_Attrib){
#ifdef FF_UNICODE_SUPPORT
				if(!wcsicmp(name, pDirent->FileName)) {
#else
				if (!FF_stricmp(name, pDirent->FileName)) {
#endif
					// Finally get the complete information
#ifdef FF_LFN_SUPPORT
					if (totalLFNs) {
						*pError = FF_PopulateLongDirent(pIoman, pDirent, lfnItem, &FetchContext);
						if(FF_isERR(*pError)) {
							FF_CleanupEntryFetch(pIoman, &FetchContext);
							return 0;
						}
					} else
#endif
					{
						FF_PopulateShortDirent(pIoman, pDirent, src);
						// HT: CurrentItem wasn't increased here
						pDirent->CurrentItem += 1;
					}
					// Object found!
					*pError = FF_CleanupEntryFetch(pIoman, &FetchContext);
					return pDirent->ObjectCluster;	// Return the cluster number
				}
			}
#ifdef FF_LFN_SUPPORT
			totalLFNs = 0;
#endif
		}
	}	// for (src = FetchContext.pBuffer->pBuffer; src < lastSrc; src += 32, pDirent->CurrentItem++)

	*pError = FF_CleanupEntryFetch(pIoman, &FetchContext);

	return 0;
}



/**
 *	@private
 **/
#ifdef FF_UNICODE_SUPPORT
FF_T_UINT32 FF_FindDir(FF_IOMAN *pIoman, const FF_T_WCHAR *path, FF_T_UINT16 pathLen, FF_ERROR *pError) {
#else
FF_T_UINT32 FF_FindDir(FF_IOMAN *pIoman, const FF_T_INT8 *path, FF_T_UINT16 pathLen, FF_ERROR *pError) {
#endif
    FF_T_UINT32     dirCluster = pIoman->pPartition->RootDirCluster;
#ifdef FF_UNICODE_SUPPORT
	FF_T_WCHAR		mytoken[FF_MAX_FILENAME];
	FF_T_WCHAR		*token;
#else
	FF_T_INT8		mytoken[FF_MAX_FILENAME];
	FF_T_INT8       *token;
#endif

    FF_T_UINT16     it = 0;         // Re-entrancy Variables for FF_strtok()
    FF_T_BOOL       last = FF_FALSE;
    FF_DIRENT       MyDir;
#ifdef FF_PATH_CACHE
	FF_T_UINT32		i;
#endif

	*pError = FF_ERR_NONE;

    if(pathLen <= 1) {      // Must be the root dir! (/ or \)
		return pIoman->pPartition->RootDirCluster;
    }

    if(path[pathLen-1] == '\\' || path[pathLen-1] == '/') {
		pathLen--;
    }

#ifdef FF_PATH_CACHE	// Is the requested path in the PATH CACHE?
	FF_PendSemaphore(pIoman->pSemaphore);	// Thread safety on shared object!
	{
		for(i = 0; i < FF_PATH_CACHE_DEPTH; i++) {
#ifdef FF_UNICODE_SUPPORT
			if(wcslen(pIoman->pPartition->PathCache[i].Path) == pathLen) {
				if(FF_strmatch(pIoman->pPartition->PathCache[i].Path, path, pathLen)) {
#else
			if(strlen(pIoman->pPartition->PathCache[i].Path) == pathLen) {
				if(FF_strmatch(pIoman->pPartition->PathCache[i].Path, path, pathLen)) {
#endif

					FF_ReleaseSemaphore(pIoman->pSemaphore);
					return pIoman->pPartition->PathCache[i].DirCluster;
				}
			}
		}
	}
	FF_ReleaseSemaphore(pIoman->pSemaphore);
#endif

    token = FF_strtok(path, mytoken, &it, &last, pathLen);

     do{
            MyDir.CurrentItem = 0;
            dirCluster = FF_FindEntryInDir(pIoman, dirCluster, token, FF_FAT_ATTR_DIR, &MyDir, pError);
			if(FF_isERR(*pError)) {
				return 0;
			}

			/*if(dirCluster == 0 && MyDir.CurrentItem == 2 && MyDir.FileName[0] == '.') { // .. Dir Entry pointing to root dir.
				dirCluster = pIoman->pPartition->RootDirCluster;
            }*/
            token = FF_strtok(path, mytoken, &it, &last, pathLen);
    }while(token != NULL);

#ifdef FF_PATH_CACHE	// Update the PATH CACHE with a new PATH
	 if(dirCluster) {	// Only cache if the dir was actually found!
		FF_PendSemaphore(pIoman->pSemaphore);
		{
			if(pathLen < FF_MAX_PATH) {	// Ensure the PATH won't cause a buffer overrun.
#ifdef FF_UNICODE_SUPPORT
				memcpy(pIoman->pPartition->PathCache[pIoman->pPartition->PCIndex].Path, path, pathLen * sizeof(FF_T_WCHAR));
#else
				memcpy(pIoman->pPartition->PathCache[pIoman->pPartition->PCIndex].Path, path, pathLen);
#endif
				pIoman->pPartition->PathCache[pIoman->pPartition->PCIndex].Path[pathLen] = '\0';
				pIoman->pPartition->PathCache[pIoman->pPartition->PCIndex].DirCluster = dirCluster;
#ifdef FF_HASH_TABLE_SUPPORT
				FF_ClearHashTable(pIoman->pPartition->PathCache[pIoman->pPartition->PCIndex].pHashTable);
#endif
				pIoman->pPartition->PCIndex += 1;
				if(pIoman->pPartition->PCIndex >= FF_PATH_CACHE_DEPTH) {
					pIoman->pPartition->PCIndex = 0;
				}
			}
		}
		FF_ReleaseSemaphore(pIoman->pSemaphore);
	 }
#endif

    return dirCluster;
}


#if defined(FF_SHORTNAME_CASE)
/**
 *	@private
 *  For short-name entries, NT/XP etc store case information in byte 0x0c
 *  Use this to show proper case of "README.txt" or "source.H"
 **/
#ifdef FF_UNICODE_SUPPORT
static void FF_CaseShortName(FF_T_WCHAR *name, FF_T_UINT8 attrib) {
#else
static void FF_CaseShortName(FF_T_INT8 *name, FF_T_UINT8 attrib) {
#endif
	FF_T_UINT8 testAttrib = FF_FAT_CASE_ATTR_BASE;
	for (; *name; name++) {
		if (*name == '.') {
			testAttrib = FF_FAT_CASE_ATTR_EXT;
		} else if ((attrib & testAttrib)) {
			if (*name >= 'A' && *name <= 'Z')
				*name += 0x20;
		} else if (*name >= 'a' && *name <= 'z') {
			*name -= 0x20;
		}
	}
}
#endif

/**
 *	@private
 **/

#ifdef FF_UNICODE_SUPPORT
static void FF_ProcessShortName(FF_T_WCHAR *name) {
	FF_T_WCHAR	shortName[13];
	FF_T_WCHAR	*ptr = name;
#else
static void FF_ProcessShortName(FF_T_INT8 *name) {
	FF_T_INT8	shortName[13];
	FF_T_INT8	*ptr = name;
#endif
	FF_T_UINT8	i;
#ifdef FF_UNICODE_SUPPORT
	memcpy(shortName, name, 11 * sizeof(FF_T_WCHAR));
#else
	memcpy(shortName, name, 11);
#endif

	for(i = 0; i < 11; i++) {
		if(shortName[i] == 0x20) {
			if (i >= 8)
				break;
			i = 7;
		} else {
			if (i == 8)
				*(ptr++) = '.';
			*(ptr++) = shortName[i];
		}
	}
	*ptr = '\0';
}

/*
#ifdef FF_UNICODE_SUPPORT
static void FF_ProcessShortName(FF_T_WCHAR *name) {
	FF_T_WCHAR	shortName[13];
#else
static void FF_ProcessShortName(FF_T_INT8 *name) {
	FF_T_INT8	shortName[13];
#endif

	FF_T_UINT8	i;
#ifdef FF_UNICODE_SUPPORT
	memcpy(shortName, name, 11 * sizeof(FF_T_WCHAR));
#else
	memcpy(shortName, name, 11);
#endif

	for(i = 0; i < 8; i++) {
		if(shortName[i] == 0x20) {
			name[i] = '\0';
			break;
		}
		name[i] = shortName[i];
	}

	if(shortName[8] != 0x20){
		name[i] = '.';
		name[i+1] = shortName[8];
		name[i+2] = shortName[9];
		name[i+3] = shortName[10];
		name[i+4] = '\0';
		for(i = 0; i < 11; i++) {
			if(name[i] == 0x20) {
				name[i] = '\0';
				break;
			}
		}
	} else {
		name[i] = '\0';
	}

}*/

#ifdef FF_TIME_SUPPORT
static void FF_PlaceTime(FF_T_UINT8 *EntryBuffer, FF_T_UINT32 Offset, FF_SYSTEMTIME *pTime) {
	FF_T_UINT16		myShort;

	// HT time changes:
	// E.g. Unzip needs to use original time rather than
	// the result of FF_GetSystemTime

	myShort = 0;
	myShort |= ((pTime->Hour    << 11) & 0xF800);
	myShort |= ((pTime->Minute  <<  5) & 0x07E0);
	myShort |= ((pTime->Second   /  2) & 0x001F);
	FF_putShort(EntryBuffer, (FF_T_UINT16) Offset, myShort);
}

static void FF_PlaceDate(FF_T_UINT8 *EntryBuffer, FF_T_UINT32 Offset, FF_SYSTEMTIME *pTime) {
	FF_T_UINT16		myShort;

	// HT time changes:
	// Unzip needs to use original date rather than
	// the current date, so make it a parameter
	myShort = 0;
	myShort |= (((pTime->Year- 1980)  <<  9) & 0xFE00) ;
	myShort |= ((pTime->Month <<  5) & 0x01E0);
	myShort |= (pTime->Day & 0x001F);
	FF_putShort(EntryBuffer, (FF_T_UINT16) Offset, myShort);
}


static void FF_GetTime(FF_SYSTEMTIME *pTime, FF_T_UINT8 *EntryBuffer, FF_T_UINT32 Offset) {
	FF_T_UINT16 myShort;
	myShort = FF_getShort(EntryBuffer, (FF_T_UINT16) Offset);
	pTime->Hour		= (((myShort & 0xF800) >> 11) & 0x001F);
	pTime->Minute	= (((myShort & 0x07E0) >>  5) & 0x003F);
	pTime->Second	= 2 * (myShort & 0x01F);
}

static void FF_GetDate(FF_SYSTEMTIME *pTime, FF_T_UINT8 *EntryBuffer, FF_T_UINT32 Offset) {
	FF_T_UINT16 myShort;
	myShort = FF_getShort(EntryBuffer, (FF_T_UINT16) Offset);
	pTime->Year		= 1980 + (((myShort & 0xFE00) >> 9) & 0x07F);
	pTime->Month	= (((myShort & 0x01E0) >> 5) & 0x000F);
	pTime->Day		= myShort & 0x01F;
}
#endif

void FF_PopulateShortDirent(FF_IOMAN *pIoman, FF_DIRENT *pDirent, FF_T_UINT8 *EntryBuffer) {
	//FF_T_UINT16 myShort;
#ifdef FF_UNICODE_SUPPORT
	FF_T_WCHAR UTF16EntryBuffer[12];
	FF_cstrntowcs(UTF16EntryBuffer, (FF_T_INT8 *) EntryBuffer, 11);
	memcpy(pDirent->FileName, UTF16EntryBuffer, 11 * sizeof(FF_T_WCHAR));
#else
	memcpy(pDirent->FileName, EntryBuffer, 11);	// Copy the filename into the Dirent object.
#endif
#if defined(FF_LFN_SUPPORT) && defined(FF_INCLUDE_SHORT_NAME)
	memcpy(pDirent->ShortName, EntryBuffer, 11);
	pDirent->ShortName[11] = '\0';
	FF_ProcessShortName(pDirent->ShortName);	// For debuggers only
#endif
	FF_ProcessShortName(pDirent->FileName);		// Format the shortname, for pleasant viewing.

#ifdef FF_HASH_TABLE_SUPPORT
/*#if FF_HASH_FUNCTION == CRC16
	FF_AddDirentHash(pIoman, pDirent->DirCluster, (FF_T_UINT32)FF_GetCRC16((FF_T_UINT8 *) pDirent->FileName, strlen(pDirent->FileName)));
#elif FF_HASH_FUNCTION == CRC8
	FF_AddDirentHash(pIoman, pDirent->DirCluster, (FF_T_UINT32)FF_GetCRC8((FF_T_UINT8 *) pDirent->FileName, strlen(pDirent->FileName)));
#endif*/
#else
	if (pIoman) {}	// Silence a compiler warning, about not referencing pIoman.
#endif

#ifdef FF_UNICODE_SUPPORT
	FF_tolower(pDirent->FileName, (FF_T_UINT32)wcslen(pDirent->FileName));
#else
	FF_tolower(pDirent->FileName, (FF_T_UINT32)strlen(pDirent->FileName));
#endif

	// Get the item's Cluster address.
	pDirent->ObjectCluster =
		((FF_T_UINT32)FF_getShort(EntryBuffer, FF_FAT_DIRENT_CLUS_HIGH) << 16) |
		 (FF_T_UINT32)FF_getShort(EntryBuffer, FF_FAT_DIRENT_CLUS_LOW);
#ifdef FF_TIME_SUPPORT
	// Get the creation Time & Date
	FF_GetTime(&pDirent->CreateTime, EntryBuffer, FF_FAT_DIRENT_CREATE_TIME);
	FF_GetDate(&pDirent->CreateTime, EntryBuffer, FF_FAT_DIRENT_CREATE_DATE);
	// Get the modified Time & Date
	// HT Here CreateTime became ModifiedTime:
	FF_GetTime(&pDirent->ModifiedTime, EntryBuffer, FF_FAT_DIRENT_LASTMOD_TIME);
	FF_GetDate(&pDirent->ModifiedTime, EntryBuffer, FF_FAT_DIRENT_LASTMOD_DATE);
	// Get the last accessed Date.
	FF_GetDate(&pDirent->AccessedTime, EntryBuffer, FF_FAT_DIRENT_LASTACC_DATE);
	pDirent->AccessedTime.Hour		= 0;
	pDirent->AccessedTime.Minute	= 0;
	pDirent->AccessedTime.Second	= 0;
#endif
	// Get the filesize.
	pDirent->Filesize = FF_getLong(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_FILESIZE));
	// Get the attribute.
	pDirent->Attrib = FF_getChar(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB));
}

/*
	Initialises a context object for FF_FetchEntryWithContext()
*/
FF_ERROR FF_InitEntryFetch(FF_IOMAN *pIoman, FF_T_UINT32 ulDirCluster, FF_FETCH_CONTEXT *pContext) {

	FF_ERROR Error;

	memset(pContext, 0, sizeof(FF_FETCH_CONTEXT));

	pContext->ulChainLength 		= FF_GetChainLength(pIoman, ulDirCluster, NULL, &Error);	// Get the total length of the chain.
	if(FF_isERR(Error)) {
		return Error;
	}
	pContext->ulDirCluster			= ulDirCluster;
	pContext->ulCurrentClusterLCN 	= ulDirCluster;
//	pContext->ulCurrentClusterNum 	= 0;  // memset has cleared it
//	pContext->ulCurrentEntry		= 0;

	if(pIoman->pPartition->Type != FF_T_FAT32) {
		// Handle Root Dirs that don't have cluster chains!
		if(pContext->ulDirCluster == pIoman->pPartition->RootDirCluster) {
			// This is a RootDIR, special consideration needs to be made, because it doesn't have a Cluster chain!
			pContext->ulChainLength = pIoman->pPartition->RootDirSectors / pIoman->pPartition->SectorsPerCluster;
			if(!pContext->ulChainLength) {		// Some media has RootDirSectors < SectorsPerCluster. This is wrong, as it should be atleast 1 cluster!
				pContext->ulChainLength = 1;
			}
		}
	}

	return FF_ERR_NONE;
}

FF_ERROR FF_CleanupEntryFetch(FF_IOMAN *pIoman, FF_FETCH_CONTEXT *pContext) {
	FF_ERROR Error = FF_ERR_NONE;
	if(pContext->pBuffer) {
		Error = FF_ReleaseBuffer(pIoman, pContext->pBuffer);
		pContext->pBuffer = NULL;
	}
	return Error;
}

/**
 *	@private
 *	@brief	Find the cluster for a given Entry within a directory
 *  @brief	Make an exception for the root directory (non FAT32 only):
 *  @brief	Just calculate the cluster (don't consult the actual FAT)
 *
 *	@param	pIoman      FF_IOMAN object that was created by FF_CreateIOMAN().
 *	@param	ulEntry     The sequence number of the entry of interest
 *  @param	pContext    Context of current search
 *
 *	@return	FF_ERR_NONE on success
 *	@return	Possible error returned by FF_TraverseFAT() or END_OF_DIR
 *
 *  Side effects:
 *    - pContext->ulCurrentClusterNum : relative cluster number (0 <= Num < ulChainLength)
 *    - pContext->ulCurrentClusterLCN : fysical cluster on the partition
 **/

static FF_ERROR FF_Traverse(FF_IOMAN *pIoman, FF_T_UINT32 ulEntry, FF_FETCH_CONTEXT *pContext)
{
	FF_T_UINT32	ulClusterNum = FF_getClusterChainNumber(pIoman, ulEntry, (FF_T_UINT16)32);
	FF_ERROR	Error;

	// Check if we're past the last cluster (ulChainLength is also valid for root sectors)
	if((ulClusterNum + 1) > pContext->ulChainLength) {
		return FF_ERR_DIR_END_OF_DIR | FF_TRAVERSE;	// End of Dir was reached!
	}

	if(pIoman->pPartition->Type != FF_T_FAT32 &&
		pContext->ulDirCluster == pIoman->pPartition->RootDirCluster) {
		// Double-check if the entry number isn't too high
		if(ulEntry > ((pIoman->pPartition->RootDirSectors * pIoman->pPartition->BlkSize) / 32)) {
			return FF_ERR_DIR_END_OF_DIR | FF_FETCHENTRYWITHCONTEXT;
		}
		pContext->ulCurrentClusterLCN = pContext->ulDirCluster;// + ulClusterNum;
	} else if(ulClusterNum != pContext->ulCurrentClusterNum) {
		// Traverse the fat gently!
		if(ulClusterNum > pContext->ulCurrentClusterNum) {
			// Start traverse from the current entry
			pContext->ulCurrentClusterLCN = FF_TraverseFAT(pIoman, pContext->ulCurrentClusterLCN, (ulClusterNum - pContext->ulCurrentClusterNum), &Error);
			if(FF_isERR(Error)) {
				return Error;
			}
		} else {
			// Start traverse from the beginning
			pContext->ulCurrentClusterLCN = FF_TraverseFAT(pIoman, pContext->ulDirCluster, ulClusterNum, &Error);
			if(FF_isERR(Error)) {
				return Error;
			}
		}
	}
	pContext->ulCurrentClusterNum = ulClusterNum;
	return FF_ERR_NONE;
}

FF_ERROR FF_FetchEntryWithContext(FF_IOMAN *pIoman, FF_T_UINT32 ulEntry, FF_FETCH_CONTEXT *pContext, FF_T_UINT8 *pEntryBuffer) {

	FF_T_UINT32	ulItemLBA;
	FF_T_UINT32	ulRelItem;
	//FF_T_UINT32	ulClusterNum;
	FF_ERROR	Error;

	Error = FF_Traverse(pIoman, ulEntry, pContext);
	if(FF_isERR(Error)) {
		return Error;
	}

	//ulClusterNum  = pContext->ulCurrentClusterNum;
	ulRelItem     = FF_getMinorBlockEntry (pIoman, ulEntry, (FF_T_UINT16)32);

	ulItemLBA = FF_Cluster2LBA (pIoman, pContext->ulCurrentClusterLCN) + FF_getMajorBlockNumber(pIoman, ulEntry, (FF_T_UINT16)32);
	if(pIoman->pPartition->Type != FF_T_FAT32 &&
		pContext->ulDirCluster == pIoman->pPartition->RootDirCluster) {
			ulItemLBA += (ulEntry / ((pIoman->pPartition->BlkSize *pIoman->pPartition->SectorsPerCluster)/32) * pIoman->pPartition->SectorsPerCluster);
	}

	ulItemLBA = FF_getRealLBA (pIoman, ulItemLBA)	+ FF_getMinorBlockNumber(pIoman, ulRelItem, (FF_T_UINT16)32);

	if(!pContext->pBuffer ||
		(pContext->pBuffer->Sector != ulItemLBA) ||
		(pContext->pBuffer->Mode & FF_MODE_WRITE)) {
		if(pContext->pBuffer) {
			Error = FF_ReleaseBuffer(pIoman, pContext->pBuffer);
			if(FF_isERR(Error)) {
				return Error;
			}
		}
		pContext->pBuffer = FF_GetBuffer(pIoman, ulItemLBA, FF_MODE_READ);
		if(!pContext->pBuffer) {
			return FF_ERR_DEVICE_DRIVER_FAILED | FF_FETCHENTRYWITHCONTEXT;
		}
	}

	if (pEntryBuffer) {	// HT Because it might be called with NULL
		memcpy(pEntryBuffer, (pContext->pBuffer->pBuffer + (ulRelItem*32)), 32);
	}

    return FF_ERR_NONE;
}


FF_ERROR FF_PushEntryWithContext(FF_IOMAN *pIoman, FF_T_UINT32 ulEntry, FF_FETCH_CONTEXT *pContext, FF_T_UINT8 *pEntryBuffer) {
	FF_T_UINT32	ulItemLBA;
	FF_T_UINT32	ulRelItem;
	//FF_T_UINT32	ulClusterNum;
	FF_ERROR	Error;

	Error = FF_Traverse(pIoman, ulEntry, pContext);
	if(FF_isERR(Error)) {
		return Error;
	}

	//ulClusterNum  = pContext->ulCurrentClusterNum;
	ulRelItem     = FF_getMinorBlockEntry (pIoman, ulEntry, (FF_T_UINT16)32);

	ulItemLBA = FF_Cluster2LBA (pIoman, pContext->ulCurrentClusterLCN) + FF_getMajorBlockNumber(pIoman, ulEntry, (FF_T_UINT16)32);
	if(pIoman->pPartition->Type != FF_T_FAT32 &&
		pContext->ulDirCluster == pIoman->pPartition->RootDirCluster) {
			ulItemLBA += (ulEntry / ((pIoman->pPartition->BlkSize *pIoman->pPartition->SectorsPerCluster)/32) * pIoman->pPartition->SectorsPerCluster);
	}

	ulItemLBA = FF_getRealLBA (pIoman, ulItemLBA)	+ FF_getMinorBlockNumber(pIoman, ulRelItem, (FF_T_UINT16)32);

	if(!pContext->pBuffer ||
		(pContext->pBuffer->Sector != ulItemLBA) ||
		!(pContext->pBuffer->Mode & FF_MODE_WRITE)) {
		if(pContext->pBuffer) {
			Error = FF_ReleaseBuffer(pIoman, pContext->pBuffer);
			if(FF_isERR(Error)) {
				return Error;
			}
		}
		pContext->pBuffer = FF_GetBuffer(pIoman, ulItemLBA, FF_MODE_WRITE);
		if(!pContext->pBuffer) {
			return FF_ERR_DEVICE_DRIVER_FAILED | FF_FETCHENTRYWITHCONTEXT;
		}
	}

	// Now change the entry:
	memcpy((pContext->pBuffer->pBuffer + (ulRelItem*32)), pEntryBuffer, 32);
//	HT: this would be against the rules :-)
//	pContext->pBuffer->Mode = FF_MODE_WRITE;
//	pContext->pBuffer->Modified = FF_TRUE;

    return FF_ERR_NONE;
}


/**
 *	@private
 **/
FF_ERROR FF_GetEntry(FF_IOMAN *pIoman, FF_T_UINT16 nEntry, FF_T_UINT32 DirCluster, FF_DIRENT *pDirent) {
	FF_T_UINT8
	EntryBuffer[32];
	FF_FETCH_CONTEXT	FetchContext;
	FF_ERROR				Error;

#ifndef FF_LFN_SUPPORT
	FF_T_UINT8			numLFNs;
#endif
	Error = FF_InitEntryFetch(pIoman, DirCluster, &FetchContext);
	if(FF_isERR(Error)) {
		return Error;
	}

	Error = FF_FetchEntryWithContext(pIoman, nEntry, &FetchContext, EntryBuffer);
	if(FF_isERR(Error)) {
		FF_CleanupEntryFetch(pIoman, &FetchContext);	// Error already, skip error checking.
		return Error;
	}
	if(EntryBuffer[0] != 0xE5) {
		if(FF_isEndOfDir(EntryBuffer)){
			Error = FF_CleanupEntryFetch(pIoman, &FetchContext);
			if(FF_isERR(Error)) {
				return Error;
			}
			return FF_ERR_DIR_END_OF_DIR | FF_GETENTRY;
		}

		pDirent->Attrib = FF_getChar(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB));

		if((pDirent->Attrib & FF_FAT_ATTR_LFN) == FF_FAT_ATTR_LFN) {
	#ifdef FF_LFN_SUPPORT
			Error = FF_PopulateLongDirent(pIoman, pDirent, nEntry, &FetchContext);
			if(FF_isERR(Error)) {
				FF_CleanupEntryFetch(pIoman, &FetchContext);	// Returning error dont check here.
				return Error;
			}

			Error = FF_CleanupEntryFetch(pIoman, &FetchContext);
			if(FF_isERR(Error)) {
				return Error;
			}

			return FF_ERR_NONE;
	#else
			// LFN Processing
			numLFNs = (FF_T_UINT8)(EntryBuffer[0] & ~0x40);
			pDirent->CurrentItem += (numLFNs - 1);
	#endif
		} else if((pDirent->Attrib & FF_FAT_ATTR_VOLID) == FF_FAT_ATTR_VOLID) {
			// Do Nothing

		} else {
			FF_PopulateShortDirent(pIoman, pDirent, EntryBuffer);
			pDirent->CurrentItem += 1;
			Error = FF_CleanupEntryFetch(pIoman, &FetchContext);
			if(FF_isERR(Error)) {
				return Error;
			}
			return 0;
		}
	}

	return FF_CleanupEntryFetch(pIoman, &FetchContext);
}

FF_T_BOOL FF_isEndOfDir(FF_T_UINT8 *EntryBuffer) {
	return !(EntryBuffer[0]);
}

#ifdef FF_HASH_CACHE
FF_ERROR FF_AddDirentHash(FF_IOMAN *pIoman, FF_T_UINT32 ulDirCluster, FF_T_UINT32 ulHash) {
	FF_T_UINT32 i;
	FF_HASH_TABLE pHash = NULL;
	for(i = 0; i < FF_HASH_CACHE_DEPTH; i++) {
		if(pIoman->HashCache[i].ulDirCluster == ulDirCluster) {
			pHash = pIoman->HashCache[i].pHashTable;
			break;
		}
	}

	if(pHash) {
		FF_SetHash(pHash, ulHash);
	}

	return FF_ERR_NONE;
}

FF_T_BOOL FF_CheckDirentHash(FF_IOMAN *pIoman, FF_T_UINT32 ulDirCluster, FF_T_UINT32 ulHash) {
	FF_T_UINT32 i;
	FF_HASH_TABLE pHash = NULL;
	for(i = 0; i < FF_HASH_CACHE_DEPTH; i++) {
		if(pIoman->HashCache[i].ulDirCluster == ulDirCluster) {
			pHash = pIoman->HashCache[i].pHashTable;
			break;
		}
	}

	if(pHash) {
		return FF_isHashSet(pHash, ulHash);
	}

	return FF_FALSE;
}

FF_T_BOOL FF_DirHashed(FF_IOMAN *pIoman, FF_T_UINT32 ulDirCluster) {
	FF_T_UINT32 i;
	for(i = 0; i < FF_HASH_CACHE_DEPTH; i++) {
		if(pIoman->HashCache[i].ulDirCluster == ulDirCluster) {
			return FF_TRUE;
		}
	}

	return FF_FALSE;
}

FF_ERROR FF_HashDir(FF_IOMAN *pIoman, FF_T_UINT32 ulDirCluster) {
	// Find most suitable Hash Table to replace!
	FF_T_UINT32 		i;
	FF_HASHCACHE 		*pHashCache = NULL;
	FF_FETCH_CONTEXT	FetchContext;
	FF_T_UINT8			EntryBuffer[32], ucAttrib;
	FF_T_UINT32			ulHash;
	FF_ERROR			Error;

	if(FF_DirHashed(pIoman, ulDirCluster)) {
		return FF_ERR_NONE;			// Don't wastefully re-hash a dir!
	}

	//printf("----- Hashing Directory\n");

	for(i = 0; i < FF_HASH_CACHE_DEPTH; i++) {
		if(!pIoman->HashCache[i].ulNumHandles) {
			if(!pHashCache) {
				pHashCache = &pIoman->HashCache[i];
			} else {
				if((pIoman->HashCache[i].ulMisses > pHashCache->ulMisses)) {
					pHashCache = &pIoman->HashCache[i];
				}
			}
		}
	}

	if(pHashCache) {
		// Clear the hash table!
		FF_ClearHashTable(pHashCache->pHashTable);
		pHashCache->ulDirCluster = ulDirCluster;
		pHashCache->ulMisses = 0;

		// Hash the directory!

		FF_InitEntryFetch(pIoman, ulDirCluster, &FetchContext);

		for(i = 0; i < 0xFFFF; i++) {
			if(FF_FetchEntryWithContext(pIoman, i, &FetchContext, EntryBuffer)) {
				break;	// HT addition
			}
			ucAttrib = FF_getChar(EntryBuffer, FF_FAT_DIRENT_ATTRIB);
			if(FF_getChar(EntryBuffer, 0x00) != 0xE5) {
				if(ucAttrib != FF_FAT_ATTR_LFN) {
					FF_ProcessShortName((FF_T_INT8 *)EntryBuffer);
					if(FF_isEndOfDir(EntryBuffer)) {
						// HT uncommented
						Error = FF_CleanupEntryFetch(pIoman, &FetchContext);
						if(FF_isERR(Error)) {
							return Error;
						}
						return FF_ERR_NONE;
					}

					// Generate the Hash
#if FF_HASH_FUNCTION == CRC16
					ulHash = FF_GetCRC16(EntryBuffer, strlen((const FF_T_INT8 *) EntryBuffer));
#elif FF_HASH_FUNCTION == CRC8
					ulHash = FF_GetCRC8(EntryBuffer, strlen((const FF_T_INT8 *) EntryBuffer));
#endif
					FF_SetHash(pHashCache->pHashTable, ulHash);

				}
			}
		}

		Error = FF_CleanupEntryFetch(pIoman, &FetchContext);
		if(FF_isERR(Error)) {
			return Error;
		}

		return FF_ERR_NONE;
	}

	return -1;
}

/*void FF_SetDirHashed(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster) {
	int i;
	for(i = 0; i < FF_PATH_CACHE_DEPTH; i++) {
		if(pIoman->pPartition->PathCache[i].DirCluster == DirCluster) {
			pIoman->pPartition->PathCache[i].bHashed = FF_TRUE;
			return;
		}
	}
}*/
#endif

FF_ERROR FF_PopulateLongDirent(FF_IOMAN *pIoman, FF_DIRENT *pDirent, FF_T_UINT16 nEntry, FF_FETCH_CONTEXT *pFetchContext) {
	// First get the entire name as UTF-16 from the LFN's.
	// Then transform into the API's native string format.

	FF_ERROR	Error;
	FF_T_UINT	uiNumLFNs;
#ifdef FF_UNICODE_SUPPORT
	FF_T_UINT	uiLfnLength = 0;
#endif
#ifndef FF_UNICODE_SUPPORT
	FF_T_UINT	i,y;
#endif

#ifdef FF_UNICODE_SUPPORT
//	FF_T_WCHAR	*lastPtr = pDirent->FileName + sizeof(pDirent->FileName);
//	FF_T_WCHAR	*ptr;
#else
#ifndef FF_UNICODE_UTF8_SUPPORT
	FF_T_INT8	*lastPtr = pDirent->FileName + sizeof (pDirent->FileName);
	FF_T_INT8	*ptr;
#endif
#endif
#ifdef FF_UNICODE_UTF8_SUPPORT
//	FF_T_SINT32	slRetVal;
	FF_T_UINT16 nLfnBegin;
	FF_T_UINT16	usUtf8Len = 0;
#endif
	FF_T_UINT8	ucCheckSum;

	FF_T_UINT8	EntryBuffer[32];
	//FF_T_UINT16	UTF16Name[FF_MAX_FILENAME];

#ifdef FF_UNICODE_SUPPORT
	FF_T_WCHAR	WCEntryBuffer[32];
	FF_T_WCHAR	ShortName[13];
#else
	FF_T_INT8	ShortName[13];
#endif

	Error = FF_FetchEntryWithContext(pIoman, nEntry++, pFetchContext, EntryBuffer);
	if(FF_isERR(Error)) {
		return Error;
	}

	uiNumLFNs = (FF_T_UINT)(EntryBuffer[0] & ~0x40);
	ucCheckSum = FF_getChar(EntryBuffer, FF_FAT_LFN_CHECKSUM);

#ifdef FF_UNICODE_SUPPORT	// UTF-16 Can simply get segments of the UTF-16 sequence going forward
							// in the dirents. (I.e. reversed order).

	while(uiNumLFNs) {	// Avoid stack intensive use of a UTF-16 buffer. Stream direct to FileName dirent field in correct format.

		// memcopy direct!-UTF-16 support
		memcpy(pDirent->FileName + ((uiNumLFNs - 1) * 13) + 0,  &EntryBuffer[FF_FAT_LFN_NAME_1], 10);
		memcpy(pDirent->FileName + ((uiNumLFNs - 1) * 13) + 5,  &EntryBuffer[FF_FAT_LFN_NAME_2], 12);
		memcpy(pDirent->FileName + ((uiNumLFNs - 1) * 13) + 11, &EntryBuffer[FF_FAT_LFN_NAME_3], 4);


		uiLfnLength += 13;

		Error = FF_FetchEntryWithContext(pIoman, nEntry++, pFetchContext, EntryBuffer);
		if(FF_isERR(Error)) {
			return Error;
		}
		uiNumLFNs--;
	}

	pDirent->FileName[uiLfnLength] = '\0';
#endif

#ifdef FF_UNICODE_UTF8_SUPPORT
	// UTF-8 Sequence, we can only convert this from the beginning, must receive entries in reverse.
	nLfnBegin = nEntry - 1;

	for(i = 0; i < uiNumLFNs; i++) {
		Error = FF_FetchEntryWithContext(pIoman, (nLfnBegin + (uiNumLFNs - 1) - i), pFetchContext, EntryBuffer);
		if(FF_isERR(Error)) {
			return Error;
		}

		// Now have the first part of the UTF-16 sequence. Stream into a UTF-8 sequence.
		for(y = 0; y < 5; y++) {
			Error = FF_Utf16ctoUtf8c((FF_T_UINT8 *) &pDirent->FileName[usUtf8Len], (FF_T_UINT16 *) &EntryBuffer[FF_FAT_LFN_NAME_1 + (y*2)], sizeof(pDirent->FileName) - usUtf8Len);
			if(Error > 0) {
				usUtf8Len += (FF_T_UINT16) Error;
			}
		}

		for(y = 0; y < 6; y++) {
			Error = FF_Utf16ctoUtf8c((FF_T_UINT8 *) &pDirent->FileName[usUtf8Len], (FF_T_UINT16 *) &EntryBuffer[FF_FAT_LFN_NAME_2 + (y*2)], sizeof(pDirent->FileName) - usUtf8Len);
			if(Error > 0) {
				usUtf8Len += (FF_T_UINT16) Error;
			}
		}

		for(y = 0; y < 2; y++) {
			Error = FF_Utf16ctoUtf8c((FF_T_UINT8 *) &pDirent->FileName[usUtf8Len], (FF_T_UINT16 *) &EntryBuffer[FF_FAT_LFN_NAME_3 + (y*2)], sizeof(pDirent->FileName) - usUtf8Len);
			if(Error > 0) {
				usUtf8Len += (FF_T_UINT16) Error;
			}
		}
		nEntry++;
	}

	pDirent->FileName[usUtf8Len] = '\0';

	// Put Entry context to correct position.
	Error = FF_FetchEntryWithContext(pIoman, nEntry-1, pFetchContext, EntryBuffer);
	if(FF_isERR(Error)) {
		return Error;
	}

#endif

#if !defined(FF_UNICODE_SUPPORT) && !defined(FF_UNICODE_UTF8_SUPPORT)	// No Unicode, simple ASCII.
	lastPtr[-1] = '\0';
	y = uiNumLFNs;
	while(uiNumLFNs--) {
		ptr = pDirent->FileName + (uiNumLFNs * 13);
		for(i = 0; i < 10 && ptr < lastPtr; i += 2) {
			*(ptr++) = EntryBuffer[FF_FAT_LFN_NAME_1 + i];
		}

		for(i = 0; i < 12 && ptr < lastPtr; i += 2) {
			*(ptr++) = EntryBuffer[FF_FAT_LFN_NAME_2 + i];
		}

		for(i = 0; i < 4 && ptr < lastPtr; i += 2) {
			*(ptr++) = EntryBuffer[FF_FAT_LFN_NAME_3 + i];
		}
		if (uiNumLFNs == y-1 && ptr < lastPtr)
			*ptr = '\0';

		Error = FF_FetchEntryWithContext(pIoman, nEntry++, pFetchContext, EntryBuffer);
		if(FF_isERR(Error)) {
			return Error;
		}
	}
#endif
	// Process the Shortname. -- LFN Transformation is now complete.
	// Process the ShortName Entry

	// if SHORTNAMES must be included, simple byte copy into shortname buffer.
#if defined(FF_LFN_SUPPORT) && defined(FF_INCLUDE_SHORT_NAME)
	memcpy(pDirent->ShortName, EntryBuffer, 11);
	pDirent->ShortName[11] = '\0';
	FF_ProcessShortName(pDirent->ShortName);
#endif

#ifdef FF_UNICODE_SUPPORT
	FF_cstrntowcs(WCEntryBuffer, (FF_T_INT8 *) EntryBuffer, 32);
	memcpy(ShortName, WCEntryBuffer, 11 * sizeof(FF_T_WCHAR));
#else
	memcpy(ShortName, EntryBuffer, 11);
#endif
	FF_ProcessShortName(ShortName);
	if(ucCheckSum != FF_CreateChkSum(EntryBuffer)) {
#ifdef FF_UNICODE_SUPPORT
		wcscpy(pDirent->FileName, ShortName);
#else
		strcpy(pDirent->FileName, ShortName);
#endif
	}

	// Finally fill in the other details
	pDirent->ObjectCluster =
		((FF_T_UINT32)FF_getShort(EntryBuffer, FF_FAT_DIRENT_CLUS_HIGH) << 16) |
		 (FF_T_UINT32)FF_getShort(EntryBuffer, FF_FAT_DIRENT_CLUS_LOW);

#ifdef FF_TIME_SUPPORT
	// Get the creation Time & Date
	FF_GetTime(&pDirent->CreateTime, EntryBuffer, FF_FAT_DIRENT_CREATE_TIME);
	FF_GetDate(&pDirent->CreateTime, EntryBuffer, FF_FAT_DIRENT_CREATE_DATE);
	// Get the modified Time & Date
	// HT Here CreateTime has become ModifiedTime, as it should:
	FF_GetTime(&pDirent->ModifiedTime, EntryBuffer, FF_FAT_DIRENT_LASTMOD_TIME);
	FF_GetDate(&pDirent->ModifiedTime, EntryBuffer, FF_FAT_DIRENT_LASTMOD_DATE);
	// Get the last accessed Date.
	FF_GetDate(&pDirent->AccessedTime, EntryBuffer, FF_FAT_DIRENT_LASTACC_DATE);
	// HT Why should these times be zero'd ?
	pDirent->AccessedTime.Hour		= 0;
	pDirent->AccessedTime.Minute	= 0;
	pDirent->AccessedTime.Second	= 0;
#endif

	// Get the filesize.
	pDirent->Filesize = FF_getLong(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_FILESIZE));
	// Get the attribute.
	pDirent->Attrib = FF_getChar(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB));

	pDirent->CurrentItem = nEntry;
	//return x;
	return FF_ERR_NONE;
}

/*
FF_ERROR FF_PopulateLongDirent(FF_IOMAN *pIoman, FF_DIRENT *pDirent, FF_T_UINT16 nEntry, FF_FETCH_CONTEXT *pFetchContext) {
	FF_T_UINT8	EntryBuffer[32];
#ifdef FF_UNICODE_SUPPORT
	FF_T_WCHAR	UTF16EntryBuffer[32];
	FF_T_WCHAR	ShortName[13];
#if WCHAR_MAX > 0xFFFF
	FF_T_UINT16 i,y;
#endif
#else
	FF_T_INT8	ShortName[13];
#ifdef FF_UNICODE_UTF8_SUPPORT
	FF_T_SINT32 i, y;
#else
	FF_T_UINT16 i, y;
#endif
#endif

#ifdef FF_UNICODE_UTF8_SUPPORT
	FF_T_UINT16	UTF16Name[FF_MAX_FILENAME];	// Read in the entire UTF-16 name into this buffer.
	FF_T_UINT16 *UTF16cptr;
#endif
	FF_T_UINT8 numLFNs;
	FF_T_UINT8 x;
	FF_T_UINT8 CheckSum = 0;

	FF_T_UINT16 lenlfn = 0;
	FF_T_UINT16 myShort;
	FF_ERROR	Error;

	Error = FF_FetchEntryWithContext(pIoman, nEntry++, pFetchContext, EntryBuffer);
	if(FF_isERR(Error)) {
		return Error;
	}

	numLFNs = (FF_T_UINT8)(EntryBuffer[0] & ~0x40);
	// Handle the name
	CheckSum = FF_getChar(EntryBuffer, FF_FAT_LFN_CHECKSUM);

	x = numLFNs;
	while(numLFNs) {
		if(numLFNs > 1) {
			numLFNs = numLFNs;
		}

#ifdef FF_UNICODE_SUPPORT
		// Simply fill the FileName buffer with UTF-16 Filename!
#if WCHAR_MAX <= 0xFFFF	// System works in UTF-16 so we can trust it if we just copy the UTF-16 strings directly.
		memcpy(pDirent->FileName + ((numLFNs - 1) * 13) + 0,	&EntryBuffer[FF_FAT_LFN_NAME_1], (5 * 2));
		memcpy(pDirent->FileName + ((numLFNs - 1) * 13) + 5,	&EntryBuffer[FF_FAT_LFN_NAME_2], (6 * 2));
		memcpy(pDirent->FileName + ((numLFNs - 1) * 13) + 11,	&EntryBuffer[FF_FAT_LFN_NAME_3], (2 * 2));
		lenlfn += 13;
#else
		for(i = 0, y = 0; i < 5; i++, y += 2) {
			FF_Utf16ctoUtf32c((FF_T_UINT32 *)&pDirent->FileName[i + ((numLFNs - 1) * 13)], (FF_T_UINT16 *) &EntryBuffer[FF_FAT_LFN_NAME_1 + y]);
			//pDirent->FileName[i + ((numLFNs - 1) * 13)] = (FF_T_WCHAR) ((FF_T_WCHAR) EntryBuffer[FF_FAT_LFN_NAME_1 + y] | ((FF_T_WCHAR) EntryBuffer[FF_FAT_LFN_NAME_1 + y + 1] >> 8));
			lenlfn++;
		}
		for(i = 0, y = 0; i < 6; i++, y += 2) {
			FF_Utf16ctoUtf32c((FF_T_UINT32 *)&pDirent->FileName[i + ((numLFNs - 1) * 13) + 5], (FF_T_UINT16 *) &EntryBuffer[FF_FAT_LFN_NAME_2 + y]);
			//pDirent->FileName[i + ((numLFNs - 1) * 13) + 5] = (FF_T_WCHAR) ((FF_T_WCHAR) EntryBuffer[FF_FAT_LFN_NAME_2 + y] | ((FF_T_WCHAR) EntryBuffer[FF_FAT_LFN_NAME_2 + y + 1] >> 8));
			lenlfn++;
		}
		for(i = 0, y = 0; i < 2; i++, y += 2) {
			FF_Utf16ctoUtf32c((FF_T_UINT32 *)&pDirent->FileName[i + ((numLFNs - 1) * 13) + 11], (FF_T_UINT16 *) &EntryBuffer[FF_FAT_LFN_NAME_3 + y]);
			//pDirent->FileName[i + ((numLFNs - 1) * 13) + 11] = (FF_T_WCHAR) ((FF_T_WCHAR) EntryBuffer[FF_FAT_LFN_NAME_3 + y] | ((FF_T_WCHAR)EntryBuffer[FF_FAT_LFN_NAME_3 + y + 1] >> 8));
			lenlfn++;
		}
#endif
		// Copy each part of the LFNS
#else
#ifdef FF_UNICODE_UTF8_SUPPORT
		memcpy(UTF16Name + ((numLFNs - 1) * 13) + 0,	&EntryBuffer[FF_FAT_LFN_NAME_1], (5 * 2));
		memcpy(UTF16Name + ((numLFNs - 1) * 13) + 5,	&EntryBuffer[FF_FAT_LFN_NAME_2], (6 * 2));
		memcpy(UTF16Name + ((numLFNs - 1) * 13) + 11,	&EntryBuffer[FF_FAT_LFN_NAME_3], (2 * 2));
		lenlfn += 13;
#else
		// Attempts to pull ASCII from UTF-8 encoding.
		for(i = 0, y = 0; i < 5; i++, y += 2) {
			pDirent->FileName[i + ((numLFNs - 1) * 13)] = EntryBuffer[FF_FAT_LFN_NAME_1 + y];
			lenlfn++;
		}

		for(i = 0, y = 0; i < 6; i++, y += 2) {
			pDirent->FileName[i + ((numLFNs - 1) * 13) + 5] = EntryBuffer[FF_FAT_LFN_NAME_2 + y];
			lenlfn++;
		}

		for(i = 0, y = 0; i < 2; i++, y += 2) {
			pDirent->FileName[i + ((numLFNs - 1) * 13) + 11] = EntryBuffer[FF_FAT_LFN_NAME_3 + y];
			lenlfn++;
		}
#endif
#endif

		Error = FF_FetchEntryWithContext(pIoman, nEntry++, pFetchContext, EntryBuffer);
		if(FF_isERR(Error)) {
			return Error;
		}
		numLFNs--;
	}

#ifdef FF_UNICODE_UTF8_SUPPORT
	UTF16cptr = UTF16Name;
	UTF16Name[lenlfn] = '\0';
	i = 0;	// Keep tabs of the current char position in the UTF-8 sequence.
	while(*UTF16cptr) {
		y = FF_Utf16ctoUtf8c((FF_T_UINT8 *)&pDirent->FileName[i], UTF16cptr, (FF_MAX_FILENAME - i));
		i += y;
		if(FF_GetUtf16SequenceLen(*UTF16cptr++) == 2) {	// IF this is a surrogate, then bump the UTF16 Pointer.
			UTF16cptr++;
		}
	}
	pDirent->FileName[i] = '\0';
#else
	pDirent->FileName[lenlfn] = '\0';
#endif

	// Process the ShortName Entry
#ifdef FF_UNICODE_SUPPORT
	FF_cstrntowcs(UTF16EntryBuffer, (FF_T_INT8 *) EntryBuffer, 32);
	memcpy(ShortName, UTF16EntryBuffer, 11 * sizeof(FF_T_WCHAR));
#else
	memcpy(ShortName, EntryBuffer, 11);
#endif
	if(CheckSum != FF_CreateChkSum(EntryBuffer)) {
		FF_ProcessShortName(ShortName);
#ifdef FF_UNICODE_SUPPORT
		wcscpy(pDirent->FileName, ShortName);
#else
		strcpy(pDirent->FileName, ShortName);
#endif
	} else {
		FF_ProcessShortName(ShortName);
	}

#ifdef FF_HASH_TABLE_SUPPORT*/
/*#if FF_HASH_FUNCTION == CRC16
	FF_AddDirentHash(pIoman, pFetchContext->ulDirCluster, (FF_T_UINT32)FF_GetCRC16((FF_T_UINT8 *) ShortName, strlen(ShortName)));
#elif FF_HASH_FUNCTION == CRC8
	FF_AddDirentHash(pIoman, DirCluster, (FF_T_UINT32)FF_GetCRC8((FF_T_UINT8 *) ShortName, strlen(ShortName)));
#endif*//*
#endif

	myShort = FF_getShort(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_HIGH));
	pDirent->ObjectCluster = (FF_T_UINT32) (myShort << 16);
	myShort = FF_getShort(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_LOW));
	pDirent->ObjectCluster |= myShort;

#ifdef FF_TIME_SUPPORT
	// Get the creation Time & Date
	FF_GetTime(&pDirent->CreateTime, EntryBuffer, FF_FAT_DIRENT_CREATE_TIME);
	FF_GetDate(&pDirent->CreateTime, EntryBuffer, FF_FAT_DIRENT_CREATE_DATE);
	// Get the modified Time & Date
	FF_GetTime(&pDirent->CreateTime, EntryBuffer, FF_FAT_DIRENT_LASTMOD_TIME);
	FF_GetDate(&pDirent->CreateTime, EntryBuffer, FF_FAT_DIRENT_LASTMOD_DATE);
	// Get the last accessed Date.
	FF_GetDate(&pDirent->AccessedTime, EntryBuffer, FF_FAT_DIRENT_LASTACC_DATE);
	pDirent->AccessedTime.Hour		= 0;
	pDirent->AccessedTime.Minute	= 0;
	pDirent->AccessedTime.Second	= 0;
#endif

	// Get the filesize.
	pDirent->Filesize = FF_getLong(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_FILESIZE));
	// Get the attribute.
	pDirent->Attrib = FF_getChar(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB));

	pDirent->CurrentItem = nEntry;
	//return x;
	return FF_ERR_NONE;
}
*/
/**
 *	@public
 *	@brief	Find's the first directory entry for the provided path.
 *
 *	All values recorded in pDirent must be preserved to and between calls to
 *	FF_FindNext().
 *
 *	If FF_FINDAPI_ALLOW_WILDCARDS is defined, then path will have the following behaviour:
 *
 *	path = "\" 					- Open the root dir, and iterate through all items.
 *	path = "\*.c"				- Open the root dir, showing only files matching *.c wildcard.
 *	path = "\sub1\newdir"		- Get the DIRENT for the newdir directory in /sub1/ if one exists.
 *	path = "\sub1\newdir\"		- Open the directory /sub1/newdir/ and iterate through all items.
 *	path = "\sub1\newdir\*.c"	- Open the directory /sub1/newdir/ and iterate through all items matching the *.c wildcard.
 *
 *	It is important to distinguish the differences in behaviour between opening a Find operation
 *	on a path like /sub1 and /sub1/. (/sub1 gets the sub1 dirent from the / dir, whereas /sub/ opens the sub1 dir).
 *
 *	Note, as compatible with other similar APIs, FullFAT also accepts \sub1\* for the same behaviour as
 *	/sub1/.
 *
 *	For more up-to-date information please see the FullFAT wiki pages.
 *
 *	@param	pIoman		FF_IOMAN object that was created by FF_CreateIOMAN().
 *	@param	pDirent		FF_DIRENT object to store the entry information.
 *	@param	path		String to of the path to the Dir being listed.
 *
 *	@return	0 on success
 *	@return	FF_ERR_DEVICE_DRIVER_FAILED if device access failed.
 *	@return -2 if Dir was not found.
 *
 **/
#ifdef FF_UNICODE_SUPPORT
FF_ERROR FF_FindFirst(FF_IOMAN *pIoman, FF_DIRENT *pDirent, const FF_T_WCHAR *path) {
#else
FF_ERROR FF_FindFirst(FF_IOMAN *pIoman, FF_DIRENT *pDirent, const FF_T_INT8 *path) {
#endif
#ifdef FF_UNICODE_SUPPORT
	FF_T_UINT16	PathLen = (FF_T_UINT16) wcslen(path);
#else
	FF_T_UINT16	PathLen = (FF_T_UINT16) strlen(path);
#endif
	FF_ERROR	Error;

#ifdef FF_FINDAPI_ALLOW_WILDCARDS
	FF_T_UINT16 i = 0;
#ifdef FF_UNICODE_SUPPORT
	const FF_T_WCHAR *szWildCard;	// Check for a Wild-card.
#else
	const FF_T_INT8	*szWildCard;	// Check for a Wild-card.
#endif
#endif

	if(!pIoman) {
		return FF_ERR_NULL_POINTER | FF_FINDFIRST;
	}

	memset(pDirent, 0, sizeof(FF_DIRENT));

	// Detect a Wild-Card on the End, or Filename, as apposed to a complete path.
#ifndef FF_FINDAPI_ALLOW_WILDCARDS
	pDirent->DirCluster = FF_FindDir(pIoman, path, PathLen, &Error);	// Get the directory cluster, if it exists.
	if(FF_isERR(Error)) {
		return Error;
	}
#endif

#ifdef FF_FINDAPI_ALLOW_WILDCARDS
	pDirent->szWildCard[0] = '\0';	// WildCard blank if its not a wildCard.

	szWildCard = &path[PathLen - 1];

	if(PathLen) {
		while(*szWildCard != '\\' && *szWildCard != '/') {	// Open the dir of the last token.
			i++;
			szWildCard--;
			if(!(PathLen - i)) {
				break;
			}
		}
	}

	pDirent->DirCluster = FF_FindDir(pIoman, path, PathLen - i, &Error);
	if(FF_isERR(Error)) {
		return Error;
	}
	if(pDirent->DirCluster) {
		// Valid Dir found, copy the wildCard to filename!
#ifdef FF_UNICODE_SUPPORT
		wcsncpy(pDirent->szWildCard, ++szWildCard, FF_MAX_FILENAME);
#else
		strncpy(pDirent->szWildCard, ++szWildCard, FF_MAX_FILENAME);
#endif
		if(pDirent->szWildCard[i-1] == ':') {
			pDirent->bInvertWildCard = 1;
			pDirent->szWildCard[i-1] = '\0';
		}
	}
#endif

	if(pDirent->DirCluster == 0) {
		return FF_ERR_DIR_INVALID_PATH | FF_FINDFIRST;
	}

	// Initialise the Fetch Context
	Error = FF_InitEntryFetch(pIoman, pDirent->DirCluster, &pDirent->FetchContext);
	if(FF_isERR(Error)) {
		return Error;
	}

	pDirent->CurrentItem = 0;

	return FF_FindNext(pIoman, pDirent);

}

/**
 *	@public
 *	@brief	Get's the next Entry based on the data recorded in the FF_DIRENT object.
 *
 *	All values recorded in pDirent must be preserved to and between calls to
 *	FF_FindNext(). Please see @see FF_FindFirst() for find initialisation.
 *
 *	@param	pIoman		FF_IOMAN object that was created by FF_CreateIOMAN().
 *	@param	pDirent		FF_DIRENT object to store the entry information. (As initialised by FF_FindFirst()).
 *
 *	@return FF_ERR_DEVICE_DRIVER_FAILED is device access failed.
 *
 **/
FF_ERROR FF_FindNext(FF_IOMAN *pIoman, FF_DIRENT *pDirent) {

	FF_ERROR	Error;
	FF_T_UINT8	numLFNs;
	FF_T_UINT8	EntryBuffer[32];
#ifdef FF_FINDAPI_ALLOW_WILDCARDS
	FF_T_BOOL	b;
#endif

	if(!pIoman) {
		return FF_ERR_NULL_POINTER | FF_FINDNEXT;
	}

	for(; pDirent->CurrentItem < 0xFFFF; pDirent->CurrentItem += 1) {
		Error = FF_FetchEntryWithContext(pIoman, pDirent->CurrentItem, &pDirent->FetchContext, EntryBuffer);
		if(FF_isERR(Error)) {
			FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);	// Don't check errors, already passing an error.
			return Error;
		}
		if(EntryBuffer[0] != FF_FAT_DELETED) {
			if(FF_isEndOfDir(EntryBuffer)){
				FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
				return FF_ERR_DIR_END_OF_DIR | FF_FINDNEXT;
			}
			pDirent->Attrib = FF_getChar(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB));
			if((pDirent->Attrib & FF_FAT_ATTR_LFN) == FF_FAT_ATTR_LFN) {
				// LFN Processing
				numLFNs = (FF_T_UINT8)(EntryBuffer[0] & ~0x40);
				// Get the shortname and check if it is marked deleted.
#ifdef FF_LFN_SUPPORT
				// Fetch the shortname, and get it's checksum, or for a deleted item with
				// orphaned LFN entries.
				Error = FF_FetchEntryWithContext(pIoman, (pDirent->CurrentItem + numLFNs), &pDirent->FetchContext, EntryBuffer);
				if(FF_isERR(Error)) {
					FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
					return Error;
				}

				if(EntryBuffer[0] != FF_FAT_DELETED) {
					Error = FF_PopulateLongDirent(pIoman, pDirent, pDirent->CurrentItem, &pDirent->FetchContext);
					if(FF_isERR(Error)) {
						FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
						return Error;
					}
#ifdef FF_INCLUDE_SHORT_NAME
					pDirent->Attrib |= FF_FAT_ATTR_IS_LFN;
#endif

#ifdef FF_FINDAPI_ALLOW_WILDCARDS
#ifdef FF_UNICODE_SUPPORT
					if(wcscmp(pDirent->szWildCard, L""))
#else
					if(pDirent->szWildCard[0])
#endif
					{  // HT put single bracket here because of bracket-matching within editor
						b = FF_wildcompare(pDirent->szWildCard, pDirent->FileName);
						if(pDirent->bInvertWildCard) {
							b = !b;
						}
						if(b) {
							Error = FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
							if(FF_isERR(Error)) {
								return Error;
							}
							return FF_ERR_NONE;
						}

						pDirent->CurrentItem -= 1;
					} else {
						Error = FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
						if(FF_isERR(Error)) {
							return Error;
						}
						return FF_ERR_NONE;
					}
#else
					Error = FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
					if(FF_isERR(Error)) {
						return Error;
					}
					return FF_ERR_NONE;
#endif
				}
#else
				pDirent->CurrentItem += (numLFNs - 1);
#endif
			} else if((pDirent->Attrib & FF_FAT_ATTR_VOLID) == FF_FAT_ATTR_VOLID) {
				// Do Nothing

			} else {
				FF_PopulateShortDirent(pIoman, pDirent, EntryBuffer);
#if defined(FF_SHORTNAME_CASE)
				// Apply NT/XP+ bits to get correct case
				FF_CaseShortName(pDirent->FileName, FF_getChar(EntryBuffer, FF_FAT_CASE_OFFS));
#endif
#ifdef FF_FINDAPI_ALLOW_WILDCARDS
				if(pDirent->szWildCard[0]) {
					b = FF_wildcompare(pDirent->szWildCard, pDirent->FileName);
					if(pDirent->bInvertWildCard) {
						b = !b;
					}
					if(b) {
						pDirent->CurrentItem += 1;
						Error = FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
						if(FF_isERR(Error)) {
							return Error;
						}
						return FF_ERR_NONE;
					}
				} else {
					pDirent->CurrentItem += 1;
					Error = FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
					if(FF_isERR(Error)) {
						return Error;
					}
					return FF_ERR_NONE;
				}
#else

				pDirent->CurrentItem += 1;
				FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
				if(FF_isERR(Error)) {
					return Error;
				}
				return FF_ERR_NONE;
#endif
			}
		}
	}
	Error = FF_CleanupEntryFetch(pIoman, &pDirent->FetchContext);
	if(FF_isERR(Error)) {
		return Error;
	}

	return FF_ERR_DIR_END_OF_DIR | FF_FINDNEXT;
}


FF_ERROR FF_RewindFind(FF_IOMAN *pIoman, FF_DIRENT *pDirent) {
	if(!pIoman) {
		return FF_ERR_NULL_POINTER | FF_REWINDFIND;
	}
	pDirent->CurrentItem = 0;
	return FF_ERR_NONE;
}

/*
	Returns >= 0 for a free dirent entry.
	Returns <  0 with and Error code if anything goes wrong.
*/
static FF_T_SINT32 FF_FindFreeDirent(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_UINT16 Sequential) {

	FF_T_UINT8			EntryBuffer[32];
	FF_T_UINT16			freeCount = 0;
	FF_T_UINT			nEntry;
	FF_ERROR			Error;
	FF_T_UINT32			DirLength;
	FF_FETCH_CONTEXT	FetchContext;

	Error = FF_InitEntryFetch(pIoman, DirCluster, &FetchContext);
	if(FF_isERR(Error)) {
		return Error;
	}

	for(nEntry = 0; nEntry < 0xFFFF; nEntry++) {
		Error = FF_FetchEntryWithContext(pIoman, nEntry, &FetchContext, EntryBuffer);
		if(FF_GETERROR(Error) == FF_ERR_DIR_END_OF_DIR) {

			Error = FF_ExtendDirectory(pIoman, DirCluster);
			if(FF_isERR(Error)) {
				FF_CleanupEntryFetch(pIoman, &FetchContext);
				return Error;
			}

			Error = FF_CleanupEntryFetch(pIoman, &FetchContext);
			if(FF_isERR(Error)) {
				return Error;
			}

			return nEntry;
		} else {
			if(FF_isERR(Error)) {
				FF_CleanupEntryFetch(pIoman, &FetchContext); // Dont override the current error!
				return Error;
			}
		}
		if(FF_isEndOfDir(EntryBuffer)) {	// If its the end of the Dir, then FreeDirents from here.
			// Check Dir is long enough!
			DirLength = FetchContext.ulChainLength;//FF_GetChainLength(pIoman, DirCluster, &iEndOfChain);
			if((nEntry + Sequential) > ((DirLength * ((FF_T_UINT)pIoman->pPartition->SectorsPerCluster * pIoman->pPartition->BlkSize)) / 32)) {
				Error = FF_ExtendDirectory(pIoman, DirCluster);
			}
			if(FF_isERR(Error)) {
				FF_CleanupEntryFetch(pIoman, &FetchContext);
				return Error;
			}

			Error = FF_CleanupEntryFetch(pIoman, &FetchContext);
			if(FF_isERR(Error)) {
				return Error;
			}

			return nEntry;
		}
		if(EntryBuffer[0] == 0xE5) {
			freeCount++;
		} else {
			freeCount = 0;
		}

		if(freeCount == Sequential) {
			Error = FF_CleanupEntryFetch(pIoman, &FetchContext);
			if(FF_isERR(Error)) {
				return Error;
			}
			return (nEntry - (Sequential - 1));// Return the beginning entry in the sequential sequence.
		}
	}

	Error = FF_CleanupEntryFetch(pIoman, &FetchContext);
	if(FF_isERR(Error)) {
		return Error;
	}

	return FF_ERR_DIR_DIRECTORY_FULL | FF_FINDFREEDIRENT;
}
FF_ERROR FF_PutEntry(FF_IOMAN *pIoman, FF_T_UINT16 Entry, FF_T_UINT32 DirCluster, FF_DIRENT *pDirent) {
	FF_ERROR	Error;
	FF_T_UINT8	EntryBuffer[32];
	FF_FETCH_CONTEXT FetchContext;

	// HT: use the standard access routine to get the same logic for root dirs
	Error = FF_InitEntryFetch(pIoman, DirCluster, &FetchContext);
	if(!FF_isERR(Error)) {
		Error = FF_FetchEntryWithContext(pIoman, Entry, &FetchContext, EntryBuffer);
		if(!FF_isERR(Error)) {
			// Cleanup probably not necessary here?
			// FF_PushEntryWithContext checks for R/W flag
			Error = FF_CleanupEntryFetch(pIoman, &FetchContext);
			if(FF_isERR(Error)) {
				goto cleanup;
			}

			FF_putChar(EntryBuffer,  FF_FAT_DIRENT_ATTRIB,    pDirent->Attrib);
			FF_putShort(EntryBuffer, FF_FAT_DIRENT_CLUS_HIGH, (FF_T_UINT16)(pDirent->ObjectCluster >> 16));
			FF_putShort(EntryBuffer, FF_FAT_DIRENT_CLUS_LOW,  (FF_T_UINT16)(pDirent->ObjectCluster));
			FF_putLong(EntryBuffer,  FF_FAT_DIRENT_FILESIZE,  pDirent->Filesize);
#ifdef FF_TIME_SUPPORT
			FF_GetSystemTime(&pDirent->AccessedTime);	///< Date of Last Access.
			FF_PlaceTime(EntryBuffer, FF_FAT_DIRENT_LASTACC_DATE, &pDirent->AccessedTime);
			FF_PlaceDate(EntryBuffer, FF_FAT_DIRENT_LASTACC_DATE, &pDirent->AccessedTime);	// Last accessed date.
			FF_PlaceTime(EntryBuffer, FF_FAT_DIRENT_CREATE_TIME,  &pDirent->CreateTime);
			FF_PlaceDate(EntryBuffer, FF_FAT_DIRENT_CREATE_DATE,  &pDirent->CreateTime);
			FF_PlaceTime(EntryBuffer, FF_FAT_DIRENT_LASTMOD_TIME, &pDirent->ModifiedTime);
			FF_PlaceDate(EntryBuffer, FF_FAT_DIRENT_LASTMOD_DATE, &pDirent->ModifiedTime);
#endif
			Error = FF_PushEntryWithContext(pIoman, Entry, &FetchContext, EntryBuffer);
		}
	}

cleanup:
	FF_CleanupEntryFetch(pIoman, &FetchContext);
	return Error;
}

FF_T_BOOL FF_ValidShortChar (FF_T_INT8 Chr)
{
	return (Chr >= 'A' && Chr <= 'Z') ||
		(Chr >= 'a' && Chr <= 'z') ||	// lower-case can be stored using NT/XP attribute
		(Chr >= '0' && Chr <= '9') ||
		strchr ("$%-_@~`!(){}^#&", Chr) != NULL;
}

FF_T_BOOL FF_ValidLongChar (FF_T_INT8 Chr)
{
	return Chr >= 0x20 && strchr ("/\\:*?\"<>|", Chr) == NULL;
}

#ifdef FF_UNICODE_SUPPORT
FF_T_SINT32 FF_CreateShortName(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_WCHAR *ShortName, FF_T_WCHAR *LongName) {
#else
FF_T_SINT32 FF_CreateShortName(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_INT8 *ShortName, FF_T_INT8 *LongName) {
#endif
	FF_T_UINT8 caseAttrib = 0;
#if defined(FF_SHORTNAME_CASE)
	FF_T_UINT8 testAttrib = FF_FAT_CASE_ATTR_BASE;
#endif

	FF_T_UINT16 i,x,y,last_dot;
	FF_T_UINT16 first_tilde = 6;
#ifdef FF_UNICODE_SUPPORT
	FF_T_WCHAR	MyShortName[13];
#else
	FF_T_INT8	MyShortName[13];
#endif
	FF_T_UINT16 NameLen;
	FF_T_BOOL	FitsShort = FF_TRUE;
	FF_T_BOOL	SizeOk = FF_TRUE;
	FF_DIRENT	MyDir;
//	FF_T_BOOL   found;
	//FF_T_SINT8	RetVal = 0;
	FF_T_INT8	NumberBuf[6];
	FF_ERROR	Error;

#ifdef FF_UNICODE_SUPPORT
	NameLen = (FF_T_UINT16) wcslen(LongName);
#else
	NameLen = (FF_T_UINT16) strlen(LongName);
#endif

	// Does LongName fit a shortname?

	for(i = 0, x = 0, last_dot = NameLen; i < NameLen; i++) {
		if(LongName[i] != '.') {
			x++;
		} else {
			last_dot = i;
		}
	}

	if (NameLen > 12 || NameLen-x > 1 || NameLen-last_dot > 4 || last_dot > 8) {
		SizeOk = FF_FALSE;
	}

	for(i = 0, x = 0; i < 11; x++) {
		FF_T_INT8 ch = (FF_T_INT8) LongName[x];
		if (!ch)
			break;
		if (x == last_dot) {
			// Remember where we put the first space
			if (first_tilde > i)
				first_tilde = i;
			while (i < 8)
				ShortName[i++] = 0x20;
#if defined(FF_SHORTNAME_CASE)
			testAttrib = FF_FAT_CASE_ATTR_EXT;
#endif
		} else {
			if (i == 8) {
				// HT change
				// Code hung with this filename:
				// 'Chopin, Frederic - The Piano Works Op. 55, Op. 62, Op'
				if (x <= last_dot) {
					x = last_dot;
					ch = (FF_T_INT8) LongName[x];
					if (!ch)
						break;
					ch = (FF_T_INT8) LongName[++x];
#if defined(FF_SHORTNAME_CASE)
					testAttrib = FF_FAT_CASE_ATTR_EXT;
#endif
				}
			}
			if (!FF_ValidShortChar (ch)) {
				FitsShort = FF_FALSE;
				continue;
			}
			if (ch >= 'a' && ch <= 'z') {
				ch -= 0x20;
#if defined(FF_SHORTNAME_CASE)
				if (testAttrib)
					caseAttrib |= testAttrib;
				else
					FitsShort = FF_FALSE;	// We had capital: does not fit
			} else if (ch >= 'A' && ch <= 'Z') {
				if (caseAttrib & testAttrib)
					FitsShort = FF_FALSE;	// We had lower-case: does not fit
				testAttrib = 0;
#endif
			}
			ShortName[i++] = ch;
		}
	}
	while (i < 11)
		ShortName[i++] = 0x20;
	if (last_dot < first_tilde)
		first_tilde = last_dot;
	if (NameLen < first_tilde)	// Names like "Abc" will become "~Abc"
		first_tilde = NameLen;

	// Tail :
	memcpy(MyShortName, ShortName, 11);
	FF_ProcessShortName(MyShortName);
	if(FitsShort && SizeOk) {
		if (!FF_FindEntryInDir(pIoman, DirCluster, MyShortName, 0x00, &MyDir, &Error)) {
			if(FF_isERR(Error)) {
				return Error;
			}
			return caseAttrib | 0x01;
		}
		return FF_ERR_DIR_OBJECT_EXISTS | FF_CREATESHORTNAME;
	}
	for(i = (SizeOk ? 0 : 1); i < 0x0000FFFF; i++) { // Max Number of Entries in a DIR!
		// In the first round, check if the original name can be used
		// Makefile will be stored as "makefile" and not as "makefi~1"
		if (i) {
			sprintf(NumberBuf, "%d", i);
			NameLen = (FF_T_UINT16) strlen(NumberBuf);
			x = 7 - NameLen;
			if (x > first_tilde)
				x = first_tilde;
			ShortName[x++] = '~';
			for(y = 0; y < NameLen; y++) {
				ShortName[x+y] = NumberBuf[y];
			}
		}
		memcpy(MyShortName, ShortName, 11);
		FF_ProcessShortName(MyShortName);
		if(!FF_ShortNameExists(pIoman, DirCluster, MyShortName, &Error)) {
/*
// HT: will do this later when everything has been checked
// if not, hash entries might become incorrect
#ifdef FF_HASH_CACHE
#if FF_HASH_FUNCTION == CRC16
			FF_AddDirentHash(pIoman, DirCluster, (FF_T_UINT32) FF_GetCRC16((FF_T_UINT8*)MyShortName, strlen(MyShortName)));
#elif FF_HASH_FUNCTION == CRC8
			FF_AddDirentHash(pIoman, DirCluster, (FF_T_UINT32) FF_GetCRC8((FF_T_UINT8*)MyShortName, strlen(MyShortName)));
#endif
#endif
*/

			if(FF_isERR(Error)) {
				return Error;
			}
			return FF_ERR_NONE;
		}
	}
	// Add a tail and special number until we're happy :D

	return FF_ERR_DIR_DIRECTORY_FULL | FF_CREATESHORTNAME;
}


#ifdef FF_LFN_SUPPORT
static FF_T_SINT8 FF_CreateLFNEntry(FF_T_UINT8 *EntryBuffer, FF_T_UINT8 *Name, FF_T_UINT uiNameLen, FF_T_UINT uiLFN, FF_T_UINT8 CheckSum) {
	/*
	 *	HT for JW:
	 *	Changed *Name from 16- to of 8-bits
     *	The caller of this function doesn't need an expensive
	 *	FF_T_UINT16 usUtf16Name[FF_MAX_FILENAME + 1];
	 *  in case UNICODE isn't used
	 *  Also did quite a bit of optimisation here
     *  and tested well
	 */
	FF_T_UINT i, x;

	memset(EntryBuffer, 0, 32);

	FF_putChar(EntryBuffer, FF_FAT_LFN_ORD,			(FF_T_UINT8) ((uiLFN & ~0x40)));
	FF_putChar(EntryBuffer, FF_FAT_DIRENT_ATTRIB,	(FF_T_UINT8) FF_FAT_ATTR_LFN);
	FF_putChar(EntryBuffer, FF_FAT_LFN_CHECKSUM,	(FF_T_UINT8) CheckSum);

	// Name_1
	i = 0;
	for(x = FF_FAT_LFN_NAME_1; i < 5; i++, x += 2) {
		if (i < uiNameLen) {
			EntryBuffer[x] = *(Name++);
#if defined(FF_UNICODE_SUPPORT) || defined(FF_UNICODE_UTF8_SUPPORT)
			EntryBuffer[x + 1] = *(Name++);
#endif
		} else if (i > uiNameLen) {
			EntryBuffer[x]		= 0xFF;
			EntryBuffer[x + 1]	= 0xFF;
		}
	}

	// Name_2
	for(x = FF_FAT_LFN_NAME_2; i < 11; i++, x += 2) {
		if (i < uiNameLen) {
			EntryBuffer[x] = *(Name++);
#if defined(FF_UNICODE_SUPPORT) || defined(FF_UNICODE_UTF8_SUPPORT)
			EntryBuffer[x + 1] = *(Name++);
#endif
		} else if (i > uiNameLen) {
			EntryBuffer[x]		= 0xFF;
			EntryBuffer[x + 1]	= 0xFF;
		}
	}

	// Name_3
	for(x = FF_FAT_LFN_NAME_3; i < 13; i++, x += 2) {
		if(i < uiNameLen) {
			EntryBuffer[x] = *(Name++);
#if defined(FF_UNICODE_SUPPORT) || defined(FF_UNICODE_UTF8_SUPPORT)
			EntryBuffer[x + 1] = *(Name++);
#endif
		} else if (i > uiNameLen) {
			EntryBuffer[x]		= 0xFF;
			EntryBuffer[x + 1]	= 0xFF;
		}
	}

	return FF_ERR_NONE;
}
#endif

#ifdef FF_LFN_SUPPORT
#ifdef FF_UNICODE_SUPPORT
static FF_ERROR FF_CreateLFNs(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_WCHAR *Name, FF_T_UINT8 CheckSum, FF_T_UINT16 nEntry) {
#else
static FF_ERROR FF_CreateLFNs(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_INT8 *Name, FF_T_UINT8 CheckSum, FF_T_UINT16 nEntry) {
#endif
	FF_ERROR			Error;
	FF_T_UINT			uiNumLFNs;
	FF_T_UINT			uiEndPos;
	FF_T_UINT			i,y;

#ifdef FF_UNICODE_UTF8_SUPPORT
	FF_T_SINT32			slRetVal;
#endif

#ifndef FF_UNICODE_SUPPORT
#ifndef FF_UNICODE_UTF8_SUPPORT
	//FF_T_UINT16			*pUtf16;
#endif
#endif

	FF_FETCH_CONTEXT	FetchContext;

	FF_T_UINT8			EntryBuffer[32];
	// HT: Save a stack-expensive declaration of "usUtf16Name"
#if defined(FF_UNICODE_SUPPORT) || defined(FF_UNICODE_UTF8_SUPPORT)
	FF_T_UINT16			usUtf16Name[FF_MAX_FILENAME + 1];
#endif

#ifndef FF_UNICODE_SUPPORT
	FF_T_INT8			*NamePtr;
#else
	FF_T_INT16			*NamePtr;
#endif


#ifdef FF_UNICODE_SUPPORT
#if WCHAR_MAX <= 0xFFFF
	y = wcslen(Name);
	if(y > FF_MAX_FILENAME) {
		return FF_ERR_DIR_NAME_TOO_LONG | FF_CREATELFNS;
	}
	wcsncpy(usUtf16Name, Name, FF_MAX_FILENAME);
#else
	i = 0;
	y = 0;
	while(Name[i]) {
		FF_Utf32ctoUtf16c(&usUtf16Name[y], (FF_T_UINT32) Name[i], FF_MAX_FILENAME - i);
		y += FF_GetUtf16SequenceLen(usUtf16Name[y]);
		i++;
		if(y > FF_MAX_FILENAME) {
			return FF_ERR_DIR_NAME_TOO_LONG | FF_CREATELFNS;
		}
	}
#endif
#endif
	// Convert the name into UTF-16 format.
#ifdef FF_UNICODE_UTF8_SUPPORT
	// Simply convert the UTF8 to UTF16 and be done with it.
	i = 0;
	y = 0;
	while(Name[i]) {
		slRetVal = FF_Utf8ctoUtf16c(&usUtf16Name[y], (FF_T_UINT8 *)&Name[i], FF_MAX_FILENAME - i);
		if(slRetVal > 0) {
			i += slRetVal;
		} else {
			break;	// No more space in the UTF-16 buffer, simply truncate for safety.
		}
		y += FF_GetUtf16SequenceLen(usUtf16Name[y]);
		if(y > FF_MAX_FILENAME) {
			return FF_ERR_DIR_NAME_TOO_LONG | FF_CREATELFNS;
		}
	}
#elif !defined(FF_UNICODE_SUPPORT)
	// Just check the length
	y = strlen(Name);
	if(y > FF_MAX_FILENAME) {
		return FF_ERR_DIR_NAME_TOO_LONG | FF_CREATELFNS;
	}
#endif

	// Whole name is now in a valid UTF-16 format. Lets go make thos LFN's.
	// i should at this point be the length of the name.

	uiNumLFNs	= y / 13;	// Number of LFNs is the total number of UTF-16 units, divided by 13 (13 units per LFN).
	uiEndPos	= y % 13;	// The ending position in an LFN, of the last LFN UTF-16 charachter.

	if(uiEndPos) {
		uiNumLFNs++;
	} else {
		uiEndPos = 13;
	}

	Error = FF_InitEntryFetch(pIoman, DirCluster, &FetchContext);
	if(FF_isERR(Error)) {
		return Error;
	}
#if defined(FF_UNICODE_SUPPORT)
	NamePtr = (FF_T_INT16*)(usUtf16Name + 13 * (uiNumLFNs-1));
#elif defined(FF_UNICODE_UTF8_SUPPORT)
	NamePtr = (FF_T_INT8*)(usUtf16Name + 13 * (uiNumLFNs-1));
#else
	NamePtr = Name + 13 * (uiNumLFNs-1);
#endif
	// After this point, i is no longer the length of the Filename in UTF-16 units.
	for(i = uiNumLFNs; i > 0; i--) {
		if(i == uiNumLFNs) {
			FF_CreateLFNEntry(EntryBuffer, (FF_T_UINT8 *) NamePtr, uiEndPos, i, CheckSum);
			EntryBuffer[0] |= 0x40;
		} else {
			FF_CreateLFNEntry(EntryBuffer, (FF_T_UINT8 *) NamePtr, 13, i, CheckSum);
		}
#if defined(FF_UNICODE_SUPPORT) || defined(FF_UNICODE_UTF8_SUPPORT)
		NamePtr -= 13 * sizeof *usUtf16Name;
#else
		NamePtr -= 13 * sizeof *Name;
#endif
		Error = FF_PushEntryWithContext(pIoman, nEntry + (uiNumLFNs - i), &FetchContext, EntryBuffer);
		if(FF_isERR(Error)) {
			FF_CleanupEntryFetch(pIoman, &FetchContext);	// Dont override error!
			return Error;
		}
	}

	Error = FF_CleanupEntryFetch(pIoman, &FetchContext);
	if(FF_isERR(Error)) {
		return Error;
	}

	return FF_ERR_NONE;
}
#endif

FF_ERROR FF_ExtendDirectory(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster) {
	FF_T_UINT32 CurrentCluster;
	FF_T_UINT32 NextCluster;
	FF_ERROR Error;
	FF_FatBuffers FatBuf;

	if(pIoman->pPartition->Type != FF_T_FAT32) {
		if(DirCluster == pIoman->pPartition->RootDirCluster) {
			return FF_ERR_DIR_CANT_EXTEND_ROOT_DIR | FF_EXTENDDIRECTORY;
		}
	}

	if(!pIoman->pPartition->FreeClusterCount) {
		pIoman->pPartition->FreeClusterCount = FF_CountFreeClusters(pIoman, &Error);
		if(FF_isERR(Error)) {
			return Error;
		}
		if(pIoman->pPartition->FreeClusterCount == 0) {
			return FF_ERR_FAT_NO_FREE_CLUSTERS | FF_EXTENDDIRECTORY;
		}
	}

	FF_lockFAT(pIoman);
	{
		CurrentCluster = FF_FindEndOfChain(pIoman, DirCluster, &Error);
		if(FF_isERR(Error)) {
			FF_unlockFAT(pIoman);
			return Error;
		}

		NextCluster = FF_FindFreeCluster(pIoman, &Error);
		if(FF_isERR(Error)) {
			FF_unlockFAT(pIoman);
			return Error;
		}

		FF_InitFatBuffer (&FatBuf, FF_MODE_WRITE);
		Error = FF_putFatEntry(pIoman, CurrentCluster, NextCluster, &FatBuf);
		if(!FF_isERR(Error)) {
			Error = FF_putFatEntry(pIoman, NextCluster, 0xFFFFFFFF, &FatBuf);
		}
		Error = FF_ReleaseFatBuffer(pIoman, &FatBuf);
		if(FF_isERR(Error)) {
			FF_unlockFAT(pIoman);
			return Error;
		}
	}
	FF_unlockFAT(pIoman);

	Error = FF_ClearCluster(pIoman, NextCluster);
	if(FF_isERR(Error)) {
		FF_unlockFAT(pIoman);
		return Error;
	}

	Error = FF_DecreaseFreeClusters(pIoman, 1);
	if(FF_isERR(Error)) {
		FF_unlockFAT(pIoman);
		return Error;
	}

	return FF_ERR_NONE;
}

static const FF_T_UINT8 forbiddenChrs[] = {
//  Windows says: don't use these characters: '\/:*?"<>|'
//    "     *     /	    :     <     >     ?    '\'    ?     |
	0x22, 0x2A, 0x2F, 0x3A, 0x3C, 0x3E, 0x3F, 0x5C, 0x7F, 0x7C
};
#ifdef FF_UNICODE_SUPPORT
static void FF_MakeNameCompliant(FF_T_WCHAR *Name) {
#else
static void FF_MakeNameCompliant(FF_T_UINT8 *Name) {
#endif
	FF_T_INT index;
	if((FF_T_UINT8) Name[0] == 0xE5) {	// Support Japanese KANJI symbol.
		Name[0] = 0x05;
	}
	for (; *Name; Name++) {
		for (index = 0; index < sizeof forbiddenChrs; index++) {
			if (*Name == forbiddenChrs[index]) {
				*Name = '_';
				break;
			}
		}
	}
}

FF_ERROR FF_CreateDirent(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_DIRENT *pDirent) {

	FF_T_UINT8	EntryBuffer[32];
#ifdef FF_HASH_CACHE
	FF_T_INT8	ShortName[13];
#endif
#ifdef FF_UNICODE_SUPPORT
	FF_T_UINT16	NameLen = (FF_T_UINT16) wcslen(pDirent->FileName);
#else
	FF_T_UINT16	NameLen = (FF_T_UINT16) strlen(pDirent->FileName);
#endif
	// HT calculate with roundup
	FF_T_UINT8	numLFNs = (FF_T_UINT8) ((NameLen+12) / 13);
	FF_T_SINT32	FreeEntry;
	FF_ERROR	RetVal = FF_ERR_NONE;
	FF_T_UINT8	Entries;

	FF_FETCH_CONTEXT FetchContext;

#ifdef FF_LFN_SUPPORT
	FF_T_UINT8	CheckSum;
#endif

#ifdef FF_UNICODE_SUPPORT
	FF_T_WCHAR	UTF16EntryBuffer[32];
#if WCHAR_MAX > 0xFFFF
	// Check that the filename won't exceed the max LFN length if converted to UTF-16.
	//if(FF_Utf32GetUtf16Len((FF_T_UINT32 *) pDirent->FileName) > FF_MAX_FILENAME) {
	//	return FF_ERR_UNICODE_CONVERSION_EXCEEDED;
	//}
#endif

#endif

#ifdef FF_UNICODE_SUPPORT
	FF_MakeNameCompliant(pDirent->FileName);	// Ensure we don't break the Dir tables.
#else
	FF_MakeNameCompliant((FF_T_UINT8 *)pDirent->FileName);	// Ensure we don't break the Dir tables.
#endif
	memset(EntryBuffer, 0, sizeof EntryBuffer);

#ifdef FF_LFN_SUPPORT
	// Create and push the LFN's
	Entries = numLFNs + 1;	// Find enough places for the LFNs and the ShortName
#else
	Entries = 1;
#endif

	// Create the ShortName
	FF_lockDIR(pIoman);
	{
		// HT: As CreateShortName() can fail, it should be called before
		//     finding a free dir entry
#ifdef FF_UNICODE_SUPPORT
		//FF_cstrntowcs(UTF16EntryBuffer, (FF_T_INT8 *) EntryBuffer, 32);
		FF_T_SINT32 FitShort = FF_CreateShortName(pIoman, DirCluster, UTF16EntryBuffer, pDirent->FileName);
#else
		FF_T_SINT32 FitShort = FF_CreateShortName(pIoman, DirCluster, (FF_T_INT8 *) EntryBuffer, pDirent->FileName);
#endif
		if (FF_isERR(FitShort)) {
			RetVal = FitShort;
			FF_unlockDIR(pIoman);
			return RetVal;
		}
		if (FitShort) {
			numLFNs = 0;
			Entries = 1;
		}
		FreeEntry = FF_FindFreeDirent(pIoman, DirCluster, Entries);
		if (FF_isERR(FreeEntry)) {
			RetVal = FreeEntry;
		} else {
#ifdef FF_LFN_SUPPORT
#ifdef FF_UNICODE_SUPPORT
			FF_wcsntocstr((FF_T_INT8 *) EntryBuffer, UTF16EntryBuffer, 11);
#endif
			if (numLFNs) {
				CheckSum = FF_CreateChkSum(EntryBuffer);
				RetVal = FF_CreateLFNs(pIoman, DirCluster, pDirent->FileName, CheckSum, (FF_T_UINT16) FreeEntry);
			}
#else
			numLFNs = 0;
#endif // FF_LFN_SUPPORT
			if (!FF_isERR(RetVal)) {
#ifdef FF_TIME_SUPPORT
				FF_GetSystemTime(&pDirent->CreateTime);		///< Date and Time Created.
				pDirent->ModifiedTime = pDirent->CreateTime;	///< Date and Time Modified.
				pDirent->AccessedTime = pDirent->CreateTime;	///< Date of Last Access.
				FF_PlaceTime(EntryBuffer, FF_FAT_DIRENT_CREATE_TIME, &pDirent->CreateTime);
				FF_PlaceDate(EntryBuffer, FF_FAT_DIRENT_CREATE_DATE, &pDirent->CreateTime);
				FF_PlaceTime(EntryBuffer, FF_FAT_DIRENT_LASTMOD_TIME, &pDirent->ModifiedTime);
				FF_PlaceDate(EntryBuffer, FF_FAT_DIRENT_LASTMOD_DATE, &pDirent->ModifiedTime);
#endif

				FF_putChar(EntryBuffer,  FF_FAT_DIRENT_ATTRIB, pDirent->Attrib);
#if defined(FF_SHORTNAME_CASE)
				FF_putChar(EntryBuffer,  FF_FAT_CASE_OFFS, FitShort & (FF_FAT_CASE_ATTR_BASE|FF_FAT_CASE_ATTR_EXT));
#endif
				FF_putShort(EntryBuffer, FF_FAT_DIRENT_CLUS_HIGH, (FF_T_UINT16)(pDirent->ObjectCluster >> 16));
				FF_putShort(EntryBuffer, FF_FAT_DIRENT_CLUS_LOW, (FF_T_UINT16)(pDirent->ObjectCluster));
				FF_putLong(EntryBuffer,  FF_FAT_DIRENT_FILESIZE, pDirent->Filesize);

				RetVal = FF_InitEntryFetch(pIoman, DirCluster, &FetchContext);
				if(FF_isERR(RetVal)) {
					FF_unlockDIR(pIoman);
					return RetVal;
				}
				RetVal = FF_PushEntryWithContext(pIoman, (FF_T_UINT16) (FreeEntry + numLFNs), &FetchContext, EntryBuffer);
				if(FF_isERR(RetVal)) {
					FF_CleanupEntryFetch(pIoman, &FetchContext);
					FF_unlockDIR(pIoman);
					return RetVal;
				}

				RetVal = FF_CleanupEntryFetch(pIoman, &FetchContext);
				if(FF_isERR(RetVal)) {
					FF_unlockDIR(pIoman);
					return RetVal;
				}
#ifdef FF_HASH_CACHE
				if(!FF_DirHashed(pIoman, DirCluster)) {
					// Hash the directory
					FF_HashDir(pIoman, DirCluster);
				}

				memcpy (ShortName, EntryBuffer, 11);
				FF_ProcessShortName(ShortName);		// Format the shortname to 8.3
#if FF_HASH_FUNCTION == CRC16
				FF_AddDirentHash(pIoman, DirCluster, (FF_T_UINT32)FF_GetCRC16((FF_T_UINT8 *) ShortName, strlen(ShortName)));
#elif FF_HASH_FUNCTION == CRC8
				FF_AddDirentHash(pIoman, DirCluster, (FF_T_UINT32)FF_GetCRC8((FF_T_UINT8 *) ShortName, strlen(ShortName)));
#endif
#endif
			}
		}
	}
	FF_unlockDIR(pIoman);

	if(FF_isERR(RetVal)) {
		return RetVal;
	}

	if(pDirent) {
		pDirent->CurrentItem = (FF_T_UINT16) (FreeEntry + numLFNs);
	}

	return FF_ERR_NONE;
}


#ifdef FF_UNICODE_SUPPORT
FF_T_UINT32 FF_CreateFile(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_WCHAR *FileName, FF_DIRENT *pDirent, FF_ERROR *pError) {
#else
FF_T_UINT32 FF_CreateFile(FF_IOMAN *pIoman, FF_T_UINT32 DirCluster, FF_T_INT8 *FileName, FF_DIRENT *pDirent, FF_ERROR *pError) {
#endif
	FF_DIRENT	MyFile;
	memset (&MyFile, '\0', sizeof MyFile);
	*pError	= FF_ERR_NONE;
#ifdef FF_UNICODE_SUPPORT
	wcsncpy(MyFile.FileName, FileName, FF_MAX_FILENAME);
#else
	strncpy(MyFile.FileName, FileName, FF_MAX_FILENAME);
#endif

	MyFile.ObjectCluster = FF_CreateClusterChain(pIoman, pError);
	if(FF_isERR(*pError)) {
		// HT: TODO: can we unlink what we didn't create?
		// JW: This is ok, because if the item item has a 0 in it, the no unlinking will occur.
		// JW: I added the if statement, to ensure we don't unlink with 0.
		if(MyFile.ObjectCluster) {
			FF_lockFAT(pIoman);
			{
				FF_UnlinkClusterChain(pIoman, MyFile.ObjectCluster, 0);
			}
			FF_unlockFAT(pIoman);
		}
		FF_FlushCache(pIoman);	// Don't override error;
		return 0;
	}

	*pError = FF_CreateDirent(pIoman, DirCluster, &MyFile);
	if(FF_isERR(*pError)) {
		FF_lockFAT(pIoman);
		{
			FF_UnlinkClusterChain(pIoman, MyFile.ObjectCluster, 0);
		}
		FF_unlockFAT(pIoman);
		FF_FlushCache(pIoman);	// Don't override error;
		return 0;
	}

	*pError = FF_FlushCache(pIoman);

	if(pDirent) {
		memcpy(pDirent, &MyFile, sizeof(FF_DIRENT));
	}

	return MyFile.ObjectCluster;
}


/**
 *	@brief Creates a Directory of the specified path.
 *
 *	@param	pIoman	Pointer to the FF_IOMAN object.
 *	@param	Path	Path of the directory to create.
 *
 *	@return	FF_ERR_NULL_POINTER if pIoman was NULL.
 *	@return FF_ERR_DIR_OBJECT_EXISTS if the object specified by path already exists.
 *	@return	FF_ERR_DIR_INVALID_PATH
 *	@return FF_ERR_NONE on success.
 **/
#ifdef FF_UNICODE_SUPPORT
FF_ERROR FF_MkDir(FF_IOMAN *pIoman, const FF_T_WCHAR *Path) {
#else
FF_ERROR FF_MkDir(FF_IOMAN *pIoman, const FF_T_INT8 *Path) {
#endif
	FF_DIRENT	MyDir;
	FF_T_UINT32 DirCluster;
#ifdef FF_UNICODE_SUPPORT
	const FF_T_WCHAR *DirName;
#else
	const FF_T_INT8	*DirName;
#endif
	FF_T_UINT8	EntryBuffer[32];
	FF_T_UINT32 DotDotCluster;
	FF_T_UINT16	i;
	FF_ERROR	Error = FF_ERR_NONE;

	FF_FETCH_CONTEXT FetchContext;

	if(!pIoman) {
		return FF_ERR_NULL_POINTER | FF_MKDIR;
	}

#ifdef FF_UNICODE_SUPPORT
	i = (FF_T_UINT16) wcslen(Path);
#else
	i = (FF_T_UINT16) strlen(Path);
#endif

	while(i != 0) {
		if(Path[i] == '\\' || Path[i] == '/') {
			break;
		}
		i--;
	}

#ifdef FF_UNICODE_SUPPORT
//	wcsncpy(DirName, (Path + i + 1), FF_MAX_FILENAME);
	DirName = (Path + i + 1);
#else
//	strncpy(DirName, (Path + i + 1), FF_MAX_FILENAME);
	DirName = (Path + i + 1);
#endif

	if(i == 0) {
		i = 1;
	}

	DirCluster = FF_FindDir(pIoman, Path, i, &Error);

	if(FF_isERR(Error)) {
		return Error;
	}

	if(!DirCluster) {
		return FF_ERR_DIR_INVALID_PATH | FF_MKDIR;
	}
	memset (&MyDir, '\0', sizeof MyDir);

	if(FF_FindEntryInDir(pIoman, DirCluster, DirName, 0x00, &MyDir, &Error)) {
		if(FF_isERR(Error)) {
			return Error;
		}
		return FF_ERR_DIR_OBJECT_EXISTS | FF_MKDIR;
	}

	if((FF_isERR(Error)) && FF_GETERROR (Error) != FF_ERR_DIR_END_OF_DIR) {
		return Error;
	}

#ifdef FF_UNICODE_SUPPORT
	wcsncpy(MyDir.FileName, DirName, FF_MAX_FILENAME);
#else
	strncpy(MyDir.FileName, DirName, FF_MAX_FILENAME);
#endif
	MyDir.Filesize		= 0;
	MyDir.Attrib		= FF_FAT_ATTR_DIR;
	MyDir.ObjectCluster	= FF_CreateClusterChain(pIoman, &Error);
	if(FF_isERR(Error)) {
		return Error;
	}
	if(!MyDir.ObjectCluster) {
		// Couldn't allocate any space for the dir!
		return FF_ERR_DIR_EXTEND_FAILED | FF_MKDIR;
	}
	Error = FF_ClearCluster(pIoman, MyDir.ObjectCluster);
	if(FF_isERR(Error)) {
		FF_lockFAT(pIoman);
		{
			FF_UnlinkClusterChain(pIoman, MyDir.ObjectCluster, 0);
		}
		FF_unlockFAT(pIoman);
		FF_FlushCache(pIoman);	// Don't override error;
		return Error;
	}

	Error = FF_CreateDirent(pIoman, DirCluster, &MyDir);

	if(FF_isERR(Error)) {
		FF_lockFAT(pIoman);
		{
			FF_UnlinkClusterChain(pIoman, MyDir.ObjectCluster, 0);
		}
		FF_unlockFAT(pIoman);
		FF_FlushCache(pIoman);	// Don't override error;
		return Error;
	}

	EntryBuffer[0] = '.';
	memset(EntryBuffer + 1, ' ', 10);
	memset(EntryBuffer + 11, 0, 21);

	FF_putChar(EntryBuffer,  (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB), FF_FAT_ATTR_DIR);
	FF_putShort(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_HIGH), (FF_T_UINT16)(MyDir.ObjectCluster >> 16));
	FF_putShort(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_LOW), (FF_T_UINT16) MyDir.ObjectCluster);
	FF_putLong(EntryBuffer,  (FF_T_UINT16)(FF_FAT_DIRENT_FILESIZE), 0);

	Error = FF_InitEntryFetch(pIoman, MyDir.ObjectCluster, &FetchContext);
	if(FF_isERR(Error)) {
		FF_lockFAT(pIoman);
		{
			FF_UnlinkClusterChain(pIoman, MyDir.ObjectCluster, 0);
		}
		FF_unlockFAT(pIoman);
		FF_FlushCache(pIoman);	// Don't override error;
		return Error;
	}

	Error = FF_PushEntryWithContext(pIoman, 0, &FetchContext, EntryBuffer);
	if(FF_isERR(Error)) {
		FF_lockFAT(pIoman);
		{
			FF_UnlinkClusterChain(pIoman, MyDir.ObjectCluster, 0);
		}
		FF_unlockFAT(pIoman);
		FF_FlushCache(pIoman);	// Don't override error;
		FF_CleanupEntryFetch(pIoman, &FetchContext);	// Don't override error!
		return Error;
	}

	EntryBuffer[0] = '.';
	EntryBuffer[1] = '.';
	memset(EntryBuffer + 2, ' ', 9);
	memset(EntryBuffer + 11, 0, 21);

	if(DirCluster == pIoman->pPartition->RootDirCluster) {
		DotDotCluster = 0;
	} else {
		DotDotCluster = DirCluster;
	}

	FF_putChar(EntryBuffer,  (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB), FF_FAT_ATTR_DIR);
	FF_putShort(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_HIGH), (FF_T_UINT16)(DotDotCluster >> 16));
	FF_putShort(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_CLUS_LOW), (FF_T_UINT16) DotDotCluster);
	FF_putLong(EntryBuffer,  (FF_T_UINT16)(FF_FAT_DIRENT_FILESIZE), 0);

	//FF_PushEntry(pIoman, MyDir.ObjectCluster, 1, EntryBuffer);
	Error = FF_PushEntryWithContext(pIoman, 1, &FetchContext, EntryBuffer);
	if(FF_isERR(Error)) {
		FF_lockFAT(pIoman);
		{
			FF_UnlinkClusterChain(pIoman, MyDir.ObjectCluster, 0);
		}
		FF_unlockFAT(pIoman);
		FF_FlushCache(pIoman);	// Don't override error;
		FF_CleanupEntryFetch(pIoman, &FetchContext);	// Don't override error!
		return Error;
	}
	Error = FF_CleanupEntryFetch(pIoman, &FetchContext);
	if(FF_isERR(Error)) {
		FF_FlushCache(pIoman);	// Ensure dir was flushed to the disk!	// Don't override error;
		return Error;
	}

	Error = FF_FlushCache(pIoman);	// Ensure dir was flushed to the disk!
	if(FF_isERR(Error)) {
		return Error;
	}

	return FF_ERR_NONE;
}



FF_ERROR FF_RmLFNs(FF_IOMAN *pIoman, FF_T_UINT16 usDirEntry, FF_FETCH_CONTEXT *pContext) {

	FF_ERROR	Error;
	FF_T_UINT8	EntryBuffer[32];

	if(usDirEntry == 0) {
		return FF_ERR_NONE;
	}

	usDirEntry--;

	do {
		Error = FF_FetchEntryWithContext(pIoman, usDirEntry, pContext, EntryBuffer);
		if(FF_isERR(Error)) {
			return Error;
		}

		if(FF_getChar(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB)) == FF_FAT_ATTR_LFN) {
			FF_putChar(EntryBuffer, (FF_T_UINT16) 0, (FF_T_UINT8) 0xE5);
			Error = FF_PushEntryWithContext(pIoman, usDirEntry, pContext, EntryBuffer);
			if(FF_isERR(Error)) {
				return Error;
			}
		}

		if(usDirEntry == 0) {
			break;
		}
		usDirEntry--;
	}while(FF_getChar(EntryBuffer, (FF_T_UINT16)(FF_FAT_DIRENT_ATTRIB)) == FF_FAT_ATTR_LFN);

	return FF_ERR_NONE;
}
