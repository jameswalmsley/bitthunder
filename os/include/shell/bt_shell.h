/**
 *	BitThunder - Kernel Command Shell
 *
 **/

#ifndef _BT_SHELL_H_
#define _BT_SHELL_H_

typedef struct _BT_SHELL_CONFIG {
	BT_HANDLE hOut, hIn;
} BT_SHELL_CONFIG;

typedef int (*BT_SHELL_COMMAND_EX_FN)(int argc, char **argv, BT_SHELL_CONFIG *pConfig);
typedef int (*BT_SHELL_COMMAND_FN)(int argc, char **argv);

typedef enum _BT_SHELL_COMMAND_TYPE {
	BT_SHELL_NORMAL_COMMAND = 0,
	BT_SHELL_EXTENDED_COMMAND,
} BT_SHELL_COMMAND_TYPE;

typedef struct _BT_SHELL_COMMAND {
	const char 			   *szpName;
	BT_SHELL_COMMAND_TYPE	eType;
	union {
		BT_SHELL_COMMAND_FN	   pfnCommand;
		BT_SHELL_COMMAND_EX_FN pfnExCommand;
	};
} BT_SHELL_COMMAND;

#define BT_SHELL_COMMAND_DEF 		static const BT_ATTRIBUTE_SECTION(".bt.shell.commands") BT_SHELL_COMMAND

BT_ERROR BT_ShellCommand(char *input);

#endif
