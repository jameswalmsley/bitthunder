/*
 * Copyright (c) 2d3D, Inc.
 * Written by Abraham vd Merwe <abraham@2d3d.co.za>
 * All rights reserved.
 *
 * Renamed to flashcp.c to avoid conflicts with fcp from fsh package
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of other contributors
 *	  may be used to endorse or promote products derived from this software
 *	  without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define PROGRAM_NAME "flashtool"

#include <bitthunder.h>

#include <stdio.h>
#include <string.h>

#define KB(x) ((x) / 1024)
#define PERCENTAGE(x,total) (((x) * 100) / (total))

/* size of read/write buffer */
#define BUFSIZE (1024 * 1024 * 2)

/* cmd-line flags */
#define FLAG_NONE		0x00
#define FLAG_VERIFY 	0x01
#define FLAG_HELP		0x02
#define FLAG_FILENAME	0x04
#define FLAG_DEVICE		0x08
#define FLAG_ERASE      0x10
#define FLAG_PROGRAM    0x20
#define FLAG_BULKERASE  0x40
#define FLAG_READ		0x80

BT_HANDLE hStdout;

static BT_ERROR showusage(BT_BOOL error)
{
	bt_fprintf (hStdout,
			"\n"
			PROGRAM_NAME " -  (c) 2013 by Riegl Laser Measurement Systems GmbH\n"
			"                      by Michael Daniel \n"
			"                      based on Abraham van der Merwe's flashcp\n"
			"\n"
			"usage: " PROGRAM_NAME "      <filename> [ <device> ]     ... short version of -epv\n"
			"       " PROGRAM_NAME " -h                               ... show help\n"
			"       " PROGRAM_NAME " -epv <filename> [ <device> ]     ... same as first line\n"
			"       " PROGRAM_NAME " -b              [ <device> ]     ... bulk erase of <device>\n"
			"       " PROGRAM_NAME " -e   <filename> [ <device> ]     ... sector erase of device with size of <filename>\n"
			"       " PROGRAM_NAME " -p   <filename> [ <device> ]     ... program device\n"
			"       " PROGRAM_NAME " -v   <filename> [ <device> ]     ... verify if device contains <filename>\n"
			"       " PROGRAM_NAME " -r   <filename> [ <device> ]     ... reads data from <device> to <filename>\n"
			"\n"
			"       -h | --help      Show this help message\n"
			"       -v | --verify    check if device has same content as filename\n"
			"       -e | --erase     bulk erase if no file specified, otherwise erases size of specified file\n"
			"       -p | --program   only program device (no erase will be made - check if you need -e option before!)\n"
			"       -r | --read      read content of flash into file specified by <filename> (other flags are ignored!)\n"
			"       -b | --bulk		 Bulk erase on the specified device (the whole device is erased)\n"
			"       <filename>       File which you want to copy to flash\n"
			"       <device>         Flash device to write to (e.g. /dev/mtd0, /dev/mtd1, etc.) [optional]\n"
			"\n"
			"Note: if no device is specified, /dev/mtd0 is assumed.\n"
			);

	return (error ? BT_ERR_GENERIC : BT_ERR_NONE);
}

static BT_HANDLE safe_open (const char *pathname,BT_u32 flags)
{
	BT_HANDLE fd;
	BT_ERROR Error;
	int retry = 5;

	do {
		BT_ThreadSleep(500);
		fd = BT_Open(pathname,flags, &Error);
		retry--;
	} while (fd == NULL && retry > 0);

	if (fd == NULL)
	{
		bt_fprintf (hStdout,"Error while trying to open %s",pathname);
		if (flags & BT_GetModeFlags("rw"))
			bt_fprintf (hStdout," for read/write access");
		else if (flags & BT_GetModeFlags("r"))
			bt_fprintf (hStdout," for read access");
		else if (flags & BT_GetModeFlags("w"))
			bt_fprintf (hStdout," for write access");
		return NULL;
	}

	return fd;
}

