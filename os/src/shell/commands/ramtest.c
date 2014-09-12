typedef unsigned int datum;     /* Set the data bus width to 32 bits. */

#define MEM_TEST_DATA_ERROR		1
#define MEM_TEST_ADDRESS_STUCK_HIGH	2
#define MEM_TEST_ADDRESS_STUCK_LOW	3
#define MEM_TEST_ADDRESS_SHORTED	4
#define MEM_TEST_DEVICE_INCR_ERROR	5
#define MEM_TEST_DEVICE_DECR_ERROR	6
static unsigned int error = 0;

/**********************************************************************
 *
 * Function:    memTestDataBus()
 *
 * Description: Test the data bus wiring in a memory region by
 *              performing a walking 1's test at a fixed address
 *              within that region.  The address (and hence the
 *              memory region) is selected by the caller.
 *
 * Notes:       
 *
 * Returns:     0 if the test succeeds.  
 *              A non-zero result is the first pattern that failed.
 *
 **********************************************************************/
datum
memTestDataBus(volatile datum * address)
{
    datum pattern;
    /*
     * Perform a walking 1's test at the given address.
     */
    for (pattern = 1; pattern != 0; pattern <<= 1)
    {
        /*
         * Write the test pattern.
         */
        *address = pattern;
        /*
         * Read it back (immediately is okay for this test).
         */
        if (*address != pattern)
        {
            error = MEM_TEST_DATA_ERROR;
            return (pattern);
        }
    }
    return (0);
}   /* memTestDataBus() */


/**********************************************************************
 *
 * Function:    memTestAddressBus()
 *
 * Description: Test the address bus wiring in a memory region by
 *              performing a walking 1's test on the relevant bits
 *              of the address and checking for aliasing. This test
 *              will find single-bit address failures such as stuck
 *              -high, stuck-low, and shorted pins.  The base address
 *              and size of the region are selected by the caller.
 *
 * Notes:       For best results, the selected base address should
 *              have enough LSB 0's to guarantee single address bit
 *              changes.  For example, to test a 64-Kbyte region,
 *              select a base address on a 64-Kbyte boundary.  Also,
 *              select the region size as a power-of-two--if at all
 *              possible.
 *
 * Returns:     0 if the test succeeds.  
 *              A non-zero result is the first address at which an
 *              aliasing problem was uncovered.  By examining the
 *              contents of memory, it may be possible to gather
 *              additional information about the problem.
 *
 **********************************************************************/
datum *
memTestAddressBus(volatile datum * baseAddress, unsigned long nBytes)
{
    unsigned long addressMask = (nBytes/sizeof(datum) - 1);
    unsigned long offset;
    unsigned long testOffset;
    datum pattern     = (datum) 0xAAAAAAAA;
    datum antipattern = (datum) 0x55555555;
    /*
     * Write the default pattern at each of the power-of-two offsets.
     */
    for (offset = 1; (offset & addressMask) != 0; offset <<= 1)
    {
        baseAddress[offset] = pattern;
    }
    /*
     * Check for address bits stuck high.
     */
    testOffset = 0;
    baseAddress[testOffset] = antipattern;
    for (offset = 1; (offset & addressMask) != 0; offset <<= 1)
    {
        if (baseAddress[offset] != pattern)
        {
            error = MEM_TEST_ADDRESS_STUCK_HIGH;
            return ((datum *) &baseAddress[offset]);
        }
    }
    baseAddress[testOffset] = pattern;
    /*
     * Check for address bits stuck low or shorted.
     */
    for (testOffset = 1; (testOffset & addressMask) != 0; testOffset <<= 1)
    {
        baseAddress[testOffset] = antipattern;
        if (baseAddress[0] != pattern)
        {
            error = MEM_TEST_ADDRESS_STUCK_LOW;
            return ((datum *) &baseAddress[testOffset]);
        }
        for (offset = 1; (offset & addressMask) != 0; offset <<= 1)
        {
            if ((baseAddress[offset] != pattern) && (offset != testOffset))
            {
                error = MEM_TEST_ADDRESS_SHORTED;
                return ((datum *) &baseAddress[testOffset]);
            }
        }
        baseAddress[testOffset] = pattern;
    }
    return (0);
}   /* memTestAddressBus() */


/**********************************************************************
 *
 * Function:    memTestDevice()
 *
 * Description: Test the integrity of a physical memory device by
 *              performing an increment/decrement test over the
 *              entire region.  In the process every storage bit
 *              in the device is tested as a zero and a one.  The
 *              base address and the size of the region are
 *              selected by the caller.
 *
 * Notes:       
 *
 * Returns:     0 if the test succeeds.  Also, in that case, the
 *              entire memory region will be filled with zeros.
 *
 *              A non-zero result is the first address at which an
 *              incorrect value was read back.  By examining the
 *              contents of memory, it may be possible to gather
 *              additional information about the problem.
 *
 **********************************************************************/
