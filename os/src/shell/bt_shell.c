/**
 *	BitThunder Kernel Shell
 *
 **/

#include <bitthunder.h>
#include <string.h>
#include <ctype.h>
#include <shell/bt_env.h>
#include <stdio.h>
#include <lib/putc.h>

BT_DEF_MODULE_NAME			("shell")
BT_DEF_MODULE_DESCRIPTION	("Kernel Shell subsystem")
BT_DEF_MODULE_AUTHOR	  	("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 		h;			///< All handles must include a handle header.
	BT_HANDLE			hStdin;
	BT_HANDLE			hStdout;
	const BT_i8 			*szpPrompt;
	BT_u32	 			ulPromptLen;
	BT_i32				bPrintPrompt;
	BT_u32 				ulFlags;
	#define BT_CONFIG_SHELL_INPUT_BUFFER_SIZE	256
	BT_i8 				cStdinBuf[BT_CONFIG_SHELL_INPUT_BUFFER_SIZE];
	BT_u32 				ulStdinBufCnt;
	struct _BT_OPAQUE_HANDLE	*pNext;
};

typedef struct _BT_OPAQUE_HANDLE BT_SHELL;
typedef BT_SHELL *BT_SHELL_HANDLE;

extern const BT_SHELL_COMMAND * __bt_shell_commands_start;
extern const BT_SHELL_COMMAND * __bt_shell_commands_end;

static const BT_SHELL_COMMAND *GetShellCommand(const BT_i8 *name) {
	BT_u32 size = (BT_u32) ((BT_u32) &__bt_shell_commands_end - (BT_u32) &__bt_shell_commands_start);
	BT_u32 i;

	size /= sizeof(BT_SHELL_COMMAND);

	const BT_SHELL_COMMAND *pCommand = (BT_SHELL_COMMAND *) &__bt_shell_commands_start;

	for(i = 0; i < size; i++) {
		if(!strcmp(name, pCommand->szpName)) {
			return pCommand;
		}
		pCommand++;
	}

	return NULL;
}

static char *replace_var(char *input) {
	char *start = input + 2;
	char *end = strchr(start, '}');
	if(!end) {
		return input;
	}

	*end = '\0';
	BT_ENV_VARIABLE *env = BT_ShellGetEnv(start);
	*end = '}';

	if(!env) {
		const char *ret = "\0";
		return (char *)ret;
	}
	return env->o.string->s;
}

static char *eatspace(char *input) {
	while(isspace((int)*input)) {
		input++;	// Eat up prefixed whitespace.
	}
	return input;
}

static char *evaluate_command(BT_HANDLE hShell, char *input) {
	char *start = input + 2;
	char *end = strchr(start, ')');
	if(!end) {
		return NULL;
	}

	*end = '\0';
	BT_ERROR retval = BT_ShellCommand(NULL, start);
	*end = ')';

	char *output = BT_kMalloc(64);
	if(!output) {
		return NULL;
	}

	int len = snprintf(output, 64, "%d", (int) retval);
	output[len] = '\0';

	return output;
}

static char *reduce_value(BT_HANDLE hShell, BT_i8 *value, BT_BOOL *bAlloced) {
	if(value[0] == '$' && value[1] == '{') {
		return replace_var(value);
	} else if(value[0] == '$' && value[1] == '(') {
		char *ret = evaluate_command(hShell, value);
		if(!ret) {
			return "\0";
		}
		*bAlloced = BT_TRUE;
		return ret;
	}

	return value;
}

static char *unquote(char *line) {
	if(line[0] == '"') {
		line++;
		char *a_end = strchr(line, '"');
		if(a_end) {
			*a_end = '\0';
		}
	}
	return line;
}

