#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

static char *device_path = NULL;
static int g_active_partition = 0;
static int *p_active_partition = NULL;    ///< NULLABLE variable, backed by selected_partition..

static BT_PARTITION_PARAMETERS g_part_params;
static struct bt_part_t g_partition_info[BT_PARTITION_MAX] = { 0 };
static struct bt_part_t *p_partition_info = NULL;
static int g_partitions = 0;

static void usage_disk(BT_HANDLE hShell) {
    BT_PRSHELL("disk [block-device-path]\n");
    BT_PRSHELL("    +- Opens a disk to modify the partition table\n");
    BT_PRSHELL("    +- E.g. disk /dev/mmc0\n");
}

static void usage_close(BT_HANDLE hShell) {
    BT_PRSHELL("close\n");
    BT_PRSHELL("    +- Closes the active disk (%s)\n", device_path ? device_path : "none-selected");
}

static void usage_list(BT_HANDLE hShell) {
    BT_PRSHELL("list\n");
    BT_PRSHELL("     +- List all partitions.\n");
}

static void usage_select(BT_HANDLE hShell) {
    BT_PRSHELL("select [partition-number]\n");
    BT_PRSHELL("    +- Select a partition for editing.\n");
}

static void usage_add(BT_HANDLE hShell) {
    BT_PRSHELL("add [start address] [total sectors]\n");
    BT_PRSHELL("    +- Adds a partition to the active disk\n");
}

static void usage_delete(BT_HANDLE hShell) {
    BT_PRSHELL("delete\n");
    BT_PRSHELL("    +- Deletes the currently selected partition.\n");
}

static void usage_commit(BT_HANDLE hShell) {
    BT_PRSHELL("commit\n");
    BT_PRSHELL("    +- Commit the partition table to disk.\n");
}

static void usage(BT_HANDLE hShell) {
    BT_PRSHELL("Usage: partition [command] {[command args]}\n");
    BT_PRSHELL("Command documentation:\n");
    usage_disk(hShell);
    usage_close(hShell);
    usage_list(hShell);
    usage_select(hShell);

    usage_commit(hShell);
    /*BT_PRSHELL("     : %s select [partition-id]\n",  argv[0]);
    BT_PRSHELL("     +- Select a partition to modify.\n");
    BT_PRSHELL("     : %s clear\n",                  argv[0]);
    BT_PRSHELL("     : %s create [size]\n",          argv[0]);
    BT_PRSHELL("     +- Create a partition.\n");
    BT_PRSHELL("     : %s delete\n",                 argv[0]);
    BT_PRSHELL("     +- Delete the selcted partition\n");
    BT_PRSHELL("     : %s commit\n",                 argv[0]);
    BT_PRSHELL("     +- Commit the partition table to MBR\n");
    BT_PRSHELL("     : %s reset\n",                  argv[0]);*/
}

static int no_disk(BT_HANDLE hShell) {
    BT_PRSHELL("No active disk selected, use the disk command.\n");
    usage_disk(hShell);
    return -1;
}

static void _order_partitions() {
    int i;
    for(i = 0; i < BT_PARTITION_MAX-1; i++) {
        if(!g_partition_info[i].ulStartLBA && !g_partition_info[i].ulSectorCount) {
            g_partition_info[i] = g_partition_info[i+1];
        }
    }
}

static int bt_partition_disk(BT_HANDLE hShell, int argc, char **argv) {

    if(argc > 2) {
        usage_disk(hShell);
        return -1;
    }

    if(argc == 1) {
        BT_PRSHELL("Current disk: %s\n", device_path ? device_path : "none-selected");
        return 0;
    }

    if(!device_path) {
        int retval = 0;
        BT_ERROR Error;
        BT_HANDLE hBlock = BT_Open(argv[1], 0, &Error);
        if(!hBlock) {
            BT_PRSHELL("Error: Could not open %s\n", argv[1]);
            return -1;
        }

        /**
         *  Ensure we have a valid block device handle.
         *  This API will return BT_ERR_INVALID_HANDLE, or BT_ERR_NONE.
         *
         *  Maybe we should actually add a simple API for that!
         **/
        if(BT_GetBlockGeometry(hBlock, NULL) != BT_ERR_NONE) {
            BT_PRSHELL("Error: %s is not a RAW block device\n", argv[1]);
            retval = -1;
            goto close_out;
        }

        device_path = BT_kMalloc(strlen(argv[1]));
        strcpy(device_path, argv[1]);

close_out:
        BT_CloseHandle(hBlock);
        return retval;

    } else {
        BT_PRSHELL("Already working with: %s\n", device_path);
    }

    return 0;
}

static int bt_partition_close(BT_HANDLE hShell, int argc, char **argv) {
    if(argc != 1) {
        usage_close(hShell);
        return -1;
    }

    if(device_path) {
        BT_kFree(device_path);
        device_path = NULL;
        p_partition_info = NULL;
        g_partitions = 0;
    } else {
        return no_disk(hShell); // Error message for no active disk.
    }

    return 0;
}