datum *
memTestDevice(volatile datum * baseAddress, unsigned long nBytes,unsigned long *Info)    
{
    unsigned long offset;
    unsigned long nWords = nBytes / sizeof(datum);
    datum pattern;
    datum antipattern;
    /*
     * Fill memory with a known pattern.
     */
    for (pattern = 1, offset = 0; offset < nWords; pattern++, offset++)
    {
        baseAddress[offset] = pattern;
    }
    /*
     * Check each location and invert it for the second pass.
     */
    for (pattern = 1, offset = 0; offset < nWords; pattern++, offset++)
    {
        if (baseAddress[offset] != pattern)
        {
            error = MEM_TEST_DEVICE_INCR_ERROR;
			if (Info)
			{
				Info[0]=(unsigned long)&baseAddress[offset];
				Info[1]=(unsigned long)baseAddress[offset];
				Info[2]=(unsigned long)offset;
				Info[3]=(unsigned long)pattern;
			}
            return ((datum *) &baseAddress[offset]);
        }
        antipattern = ~pattern;
        baseAddress[offset] = antipattern;
    }
    /*
     * Check each location for the inverted pattern and zero it.
     */
    for (pattern = 1, offset = 0; offset < nWords; pattern++, offset++)
    {
        antipattern = ~pattern;
        if (baseAddress[offset] != antipattern)
        {
            error = MEM_TEST_DEVICE_DECR_ERROR;
			if (Info)
			{
				Info[0]=(unsigned long)&baseAddress[offset];
				Info[1]=(unsigned long)baseAddress[offset];
				Info[2]=(unsigned long)offset;
				Info[3]=(unsigned long)pattern;
			}
            return ((datum *) &baseAddress[offset]);
        }
    }
    return (0);
}   /* memTestDevice() */


/**********************************************************************
 *
 * Function:    memTest()
 *
 * Description: Test a 64-k chunk of SRAM.
 *
 * Notes:       
 *
 * Returns:     0 on success.
 *              Otherwise -1 indicates failure.
 *
 **********************************************************************/
int
memTest(volatile datum * baseAddress, unsigned long nBytes)
{
    if ((memTestDataBus(baseAddress) != 0) ||
        (memTestAddressBus(baseAddress, nBytes) != 0) ||
        (memTestDevice(baseAddress, nBytes, 0) != 0))
    {
        return (-1);
    }
    else
    {
        return (0);
    }
        
}   /* memTest() */

#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

static int bt_ramtest(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);
	BT_ERROR Error = BT_ERR_NONE;
	unsigned long Info[4];

	if(argc != 3) {
		bt_fprintf(hStdout, "Usage: %s [base_address] [num_bytes]\n", argv[0]);
		return 0;
	}

	volatile datum *baseAddress = (volatile datum *)strtoul(argv[1], 0, 16);
	unsigned long ulBytes = (unsigned long)strtoul(argv[2], 0, 16);
	
	bt_fprintf(hStdout, "test memory region 0x%08x...0x%08x:\n", (unsigned long)baseAddress, (unsigned long)baseAddress + ulBytes - 1);

	datum *retAddress = NULL;
	datum retPattern = 0;
	bt_fprintf(hStdout, "perform data bus test    ... ");
    	if ((retPattern = memTestDataBus(baseAddress)) != 0) {
		Error = -1;
		bt_fprintf(hStdout, "FAILED with pattern 0x%08x\n", retPattern);
	} else bt_fprintf(hStdout, "done\n");

	bt_fprintf(hStdout, "perform address bus test ... ");
    	if ((retAddress = memTestAddressBus(baseAddress,ulBytes)) != 0) {
		Error = -1;
		bt_fprintf(hStdout, "FAILED at address 0x%08x", (unsigned long)retAddress);
		if(error == MEM_TEST_ADDRESS_STUCK_HIGH) bt_fprintf(hStdout, " (STUCK HIGH)\n");
		else if(error == MEM_TEST_ADDRESS_STUCK_LOW) bt_fprintf(hStdout, " (STUCK LOW)\n");
		else if(error == MEM_TEST_ADDRESS_SHORTED) bt_fprintf(hStdout, " (SHORTED)\n");
		else bt_fprintf(hStdout, "\n");
	} else bt_fprintf(hStdout, "done\n");

	bt_fprintf(hStdout, "perform device test      ... ");
    	if ((retAddress = memTestDevice(baseAddress,ulBytes,Info)) != 0) {
		Error = -1;
		bt_fprintf(hStdout, "FAILED at address 0x%08x (0x%08x, 0x%08x, 0x%08x, 0x%08x)", (unsigned long)retAddress,Info[0],Info[1],Info[2],Info[3]);
		if(error == MEM_TEST_DEVICE_INCR_ERROR) bt_fprintf(hStdout, " (INCREMENT)\n");
		else if(error == MEM_TEST_DEVICE_DECR_ERROR) bt_fprintf(hStdout, " (DECREMENT)\n");
		else bt_fprintf(hStdout, "\n");
	}
	bt_fprintf(hStdout, "done\n");

	return Error;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "ramtest",
	.pfnCommand = bt_ramtest,
};

