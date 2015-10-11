#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

static int bt_loop(BT_HANDLE hShell, int argc, char **argv) {

    int i;
    char *command = NULL;

    int y = 0;
    for(i = 2; i < argc; i++) {
        int len = strlen(argv[i]) + 1;
        command = BT_kRealloc(command, y+len);
        strcpy(command + y, argv[i]);
        *(command+y+len-1)=' ';
        if(i == argc-1) {
        	*(command+y+len-1)='\0';
        }
        y = y+len;
    }


  int iterations = atoi(argv[1]);
  for(i = 0; i < iterations; i++) {
      BT_ShellCommand(hShell, command);
  }

  BT_kFree(command);

  return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "loop",
	.pfnCommand = bt_loop,
};
