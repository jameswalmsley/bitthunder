/**
 *	BitThunder - Kernel Command Shell
 *
 **/

#ifndef _BT_SHELL_H_
#define _BT_SHELL_H_


typedef int (*BT_SHELL_COMMAND_FN)(BT_HANDLE hShell, int argc, char **argv);

typedef struct _BT_SHELL_COMMAND {
	const char 		*szpName;
	BT_SHELL_COMMAND_FN	pfnCommand;
} BT_SHELL_COMMAND;

extern BT_HANDLE BT_stdsh;

#define BT_SHELL_COMMAND_DEF 		static const BT_ATTRIBUTE_SECTION(".bt.shell.commands") BT_SHELL_COMMAND

#define BT_SHELL_FLAG_ALLOW_EXIT	0x00000001
#define BT_SHELL_FLAG_NON_BLOCK		0x00000002	///< Don't block on read-call, return immediately.

BT_HANDLE BT_ShellCreate(BT_HANDLE hStdin, BT_HANDLE hStdout, const BT_i8 *szpPrompt, BT_u32 ulFlags, BT_ERROR *pError);

void BT_ShellDestroy(BT_HANDLE hShell);

BT_HANDLE BT_ShellGetStdout(BT_HANDLE hShell);

BT_HANDLE BT_ShellGetStdin(BT_HANDLE hShell);

const char *BT_ShellGetPrompt(BT_HANDLE hShell);

void BT_ShellUpdatePrompt(BT_HANDLE hShell, const char *szpPrompt);

BT_u32 BT_ShellGetFlags(BT_HANDLE hShell);

BT_ERROR BT_Shell(BT_HANDLE hShell);

BT_ERROR BT_ShellCommand(BT_HANDLE hShell, const char *input);

BT_ERROR BT_ShellScript(BT_HANDLE hShell, const BT_i8 *path);

#endif
