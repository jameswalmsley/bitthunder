/**
 * BitThunder Kernel Shell
 **/

#include <bitthunder.h>
#include <string.h>
#include <ctype.h>

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

BT_ERROR BT_ShellCommand(char *input) {

	BT_u32 ulArguments = 0;
	BT_u32 bIsArg = BT_FALSE;

	char *copy = BT_kMalloc(strlen(input)+1);
	if(!copy) {
		return BT_ERR_NO_MEMORY;
	}

	strcpy(copy, input);

	input = copy;

	while(isspace((int)*input)) {
		input++;	// Eat up prefixed whitespace.
	}

	bIsArg = BT_TRUE;

	char *line = input;

	while(*input) {
		if(bIsArg) {
			if(isspace((int)*input)) {
				ulArguments += 1;
				bIsArg = BT_FALSE;
			}
		} else {
			if(!isspace((int)*input)) {
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

	input = line;
	while(!*input);

	bIsArg = BT_FALSE;

	BT_u32 i = 0;

	while(*input) {
		if(!bIsArg) {
			if(!isspace((int)*input)) {
				bIsArg = BT_TRUE;
				pargs[i++] = input;
			}
		} else {
			if(isspace((int)*input)) {
				*input = '\0';
				bIsArg = BT_FALSE;
			}
		}
		input++;
	}

	const BT_SHELL_COMMAND *pCommand = GetShellCommand(pargs[0]);
	if(!pCommand) {
		BT_kFree(pargs);
		BT_kFree(copy);
		return BT_ERR_NONE;
	}

	pCommand->pfnCommand(ulArguments, pargs);

	BT_kFree(pargs);
	BT_kFree(copy);

	return BT_ERR_NONE;
}