static BT_ERROR safe_read (BT_HANDLE fd, const char *filename,void *buf, BT_u32 count, BT_BOOL verbose)
{
	BT_u64 result;
	BT_ERROR Error;

	result = BT_Read(fd, 0, count, buf, &Error);

	if (count != result)
	{
		if (verbose) bt_fprintf (hStdout,"\n");
		if (result == 0)
		{
			bt_fprintf (hStdout,"Error while reading data from %s\n",filename);
			return BT_ERR_GENERIC;
		}
		bt_fprintf (hStdout,"Short read count returned while reading from %s\n",filename);
		return BT_ERR_GENERIC;
	}
	return BT_ERR_NONE;
}

static BT_ERROR safe_write(BT_HANDLE fd, const char *filename, void *buf, BT_u32 count, BT_BOOL verbose)
{
	BT_u64 result;
	BT_ERROR Error;

	result = BT_Write(fd, 0, count, buf, &Error);

	if(count != result)
	{
		if (verbose) bt_fprintf (hStdout, "\n");
		if (result == 0)
		{
			bt_fprintf (hStdout,"Error while writing data to %s\n", filename);
			return BT_ERR_GENERIC;
		}
		bt_fprintf (hStdout, "Short write count returned while writing to %s\n", filename);
		return BT_ERR_GENERIC;
	}
	return BT_ERR_NONE;
}

static BT_ERROR safe_rewind (BT_HANDLE fd, const char *filename)
{
	if(BT_Seek(fd, 0L, BT_SEEK_SET) != BT_ERR_NONE)
	{
		bt_fprintf (hStdout, "Error while seeking to start of %s\n",filename);
		return BT_ERR_GENERIC;
	}
	return BT_ERR_NONE;
}

/******************************************************************************/

static BT_HANDLE dev_fd = NULL;
static BT_HANDLE fil_fd = NULL;

static void cleanup (void)
{
	if(dev_fd != NULL) BT_CloseHandle(dev_fd);
	if(fil_fd != NULL) BT_CloseHandle(fil_fd);
}

