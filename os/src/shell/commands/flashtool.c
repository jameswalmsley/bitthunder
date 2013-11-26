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
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

/* for debugging purposes only */
#ifdef DEBUG
#undef DEBUG
#define DEBUG(fmt,args...) { log_printf (LOG_ERROR,"%d: ",__LINE__); log_printf (LOG_ERROR,fmt,## args); }
#else
#undef DEBUG
#define DEBUG(fmt,args...)
#endif

#define KB(x) ((x) / 1024)
#define PERCENTAGE(x,total) (((x) * 100) / (total))

/* size of read/write buffer */
#define BUFSIZE (10 * 1024)

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

/* error levels */
#define LOG_NORMAL	1
#define LOG_ERROR	2

BT_HANDLE hStdout;

/*
static void log_printf (int level,const char *fmt, ...)
{
	//FILE *fp = level == LOG_NORMAL ? stdout : stderr;
	va_list ap;
	va_start (ap,fmt);
	bt_kvprintf(fmt, bt_fputc, hStdout, 10, ap);
	//vfprintf (fp,fmt,ap);
	va_end (ap);
	//fflush (fp);
}
*/

static void showusage(BT_BOOL error)
{
	bt_fprintf (hStdout,
			"\n"
			"flashtool  - (c) 2013 by Riegl Laser Measurement Systems GmbH\n"
			"                      by Michael Daniel \n"
			"                      based on Abraham van der Merwe's flashcp"
			"\n"
			"usage: %1$s      <filename> [ <device> ]     ... short version of -epv\n"
			"       %1$s -h                               ... show help\n"
			"       %1$s -epv <filename> [ <device> ]     ... same as first line\n"
			"       %1$s -e              [ <device> ]     ... bulk erase of device\n"
			"       %1$s -e   <filename> [ <device> ]     ... sector erase of device with size of <filename>\n"
			"       %1$s -p   <filename> [ <device> ]     ... program device\n"
			"       %1$s -v   <filename> [ <device> ]     ... verify if device contains <filename>\n"
			"       %1$s -r   <filename> [ <device> ]     ... reads data from <device> to <filename>\n"
			"\n"
			"       -h | --help      Show this help message\n"
			"       -v | --verify    check if device has same content as filename\n"
			"       -e | --erase     bulk erase if no file specified, otherwise erases size of specified file\n"
			"       -p | --program   only program device (no erase will be made - check if you need -e option before!)\n"
			"       -r | --read      read content of flash into file specified by <filename> (other flags are ignored!)\n"
			"       <filename>       File which you want to copy to flash\n"
			"       <device>         Flash device to write to (e.g. /dev/mtd0, /dev/mtd1, etc.) [optional]\n"
			"\n"
			"Note: if no device is specified, /dev/mtd0 is assumed.\n",
			PROGRAM_NAME);

	exit (error ? EXIT_FAILURE : EXIT_SUCCESS);
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
		bt_fprintf (hStdout,"While trying to open %s",pathname);
		if (flags & BT_GetModeFlags("rw"))
			bt_fprintf (hStdout," for read/write access");
		else if (flags & BT_GetModeFlags("r"))
			bt_fprintf (hStdout," for read access");
		else if (flags & BT_GetModeFlags("w"))
			bt_fprintf (hStdout," for write access");
		bt_fprintf (hStdout,": %m\n");
		exit (EXIT_FAILURE);
	}

	return fd;
}

static void safe_read (BT_HANDLE fd, const char *filename,void *buf, BT_u32 count, BT_BOOL verbose)
{
	BT_u64 result;
	BT_ERROR Error;

	//result = read (fd,buf,count);
	result = BT_Read(fd, 0, count, buf, &Error);

	if (count != result)
	{
		if (verbose) bt_fprintf (hStdout,"\n");
		if (result == 0)
		{
			bt_fprintf (hStdout,"While reading data from %s: %m\n",filename);
			exit (EXIT_FAILURE);
		}
		bt_fprintf (hStdout,"Short read count returned while reading from %s\n",filename);
		exit (EXIT_FAILURE);
	}
}

static void safe_write(BT_HANDLE fd, const char *filename, void *buf, BT_u32 count, BT_BOOL verbose)
{
	BT_u64 result;
	BT_ERROR Error;

	//result = write(fd, buf, count);
	result = BT_Write(fd, 0, count, buf, &Error);

	if(count != result)
	{
		if (verbose) bt_fprintf (hStdout, "\n");
		if (result == 0)
		{
			bt_fprintf (hStdout,"While writing data to %s: %m\n", filename);
			exit (EXIT_FAILURE);
		}
		bt_fprintf (hStdout, "Short write count returned while writing to %s\n", filename);
		exit (EXIT_FAILURE);
	}
}

