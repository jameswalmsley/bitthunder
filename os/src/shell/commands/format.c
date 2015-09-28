#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

static void usage(BT_HANDLE hStdout, char **argv) {
    bt_fprintf(hStdout, "Usage: %s [volume] [filesystem]\n", argv[0]);
}

static int bt_format(BT_HANDLE hShell, int argc, char **argv) {

    BT_HANDLE hStdout = BT_ShellGetStdout(hShell);

    if(argc != 3) {
        usage(hStdout, argv);
        return -1;
    }

    bt_printf("Formatting %s for the %s filesystem\n", argv[1], argv[2]);
    BT_Format(argv[1], argv[2]);

    return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
    .szpName = "format",
    .pfnCommand = bt_format,
};