static int bt_flashtool(BT_HANDLE hShell, int argc, char **argv)
{
	const char *filename = NULL,*device = NULL;
	int flags = FLAG_NONE;
	BT_u64 i;
	BT_u64 size, written;
	BT_MTD_ERASE_INFO erase;
	unsigned char * src = NULL;
	unsigned char * dest = NULL;
    char dev_t[] = "/dev/mtd0\0";
    BT_ERROR Error;
    int optind = 1;

    hStdout = BT_ShellGetStdout(hShell);
    device = dev_t;

	/*********************
	 * parse cmd-line
	 *****************/

    for(i=0; i<argc; i++) {
    	if(strlen(argv[i]) > 1 && argv[i][0] == '-' && argv[i][1] != '-')
    	{
    		int j;
    		for(j=1; j<strlen(argv[i]); j++)
    		{
    			if(argv[i][j] == 'h') {
    				flags |= FLAG_HELP;
    			} else if(argv[i][j] == 'v') {
    				flags |= FLAG_VERIFY;
    			} else if(argv[i][j] == 'e') {
    				flags |= FLAG_ERASE;
    			} else if(argv[i][j] == 'p') {
    				flags |= FLAG_PROGRAM;
    			} else if(argv[i][j] == 'r') {
    				flags |= FLAG_READ;
    			} else if(argv[i][j] == 'b') {
    				flags |= FLAG_BULKERASE;
    			} else {
    				flags |= FLAG_HELP;
    			}
    		}
    		optind++;
    	}
    	else if( /* strcmp(argv[i],"-h") == 0 || */strcmp(argv[i],"--help") == 0) {
    		flags |= FLAG_HELP;
    		optind++;
    	}
    	else if( /* strcmp(argv[i],"-v") == 0 || */strcmp(argv[i],"--verify") == 0) {
    		flags |= FLAG_VERIFY;
    		optind++;
    	}
    	else if(/*strcmp(argv[i],"-e") == 0 || */strcmp(argv[i],"--erase") == 0) {
    		flags |= FLAG_ERASE;
    		optind++;
    	}
    	else if(/*strcmp(argv[i],"-p") == 0 || */strcmp(argv[i],"--program") == 0) {
    		flags |= FLAG_PROGRAM;
    		optind++;
    	}
    	else if(/*strcmp(argv[i],"-r") == 0 || */strcmp(argv[i],"--read") == 0) {
    		flags |= FLAG_READ;
    		optind++;
    	}
    	else if(/*strcmp(argv[i],"-r") == 0 || */strcmp(argv[i],"--bulk") == 0) {
			flags |= FLAG_BULKERASE;
			optind++;
		}
    }

    if((flags & FLAG_HELP) || argc <= 1)
	{
		showusage(BT_FALSE);
		return BT_ERR_NONE;
	}

    if (optind+2 == argc)
    {
        if(flags == FLAG_NONE)
        {
            flags |= FLAG_ERASE;
            flags |= FLAG_PROGRAM;
            flags |= FLAG_VERIFY;
            flags |= FLAG_DEVICE;
            flags |= FLAG_FILENAME;
        }

		// first is filename, second is device name
		flags |= FLAG_FILENAME;
		filename = argv[optind];

		flags |= FLAG_DEVICE;
		device = argv[optind+1];
	}
	else if(optind+1 == argc)   // check if it must be a device name or a file name
    {
		if(flags & FLAG_BULKERASE)
		{
			flags |= FLAG_DEVICE;
			device = argv[optind];
		}
		else if(flags - FLAG_ERASE == FLAG_NONE)
		{
			flags |= FLAG_FILENAME;
			filename = argv[optind];
		}
		else
		{
			if(flags == FLAG_NONE)
			{
				flags |= FLAG_FILENAME;
				filename = argv[optind];

				flags |= FLAG_ERASE;
				flags |= FLAG_PROGRAM;
				flags |= FLAG_VERIFY;
			}
			else
			{
                // chose standard device, no bulk erase
                flags |= FLAG_FILENAME;
                filename = argv[optind];
                flags |= FLAG_DEVICE;
                device = dev_t;
                bt_fprintf (hStdout, "Warning: didn't specify device name, assuming '%s'.\n",device);
			}
		}
    }
    else if(optind == argc)
    {
        if(flags - FLAG_ERASE == FLAG_NONE || (flags & FLAG_BULKERASE))
        {
        	flags |= FLAG_DEVICE;
            flags |= FLAG_BULKERASE;
            device = dev_t;
            bt_fprintf (hStdout, "Warning: didn't specify device name for bulk erase, assuming '%s'.\n",device);
        }
    }

    //bt_fprintf(hStdout, "Device: %s, File: %s, Flags=%s%s%s\n", device, (filename == NULL) ? "" : filename, (flags|FLAG_ERASE) ? " -e " : "", (flags|FLAG_PROGRAM) ? " -p " : "", (flags|FLAG_VERIFY) ? " -v " : ""   );

	/* get some info about the flash device */
	dev_fd = safe_open (device , 0);
	if(dev_fd == NULL)
		goto FLASHTOOL_ERROR_EXIT;

	BT_MTD_USER_INFO info;
	Error = BT_MTD_GetUserInfo(dev_fd, &info);
	if (Error != BT_ERR_NONE)
	{
		bt_fprintf (hStdout, "This doesn't seem to be a valid MTD flash device!\n");
		goto FLASHTOOL_ERROR_EXIT;
	}


    if(flags & FLAG_READ)
    {
    	fil_fd = safe_open (filename, BT_GetModeFlags("w"));
    	if (fil_fd == NULL)
    		goto FLASHTOOL_ERROR_EXIT;

    	if(safe_rewind(fil_fd, filename) != BT_ERR_NONE)
    		goto FLASHTOOL_ERROR_EXIT;

    	if(safe_rewind(dev_fd, device) != BT_ERR_NONE)
    		goto FLASHTOOL_ERROR_EXIT;

    	size = info.size;
    	if(info.size < BUFSIZE) {
    		src = BT_kMalloc(info.size);
    		i = info.size;
    	}
    	else {
    		src = BT_kMalloc(BUFSIZE);
    		i = BUFSIZE;
    	}

    	written = 0;
    	bt_fprintf (hStdout, "Reading data to file: 0k/%lluk (0%%)", KB(info.size));
    	while (size)
    	{
    		if(size < BUFSIZE) i=size;
    		bt_fprintf (hStdout,
				"\rReading data to file: %lluk/%lluk (%llu%%)",
				KB (written + i),
				KB (info.size),
				PERCENTAGE ((written + i),info.size));

    		/* read from device */
    		if(safe_read (dev_fd, device, src, i, BT_TRUE) != BT_ERR_NONE)
    			goto FLASHTOOL_ERROR_EXIT;

    		/* write to file */
    		if(safe_write(fil_fd, filename, src, i, BT_TRUE) != BT_ERR_NONE)
    			goto FLASHTOOL_ERROR_EXIT;

    		written += i;
    		size -= i;
    	}
    	bt_fprintf (hStdout,
				"\rReading data to file: %lluk/%lluk (100%%)      \n",
				KB (info.size),
				KB (info.size));

    	goto FLASHTOOL_SUCCESS_EXIT;
    }


	if(flags & FLAG_BULKERASE)
    {
        // erase the whole device (should be faster than erasing sector by sector...)
		erase.addr = 0;
		erase.len = info.size;

		bt_fprintf (hStdout, "Starting erasing whole device with size %llu Kbytes... ",KB(info.size));

        if(BT_MTD_Erase(dev_fd, &erase) != BT_ERR_NONE)
        {
        	bt_fprintf (hStdout,
                       "Error.\n");
        	goto FLASHTOOL_ERROR_EXIT;
        }
        bt_fprintf (hStdout, " done.\n");
        goto FLASHTOOL_SUCCESS_EXIT;
    }

	/* get some info about the file we want to copy */
	fil_fd = safe_open (filename, BT_GetModeFlags("r"));
	if(fil_fd == NULL)
		goto FLASHTOOL_ERROR_EXIT;

	BT_HANDLE hInode =BT_GetInode(filename, &Error);
	if (hInode == NULL)
	{
		bt_fprintf (hStdout, "Error while trying to get the file inode handle of %s\n",filename);
		goto FLASHTOOL_ERROR_EXIT;
	}
	BT_INODE inode;
	Error = BT_ReadInode(hInode, &inode);
	if(Error == BT_ERR_NONE) {
		BT_CloseHandle(hInode);
	} else {
		BT_CloseHandle(hInode);
		bt_fprintf (hStdout, "Error while trying to get the file inode struct of %s\n",filename);
		goto FLASHTOOL_ERROR_EXIT;
	}

	/* does it fit into the device/partition? */
	if (inode.ullFileSize > info.size)
	{
		bt_fprintf (hStdout, "%s won't fit into %s!\n",filename,device);
		goto FLASHTOOL_ERROR_EXIT;
	}

	/*****************************************************
	 * erase enough blocks so that we can write the file *
	 *****************************************************/

    if(flags & FLAG_ERASE)
    {
        erase.addr = 0;
        if(info.erasesize == 0) {
        	goto FLASHTOOL_ERROR_EXIT;
        }

        erase.len = (inode.ullFileSize + info.erasesize - 1) / info.erasesize;
        erase.len *= info.erasesize;

        int blocks = erase.len / info.erasesize;
        erase.len = info.erasesize;

        for (i = 1; i <= blocks; i++)
        {
        	bt_fprintf (hStdout, "\rErasing blocks: %llu/%d (%d%%)",i,blocks,PERCENTAGE ((int)i,blocks));

            Error = BT_MTD_Erase(dev_fd, &erase);

            if(Error != BT_ERR_NONE) {
            	bt_fprintf (hStdout,"\n");
            	bt_fprintf (hStdout,
						"Error while erasing blocks 0x%.8x-0x%.8x on %s\n",
						(unsigned int) erase.addr,(unsigned int) (erase.addr + erase.len), device);
				goto FLASHTOOL_ERROR_EXIT;
            }

            erase.addr += info.erasesize;
        }
        bt_fprintf (hStdout, "\rErasing blocks: %d/%d (100%%)\n", blocks, blocks);
    }




	/**********************************
	 * write the entire file to flash *
	 **********************************/

    if(flags & FLAG_PROGRAM)
    {
    	bt_fprintf (hStdout, "Writing data: 0k/%lluk (0%%)",KB (inode.ullFileSize));
        size = inode.ullFileSize;

        if(size < BUFSIZE)
        {
        	src = BT_kMalloc(size);
			i = size;
        } else {
        	src = BT_kMalloc(BUFSIZE);
        	i = BUFSIZE;
        }


        if(src == NULL) {
        	goto FLASHTOOL_ERROR_EXIT;
        }
        written = 0;
        while (size)
        {
            if (size < BUFSIZE) i = size;
            bt_fprintf (hStdout, "\rWriting data: %lluk/%lluk (%lu%%)",
                KB (written + i),
                KB (inode.ullFileSize),
                PERCENTAGE ((unsigned long)(written + i),inode.ullFileSize)

            );

            /* read from filename */
            if(safe_read (fil_fd, filename, src, i, BT_TRUE) != BT_ERR_NONE)
            	goto FLASHTOOL_ERROR_EXIT;

            /* write to device */
            //result = BT_Write(dev_fd, 0, i, src, &Error);
            if(safe_write(dev_fd, device, src, i, BT_TRUE) != BT_ERR_NONE)
            	goto FLASHTOOL_ERROR_EXIT;

            written += i;
            size -= i;
        }
        bt_fprintf (hStdout,
                "\rWriting data: %lluk/%lluk (100%%)     \n",
                KB (inode.ullFileSize),
                KB (inode.ullFileSize));
    }

	/**********************************
	 * verify that flash == file data *
	 **********************************/

    if(flags & FLAG_VERIFY)
    {
        if(safe_rewind (fil_fd, filename) != BT_ERR_NONE)
        	goto FLASHTOOL_ERROR_EXIT;

        if(safe_rewind (dev_fd, device) != BT_ERR_NONE)
        	goto FLASHTOOL_ERROR_EXIT;

        size = inode.ullFileSize;

        if(size < BUFSIZE)
		{
        	if(src != NULL)
        		BT_kFree(src);

        	src = BT_kMalloc(size);
			dest = BT_kMalloc(size);
			i = size;
		}
        else
		{
        	if(src != NULL)
        		BT_kFree(src);

			src = BT_kMalloc(BUFSIZE);
			dest = BT_kMalloc(BUFSIZE);
			i = BUFSIZE;
		}

        written = 0;
        bt_fprintf (hStdout, "Verifying data: 0k/%lluk (0%%)",KB (inode.ullFileSize));

        while (size)
        {
            if (size < BUFSIZE) i = size;
            bt_fprintf (hStdout,
                    "\rVerifying data: %lluk/%lluk (%lu%%)",
                    KB (written + i),
                    KB (inode.ullFileSize),
                    PERCENTAGE ((written + i),inode.ullFileSize));

            /* read from filename */
            if(safe_read (fil_fd, filename, src, i, BT_TRUE) != BT_ERR_NONE)
            	goto FLASHTOOL_ERROR_EXIT;

            /* read from device */
            if(safe_read (dev_fd, device, dest, i, BT_TRUE) != BT_ERR_NONE)
            	goto FLASHTOOL_ERROR_EXIT;

            /* compare buffers */
            if (memcmp (src, dest , i))
            {
            	bt_fprintf (hStdout,
                        "File does not seem to match flash data. First mismatch at 0x%.8x-0x%.8x\n",
                        written, written + i);
                goto FLASHTOOL_ERROR_EXIT;
            }

            written += i;
            size -= i;
        }
        bt_fprintf (hStdout,
                "\rVerifying data: %lluk/%lluk (100%%)      \n",
                KB (inode.ullFileSize),
                KB (inode.ullFileSize));
    }

FLASHTOOL_SUCCESS_EXIT:
	if(src)
		BT_kFree(src);
	if(dest)
		BT_kFree(dest);

    cleanup();
    return BT_ERR_NONE;

FLASHTOOL_ERROR_EXIT:
	if(src)
		BT_kFree(src);
	if(dest)
		BT_kFree(dest);

	cleanup();
    return BT_ERR_GENERIC;
}

BT_SHELL_COMMAND_DEF oCommand = {
		.szpName	= "flashtool",
		.pfnCommand = bt_flashtool,
};