static void safe_rewind (BT_HANDLE fd, const char *filename)
{
	if(BT_Seek(fd, 0L, BT_SEEK_SET) != BT_ERR_NONE)
	//if (lseek (fd,0L,SEEK_SET) < 0)
	{
		bt_fprintf (hStdout, "While seeking to start of %s: %m\n",filename);
		exit (EXIT_FAILURE);
	}
}

/******************************************************************************/

//static int dev_fd = -1,fil_fd = -1;
static BT_HANDLE dev_fd = NULL;
static BT_HANDLE fil_fd = NULL;

static void cleanup (void)
{
	if(dev_fd != NULL) BT_CloseHandle(dev_fd);
	if(fil_fd != NULL) BT_CloseHandle(fil_fd);
}

static int bt_flashtool(BT_HANDLE hShell, int argc,char **argv)
{
	const char *filename = NULL,*device = NULL;
	int i,flags = FLAG_NONE;
	BT_u64 result;
	BT_u32 size, written;
	BT_MTD_ERASE_INFO erase;
	unsigned char src[BUFSIZE], dest[BUFSIZE];
    char dev_t[] = "/dev/mtd0\0";
    unsigned long filesize = 0;
    BT_ERROR Error;
    int optind = 0;

    hStdout = BT_ShellGetStdout(hShell);
    device = dev_t;

	/*********************
	 * parse cmd-line
	 *****************/

    for(i=0; i<argc; i++) {
    	if( strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"--help") == 0) {
    		flags |= FLAG_HELP;
    		optind++;
    	}
    	else if( strcmp(argv[i],"-v") == 0 || strcmp(argv[i],"--verify") == 0) {
    		flags |= FLAG_VERIFY;
    		optind++;
    	}
    	else if(strcmp(argv[i],"-e") == 0 || strcmp(argv[i],"--erase") == 0) {
    		flags |= FLAG_ERASE;
    		optind++;
    	}
    	else if(strcmp(argv[i],"-p") == 0 || strcmp(argv[i],"--program") == 0) {
    		flags |= FLAG_PROGRAM;
    		optind++;
    	}
    	else if(strcmp(argv[i],"-r") == 0 || strcmp(argv[i],"--read") == 0) {
    		flags |= FLAG_READ;
    		optind++;
    	}
    }

    if((flags & FLAG_HELP) || argc <= 1)
	{
		showusage(BT_FALSE);
	}

    /*

	for (;;) {
		int option_index = 0;
		static const char *short_options = "hvepr";
		static const struct option long_options[] = {
			{"help", no_argument, 0, 'h'},
			{"verify", no_argument, 0, 'v'},
			{"erase", no_argument, 0, 'e'},
			{"program", no_argument, 0, 'p'},
			{"read", no_argument, 0, 'r'},
			{0, 0, 0, 0},
		};

		int c = getopt_long(argc, argv, short_options,
				long_options, &option_index);
		if (c == EOF) {
			break;
		}

		switch (c) {
			case 'h':
				flags |= FLAG_HELP;
				DEBUG("Got FLAG_HELP\n");
				break;
			case 'v':
				flags |= FLAG_VERIFY;
				DEBUG("Got FLAG_VERIFY\n");
				break;
            case 'e':
                flags |= FLAG_ERASE;
                DEBUG("Got FLAG_ERASE\n");
                break;
            case 'p':
                flags |= FLAG_PROGRAM;
                DEBUG("Got FLAG_PRGORAM\n");
                break;
            case 'r':
            	flags |= FLAG_READ;
            	DEBUG("Got FLAG_READ\n");
            	break;
			default:
				DEBUG("Unknown parameter: %s\n",argv[option_index]);
				showusage(true);
		}
	}

	if((flags & FLAG_HELP) || argc == 1)
    {
        showusage(0);
    }

    */

    if (optind+2 == argc)
    {
        if(flags == FLAG_NONE)
        {
            // first is filename, second is device name
            flags |= FLAG_FILENAME;
            filename = argv[optind];
            DEBUG("Got filename: %s\n",filename);

            flags |= FLAG_DEVICE;
            device = argv[optind+1];
            DEBUG("Got device: %s\n",device);

            flags |= FLAG_ERASE;
            flags |= FLAG_PROGRAM;
            flags |= FLAG_VERIFY;
            flags |= FLAG_DEVICE;
            flags |= FLAG_FILENAME;
        }
        else
        {
            // first is filename, second is device name
            flags |= FLAG_FILENAME;
            filename = argv[optind];
            DEBUG("Got filename: %s\n",filename);

            flags |= FLAG_DEVICE;
            device = argv[optind+1];
            DEBUG("Got device: %s\n",device);
        }

	}
	else if(optind+1 == argc)   // check if it must be a device name or a file name
    {
        if(flags - FLAG_ERASE == FLAG_NONE)
        {
            // just do an erase on the specified device
            flags |= FLAG_BULKERASE;
            flags |= FLAG_DEVICE;
            device = argv[optind];
        }
        else
        {
            if (flags == FLAG_NONE)
            {
                filename = argv[1];
                DEBUG("Got filename: %s\n",filename);
                flags |= FLAG_ERASE;
                flags |= FLAG_PROGRAM;
                flags |= FLAG_VERIFY;
                flags |= FLAG_DEVICE;
                flags |= FLAG_FILENAME;
            }
            else
            {
                // chose standard device, no bulk erase
                flags |= FLAG_FILENAME;
                filename = argv[optind];
                DEBUG("Got filename: %s\n",filename);
                flags |= FLAG_DEVICE;
                device = dev_t;
                bt_fprintf (hStdout, "Warning: didn't specify device name, assuming '%s'.\n",device);
                DEBUG("Got device: %s\n",device);
            }

        }


    }
    else if(optind == argc)
    {
        if(flags - FLAG_ERASE == FLAG_NONE)
        {
            flags |= FLAG_BULKERASE;
        }
    }


	atexit (cleanup);

	/* get some info about the flash device */
	dev_fd = safe_open (device , BT_GetModeFlags("rw"));
	BT_MTD_USER_INFO info;
	Error = BT_MTD_GetUserInfo(dev_fd, &info);
	if (Error != BT_ERR_NONE)
	{
		DEBUG("mtd.size: %m\n");
		bt_fprintf (hStdout, "This doesn't seem to be a valid MTD flash device!\n");
		exit (EXIT_FAILURE);
	}


    if(flags & FLAG_READ)
    {
    	fil_fd = safe_open (filename, BT_GetModeFlags("w"));
    	safe_rewind(fil_fd, filename);
    	safe_rewind(dev_fd, device);
    	size = info.size;
    	i = BUFSIZE;
    	written = 0;
    	bt_fprintf (hStdout, "Reading data to file: 0k/%luk (0%%)", KB(info.size));
    	while (size)
    	{
    		if(size < BUFSIZE) i=size;
    		bt_fprintf (hStdout,
				"\rReading data to file: %dk/%luk (%d%%)",
				KB (written + i),
				KB (info.size),
				PERCENTAGE ((unsigned long)(written + i),info.size));

    		/* read from device */
    		safe_read (dev_fd, device, src, i, BT_TRUE);

    		/* write to file */
    		safe_write(fil_fd, filename, src, i, BT_TRUE);

    		written += i;
    		size -= i;
    	}
    	bt_fprintf (hStdout,
				"\rReading data to file: %luk/%luk (100%%)      \n",
				KB (info.size),
				KB (info.size));
    	DEBUG("Read and wrote %d / %luk bytes\n", written, info.size);
    	exit(EXIT_SUCCESS);
    }


	if(flags & FLAG_BULKERASE)
    {
        // erase the whole device (should be faster than erasing sector by sector...)
		erase.addr = 0;
		erase.len = info.size;

		bt_fprintf (hStdout, "Starting erasing whole device with size %d Kbytes... ",KB(info.size));

        if(BT_MTD_Erase(dev_fd, &erase) != BT_ERR_NONE)
        {
        	bt_fprintf (hStdout,
                       "Error.\n");
            exit(EXIT_FAILURE);
        }
        bt_fprintf (hStdout, " done.\n");
        exit (EXIT_SUCCESS);
    }

	/* get some info about the file we want to copy */
	fil_fd = safe_open (filename, BT_GetModeFlags("r"));
	BT_HANDLE hInode =BT_GetInode(filename, &Error);
	if (hInode == NULL)
	{
		bt_fprintf (hStdout, "While trying to get the file inode handle of %s: %m\n",filename);
		exit (EXIT_FAILURE);
	}
	BT_INODE inode;
	Error = BT_ReadInode(hInode, &inode);
	if(Error == BT_ERR_NONE) {
		BT_CloseHandle(hInode);
	} else {
		BT_CloseHandle(hInode);
		bt_fprintf (hStdout, "While trying to get the file inode struct of %s: %m\n",filename);
		exit (EXIT_FAILURE);
	}

	/* does it fit into the device/partition? */
	if (inode.ullFileSize > info.size)
	{
		bt_fprintf (hStdout, "%s won't fit into %s!\n",filename,device);
		exit (EXIT_FAILURE);
	}

	/*****************************************************
	 * erase enough blocks so that we can write the file *
	 *****************************************************/

    filesize = inode.ullFileSize;


    if(flags & FLAG_ERASE)
    {
        erase.addr = 0;
        erase.len = (inode.ullFileSize +info.erasesize - 1) / info.erasesize;
        erase.len *= info.erasesize;

        int blocks = erase.len / info.erasesize;
        erase.len = info.erasesize;
        bt_fprintf (hStdout, "Erasing blocks: 0/%d (0%%)",blocks);
        for (i = 1; i <= blocks; i++)
        {
        	bt_fprintf (hStdout, "\rErasing blocks: %d/%d (%d%%)",i,blocks,PERCENTAGE (i,blocks));

            /*
            if (ioctl (dev_fd,MEMERASE,&erase) < 0)
            {
                log_printf (LOG_NORMAL,"\n");
                log_printf (LOG_ERROR,
                        "While erasing blocks 0x%.8x-0x%.8x on %s: %m\n",
                        (unsigned int) erase.start,(unsigned int) (erase.start + erase.length),device);
                exit (EXIT_FAILURE);
            }
            */

            Error = BT_MTD_Erase(dev_fd, &erase);
            if(Error != BT_ERR_NONE) {
            	bt_fprintf (hStdout,"\n");
            	bt_fprintf (hStdout,
						"While erasing blocks 0x%.8x-0x%.8x on %s: %m\n",
						(unsigned int) erase.addr,(unsigned int) (erase.addr + erase.len), device);
				exit (EXIT_FAILURE);
            }

            erase.addr += info.erasesize;
        }
        bt_fprintf (hStdout, "\rErasing blocks: %d/%d (100%%)\n", blocks, blocks);
        DEBUG("Erased %u / %luk bytes\n",erase.len, inode.ullFileSize);
    }


	/**********************************
	 * write the entire file to flash *
	 **********************************/

    if(flags & FLAG_PROGRAM)
    {
    	bt_fprintf (hStdout, "Writing data: 0k/%luk (0%%)",KB (inode.ullFileSize));
        size = inode.ullFileSize;
        i = BUFSIZE;
        written = 0;
        while (size)
        {
            if (size < BUFSIZE) i = size;
            bt_fprintf (hStdout, "\rWriting data: %luk/%luk (%lu%%)",
                KB (written + i),
                KB (filesize),
                PERCENTAGE ((unsigned long)(written + i),filesize)

            );

            /* read from filename */
            safe_read (fil_fd, filename, src, i, BT_TRUE);

            /* write to device */

            //result = write (dev_fd,src,i);

            result = BT_Write(dev_fd, 0, i, src, &Error);
            if (i != result)
            {
            	bt_fprintf (hStdout, "\n");
                if (result < 0)
                {
                	bt_fprintf (hStdout,
                            "While writing data to 0x%.8x-0x%.8x on %s: %m\n",
                            written, written + i, device);
                    exit (EXIT_FAILURE);
                }
                bt_fprintf (hStdout,
                        "Short write count returned while writing to x%.8x-0x%.8x on %s: %d/%lu bytes written to flash\n",
                        written, written + i, device, written + result, inode.ullFileSize);
                exit (EXIT_FAILURE);
            }

            written += i;
            size -= i;
        }
        bt_fprintf (hStdout,
                "\rWriting data: %luk/%luk (100%%)     \n",
                KB (filesize),
                KB (filesize));
        DEBUG("Wrote %d / %luk bytes\n", written, inode.ullFileSize);
    }

	/**********************************
	 * verify that flash == file data *
	 **********************************/

    if(flags & FLAG_VERIFY)
    {
        safe_rewind (fil_fd, filename);
        safe_rewind (dev_fd, device);
        size = inode.ullFileSize;
        i = BUFSIZE;
        written = 0;
        bt_fprintf (hStdout, "Verifying data: 0k/%luk (0%%)",KB (inode.ullFileSize));
        while (size)
        {
            if (size < BUFSIZE) i = size;
            bt_fprintf (hStdout,
                    "\rVerifying data: %dk/%luk (%d%%)",
                    KB (written + i),
                    KB (filesize),
                    PERCENTAGE ((unsigned long)(written + i),filesize));

            /* read from filename */
            safe_read (fil_fd, filename, src, i, BT_TRUE);

            /* read from device */
            safe_read (dev_fd, device, dest, i, BT_TRUE);

            /* compare buffers */
            if (memcmp (src, dest , i))
            {
            	bt_fprintf (hStdout,
                        "File does not seem to match flash data. First mismatch at 0x%.8x-0x%.8x\n",
                        written, written + i);
                exit (EXIT_FAILURE);
            }

            written += i;
            size -= i;
        }
        bt_fprintf (hStdout,
                "\rVerifying data: %luk/%luk (100%%)      \n",
                KB (filesize),
                KB (filesize));
        DEBUG("Verified %d / %luk bytes\n", written, inode.ullFileSize);
    }
	exit (EXIT_SUCCESS);
}

BT_SHELL_COMMAND_DEF oCommand = {
		.szpName	= "flashtool",
		.pfnCommand = bt_flashtool,
};