static int bt_partition_list(BT_HANDLE hShell, int argc, char **argv) {

    if(argc != 1) {
        usage_list(hShell);
        return -1;
    }

    if(!device_path) {
        return no_disk(hShell);
    }

    BT_BLOCK_GEOMETRY oGeom;

    BT_u32 i;
    if(!p_partition_info) {
        BT_ERROR Error;
        BT_HANDLE hBlock = BT_Open(device_path, 0, &Error);
        if(!hBlock) {
            BT_PRSHELL("Error: Could not open %s\n", argv[1]);
            return -1;
        }

        BT_GetBlockGeometry(hBlock, &oGeom);

        g_partitions = BT_PartitionCount(hBlock);
        for(i = 0; i < g_partitions; i++) {
            BT_PartitionInfo(hBlock, i, &g_partition_info[i]);
        }

        p_partition_info = g_partition_info;

        BT_CloseHandle(hBlock);
    }

    BT_PRSHELL("Device %s contains %d partitions\n", device_path, g_partitions);
    BT_PRSHELL("BlockSize    : %lu\n", oGeom.ulBlockSize);
    BT_PRSHELL("Total Blocks : %lu\n", oGeom.ulTotalBlocks);

    for(i = 0; i < g_partitions; i++) {
        BT_PRSHELL("  %d : %lu : %lu\n", i, g_partition_info[i].ulStartLBA, g_partition_info[i].ulSectorCount);
    }

    return 0;
}

static int bt_partition_select(BT_HANDLE hShell, int argc, char **argv) {
    if(argc != 2) {
        usage_select(hShell);
    }

    if(!device_path) {
        return no_disk(hShell);
    }

    int partition = atoi(argv[1]);
    if(partition > g_partitions) {
        BT_PRSHELL("Invalid partition number (%d), select 0 - %d\n", partition, g_partitions-1);
        p_active_partition = NULL;  ///< An error occurred, nullify the partition selection to prevent user error.
        return -1;
    }

    g_active_partition = partition;
    p_active_partition = &g_active_partition;

    BT_PRSHELL("Selected partition: %d\n", *p_active_partition);

    return 0;
}

static int bt_partition_add(BT_HANDLE hShell, int argc, char **argv) {

    if(argc != 3) {
        usage_add(hShell);
        return -1;
    }

    if(!device_path) {
        return no_disk(hShell);
    }

    if(g_partitions > BT_PARTITION_MAX) {
        BT_PRSHELL("Cannot add another partition, all primary partition entries are filled.\n");
        return -1;
    }

    BT_u32 ulStartLBA = atoi(argv[1]);
    BT_u32 ulSectorCount = atoi(argv[2]);

    g_partition_info[g_partitions].ulStartLBA = ulStartLBA;
    g_partition_info[g_partitions].ulSectorCount = ulSectorCount;

    g_partitions++;

    BT_PRSHELL("Added a partition: (Starts: %lu, Length: %lu)\n", ulStartLBA, ulSectorCount);

    return 0;
}

static int bt_partition_clear(BT_HANDLE hShell, int argc, char **argv) {
    memset(g_partition_info, 0, sizeof(g_partition_info));
    BT_PRSHELL("Cleared the partition table.\n");
    return 0;
}

static int bt_partition_delete(BT_HANDLE hShell, int argc, char **argv) {
    if(argc != 1) {
        usage_delete(hShell);
        return -1;
    }

    if(!device_path) {
        return no_disk(hShell);
    }

    if(!p_active_partition) {
        BT_PRSHELL("No partitions have been selected, use the select command.\n");
        return -1;
    }

    memset(&p_partition_info[*p_active_partition], 0, sizeof(struct bt_part_t));

    if(*p_active_partition != g_partitions-1) {
        _order_partitions();
    }

    BT_PRSHELL("Partition %d was deleted\n", *p_active_partition);

    g_partitions -= 1;
    p_active_partition = NULL;
    g_active_partition = 0;

    return 0;
}

static int bt_partition_commit(BT_HANDLE hShell, int argc, char **argv) {
    BT_kPrint("partition: commit");
    if(argc != 1) {
        usage_commit(hShell);
        return -1;
    }

    if(!device_path) {
        return no_disk(hShell);
    }

    memset(&g_part_params, 0, sizeof(g_part_params));

    if(g_partitions > 0) {
        g_part_params.ulHiddenSectors = p_partition_info[0].ulStartLBA;
    } else {
        g_part_params.ulHiddenSectors = 2048;
    }

    g_part_params.ulPrimaryCount = g_partitions;
    g_part_params.eSizeType = BT_PARTITION_SIZE_SECTORS;

    BT_u32 i;
    for(i = 0; i < g_partitions; i++) {
        g_part_params.ulSizes[i] = p_partition_info[i].ulSectorCount;
    }

    BT_Partition(device_path, &g_part_params);

    return 0;
}

static const struct bt_shell_subcommand partition_sub_commands[] = {
    {
        .name = "disk",
        .fn_command = bt_partition_disk,
    },
    {
        .name = "close",
        .fn_command = bt_partition_close,
    },
    {
        .name = "list",
        .fn_command = bt_partition_list,
    },
    {
        .name = "select",
        .fn_command = bt_partition_select,
    },
    {
        .name = "add",
        .fn_command = bt_partition_add,
    },
    {
        .name = "clear",
        .fn_command = bt_partition_clear,
    },
    {
        .name = "delete",
        .fn_command = bt_partition_delete,
    },
    {
        .name = "commit",
        .fn_command = bt_partition_commit,
    },
    { NULL }
};

static int bt_partition(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);

    if(BT_ShellProcessSubcommands(hShell, argc, argv, partition_sub_commands)) {
        usage(hShell);
        return -1;
    }

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName 	= "partition",
	.pfnCommand = bt_partition,
};