static BT_BOOL parse_condition(BT_HANDLE hShell, BT_i8 *line) {

	BT_BOOL retval = BT_FALSE;

	char *eval = strstr(line, "==");
	if(eval) {
		char *a = line+2;
		char *b = eval + 3;
		*eval = '\0';

		a = eatspace(a);
		b = eatspace(b);

		BT_BOOL bAlloced_a = BT_FALSE;
		BT_BOOL bAlloced_b = BT_FALSE;
		a = reduce_value(hShell, a, &bAlloced_a);
		b = reduce_value(hShell, b, &bAlloced_b);

		char *a_unquoted = unquote(a);
		char *b_unquoted = unquote(b);

		if(!strcmp(a_unquoted, b_unquoted)) {
			retval = BT_TRUE;
		}

		if(bAlloced_b) {
			BT_kFree(b);
		}

		if(bAlloced_a) {
			BT_kFree(a);
		}

		*eval = '=';

		return retval;
	}

	BT_BOOL bAlloced = BT_FALSE;
	eval = eatspace(line+2);
	eval = reduce_value(hShell, eval, &bAlloced);

	if(strcmp(eval, "\0")) {
		retval = BT_TRUE;
	}

	if(bAlloced) {
		BT_kFree(eval);
	}

	return retval;
}

static char *replace_expressions(BT_HANDLE hShell, const char *input) {

	char   *last_replace 	= BT_kMalloc(strlen(input) + 1);
	char   *replaced  		= NULL;
	char   *item 			= NULL;
	char   *item_end 		= NULL;

	strcpy(last_replace, input);

	goto next_item;

	while(item) {
		BT_u32 	input_len 		= strlen(last_replace);

		if(item[1] == '{') {
			item_end = strchr(item, '}');
		} else if(item[1] == '(') {
			item_end = strchr(item, ')');
		} else {
			item++;
			goto next_item;
		}

		BT_BOOL	bAlloced = BT_FALSE;

		char   *reduced 		= reduce_value(hShell, item, &bAlloced);

		BT_u32 	reduced_len 	= strlen(reduced);
		BT_u32 	orig_len 		= (item_end - item);
		BT_s32	diff 			= reduced_len - orig_len;

		replaced = BT_kMalloc(input_len + diff + 1);
		if(!replaced) {
			if(last_replace) {
				BT_kFree(last_replace);
			}
			if(bAlloced) {
				BT_kFree(reduced);
			}
			return NULL;
		}

		char *start = last_replace;

		BT_u32 i;
		strncpy(replaced, start, (item - start));			// Copy the string from before the replacement.
		i = (item - start);

		strncpy(replaced + i, reduced, reduced_len);		// Copy the reduced string.
		i += reduced_len;

		strcpy(replaced+i, item_end + 1);					// Copy the other half of the string.

		if(bAlloced) {
			BT_kFree(reduced);
		}

		BT_kFree(last_replace);
		last_replace = replaced;

	next_item:
		item = strchr(last_replace, '$');
	}

	return last_replace;
}

BT_HANDLE BT_ShellGetStdout(BT_HANDLE hShell) {
	return (hShell ? ((BT_SHELL_HANDLE)hShell)->hStdout : NULL);
}

BT_HANDLE BT_ShellGetStdin(BT_HANDLE hShell) {
	return (hShell ? ((BT_SHELL_HANDLE)hShell)->hStdin : NULL);
}

const char *BT_ShellGetPrompt(BT_HANDLE hShell)
{
	return (hShell ? ((BT_SHELL_HANDLE)hShell)->szpPrompt : NULL);
}

void BT_ShellUpdatePrompt(BT_HANDLE hShell, const char *szpPrompt)
{
	if(hShell) {
		((BT_SHELL_HANDLE)hShell)->szpPrompt = szpPrompt;
	}
	return;
}

BT_u32 BT_ShellGetFlags(BT_HANDLE hShell)
{
	return (hShell ? ((BT_SHELL_HANDLE)hShell)->ulFlags : 0);
}

BT_ERROR BT_ShellCommand(BT_HANDLE hShell, const char *cmdline) {

	BT_ERROR Error = BT_ERR_NONE;

	BT_u32 ulArguments = 0;
	BT_u32 bIsArg = BT_FALSE;
	BT_u32 bIgnoreSpace = BT_FALSE;
	char *input;

	if(!cmdline || cmdline[0] == '\0') {
		return BT_ERR_NONE;
	}

	/*
	 *	replace_expressions always returns a reduced copy of the original input string.
	 *	therefore it can always be free'd!
	 */
	char *copy = replace_expressions(hShell, cmdline);				// This does complete string variable/command substitution.

	input = copy;
	input = eatspace(input);

	bIsArg = BT_TRUE;

	char *line = input;

	while(*input) {
		if(!bIgnoreSpace && *input == '"') {			// Allow arguments with spaces in them, is surrounded by quotes.
				bIgnoreSpace = BT_TRUE;
				input++;					// skip the char.
				continue;
		} else {
			if(*input == '"') {
				bIgnoreSpace = BT_FALSE;
				input++;
				continue;
			}
		}

		if(bIsArg) {
			if(!bIgnoreSpace && isspace((int)*input)) {
				ulArguments += 1;
				bIsArg = BT_FALSE;
			}
		} else {
			if(bIgnoreSpace || !isspace((int)*input)) {
				bIsArg = BT_TRUE;
			}
		}
		input++;
	}

	if(bIsArg) {
		ulArguments += 1;
	}

	char **pargs = BT_kMalloc(sizeof(char *) * ulArguments);
	if(!pargs) {
		return BT_ERR_NO_MEMORY;
	}

	input = eatspace(line);

	bIsArg = BT_FALSE;

	BT_u32 i = 0;
	bIgnoreSpace = BT_FALSE;

	while(*input) {
		if(!bIgnoreSpace && *input == '"') {
			bIgnoreSpace = BT_TRUE;
			input++;
			continue;
		} else {
			if(*input == '"') {
				bIgnoreSpace = BT_FALSE;
				if(bIsArg) {
					*input = '\0';
				}
				input++;
				continue;
			}
		}

		if(!bIsArg) {
			if(!isspace((int)*input)) {
				bIsArg = BT_TRUE;
				pargs[i++] = input;
			}
		} else {
			if(!bIgnoreSpace && isspace((int)*input)) {
				*input = '\0';
				bIsArg = BT_FALSE;
			}
		}
		input++;
	}

	const BT_SHELL_COMMAND *pCommand = GetShellCommand(pargs[0]);
	if(!pCommand) {
		BT_ENV_VARIABLE *env = BT_ShellGetStarredEnv(pargs[0]);
		if(env) {
			Error = BT_ShellCommand(hShell, env->o.string->s);
			goto executed;
		}

		BT_kFree(pargs);
		BT_kFree(copy);
		return -1;
	}

	Error = pCommand->pfnCommand(hShell, ulArguments, pargs);

executed:
	BT_kFree(pargs);
	BT_kFree(copy);

	return Error;
}

BT_ERROR BT_ShellScript(BT_HANDLE hShell, const BT_i8 *path) {

	BT_ERROR 	Error			= BT_ERR_NONE;
	BT_u32		if_false_depth 	= 0;
	BT_BOOL		if_reduced 		= BT_FALSE;		// Result of parsed condition.

	BT_HANDLE hFile = BT_Open(path, BT_GetModeFlags("rb"), &Error);
	if(!hFile) {
		BT_kPrint("Could not open shell script %s\n", path);
		return BT_ERR_GENERIC;
	}

	BT_i8 *line = BT_kMalloc(256);
	if(!line) {
		BT_CloseHandle(hFile);
		return BT_ERR_NO_MEMORY;
	}

	BT_u32 lineno = 0;


	BT_u32 linelen;

	while((linelen = BT_GetS(hFile, 256, line)) > 0) {

		lineno++;

		if(line[linelen-1] == '\n' || line[linelen-1] == '\r') {
			linelen -= 1;
		}

		line[linelen] = '\0';

		BT_i8 *p = eatspace(line);
		if(*p == '#') {
			continue;	// commented line!
		}

		if(p == (line + linelen)) {
			continue;
		}

		if(!strncmp(p, "if", 2)) {
			if_reduced = parse_condition(hShell, p);
			if(!if_reduced) {
				if_false_depth += 1;
			}
			continue;
		}

		if(!strncmp((char *) p, "else", 4)) {
			if(if_false_depth == 1) {
				if_false_depth ^= 1;
			} else {
				if_false_depth += 1;
			}

			continue;
		}

		if(!strncmp(p, "endif", 5)) {
			if(if_false_depth) {
				if_false_depth -= 1;
			}
			continue;
		}

		if(!if_false_depth) {
			Error = BT_ShellCommand(hShell, p);
			if(Error) {
				BT_kPrint("Error executing line %d in %s:", lineno, path);
				BT_kPrint("%d : %s", lineno, p);
				goto err_out;
			}
		}
	}

err_out:

	BT_kFree(line);
	BT_CloseHandle(hFile);

	return BT_ERR_NONE;
}

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.eType = BT_HANDLE_T_SHELL,
};

BT_HANDLE BT_ShellCreate(BT_HANDLE hStdin, BT_HANDLE hStdout, const BT_i8 *szpPrompt, BT_u32 ulFlags, BT_ERROR *pError) {
	BT_HANDLE hShell;

	hShell = BT_CreateHandle(&oHandleInterface, sizeof(BT_SHELL), pError);
	if(!hShell) {
		goto err_out;
	}

	hShell->hStdin = hStdin;
	hShell->hStdout = hStdout;
	hShell->szpPrompt = szpPrompt;
	hShell->ulPromptLen = strlen(szpPrompt);
	hShell->ulFlags = ulFlags;
	hShell->ulStdinBufCnt = 0;
	hShell->bPrintPrompt = 1;

	return hShell;

err_out:

	return NULL;
}

BT_ERROR BT_Shell(BT_HANDLE hShell) {
	BT_ERROR Error = BT_ERR_NONE;

	if(hShell) {
		do {
			// print prompt
			if(hShell->bPrintPrompt) {
				BT_Write(hShell->hStdout, 0, hShell->ulPromptLen, (char *)hShell->szpPrompt);
				hShell->bPrintPrompt = 0;
			}
			// get next char
			BT_s32 c = BT_GetC(hShell->hStdin, BT_FILE_NON_BLOCK);
			if(c >= 0) {
				if(c == '\r' || c == '\n') {
					// cr or lf detected .. echo cr and lf
					BT_Write(hShell->hStdout, 0, 2, "\r\n");
					// zero terminate command buffer
					hShell->cStdinBuf[hShell->ulStdinBufCnt] = 0;
					// execute command
					BT_ShellCommand(hShell, hShell->cStdinBuf);
					// prepare for next command
					hShell->ulStdinBufCnt = 0;
					hShell->bPrintPrompt = 1;
				} else if(c == '\b') {
					if(hShell->ulStdinBufCnt) {
						BT_PutC(hShell->hStdout, 0, c);
						BT_PutC(hShell->hStdout, 0, ' ');
						BT_PutC(hShell->hStdout, 0, c);
						hShell->ulStdinBufCnt -= 1;
					}
				} else {
					// echo char
					BT_PutC(hShell->hStdout, 0, c);
					if(hShell->ulStdinBufCnt < BT_CONFIG_SHELL_INPUT_BUFFER_SIZE-1) {
						hShell->cStdinBuf[hShell->ulStdinBufCnt++] = c;
					}
				}
			}
		}
		while((hShell->ulFlags & BT_SHELL_FLAG_NON_BLOCK) == 0);
	} else {
		Error = BT_ERR_GENERIC;
	}

	return Error;
}
