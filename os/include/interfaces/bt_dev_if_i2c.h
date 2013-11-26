#ifndef _BT_DEV_IF_I2C_H_
#define _BT_DEV_IF_I2C_H_

#include "bt_types.h"

typedef struct _BT_I2C_MESSAGE BT_I2C_MESSAGE;
typedef struct _BT_I2C_CLIENT BT_I2C_CLIENT;
typedef struct _BT_I2C_BUS BT_I2C_BUS;

typedef enum {
	BT_I2C_CLOCKRATE_100kHz = 0,
	BT_I2C_CLOCKRATE_400kHz,
	BT_I2C_CLOCKRATE_1000kHz,
	BT_I2C_CLOCKRATE_3400kHz,
} BT_I2C_CLOCKRATE;

typedef enum {
	BT_I2C_WRITE_ACCESS = 0,
	BT_I2C_READ_ACCESS,
} BT_I2C_ACCESS_MODE;

typedef struct {
	BT_u32    	ulFunctionality;
	#define BT_I2C_FUNC_I2C						0x00000001
	#define BT_I2C_FUNC_10BIT_ADDR				0x00000002
    #define BT_I2C_FUNC_PROTOCOL_MANGLING		0x00000004 			///< I2C_M_IGNORE_NAK etc.
	#define BT_I2C_FUNC_SMBUS_PEC				0x00000008
	#define BT_I2C_FUNC_NOSTART					0x00000010 /* I2C_M_NOSTART */
	#define BT_I2C_FUNC_SMBUS_BLOCK_PROC_CALL	0x00008000 /* SMBus 2.0 */
	#define BT_I2C_FUNC_SMBUS_QUICK				0x00010000
	#define BT_I2C_FUNC_SMBUS_READ_BYTE			0x00020000
	#define BT_I2C_FUNC_SMBUS_WRITE_BYTE		0x00040000
	#define BT_I2C_FUNC_SMBUS_READ_BYTE_DATA	0x00080000
	#define BT_I2C_FUNC_SMBUS_WRITE_BYTE_DATA	0x00100000
	#define BT_I2C_FUNC_SMBUS_READ_WORD_DATA	0x00200000
	#define BT_I2C_FUNC_SMBUS_WRITE_WORD_DATA	0x00400000
	#define BT_I2C_FUNC_SMBUS_PROC_CALL			0x00800000
	#define BT_I2C_FUNC_SMBUS_READ_BLOCK_DATA	0x01000000
	#define BT_I2C_FUNC_SMBUS_WRITE_BLOCK_DATA 	0x02000000
	#define BT_I2C_FUNC_SMBUS_READ_I2C_BLOCK	0x04000000 /* I2C-like block xfer  */
	#define BT_I2C_FUNC_SMBUS_WRITE_I2C_BLOCK	0x08000000 /* w/ 1-byte reg. addr. */
	#define BT_I2C_FUNC_SMBUS_BYTE				(BT_I2C_FUNC_SMBUS_READ_BYTE | \
	                                             BT_I2C_FUNC_SMBUS_WRITE_BYTE)
	#define BT_I2C_FUNC_SMBUS_BYTE_DATA			(BT_I2C_FUNC_SMBUS_READ_BYTE_DATA | \
	 		    			                     BT_I2C_FUNC_SMBUS_WRITE_BYTE_DATA)
	#define BT_I2C_FUNC_SMBUS_WORD_DATA			(BT_I2C_FUNC_SMBUS_READ_WORD_DATA | \
	 		    			                     BT_I2C_FUNC_SMBUS_WRITE_WORD_DATA)
	#define BT_I2C_FUNC_SMBUS_BLOCK_DATA		(BT_I2C_FUNC_SMBUS_READ_BLOCK_DATA | \
	 		    			                     BT_I2C_FUNC_SMBUS_WRITE_BLOCK_DATA)
	#define BT_I2C_FUNC_SMBUS_I2C_BLOCK			(BT_I2C_FUNC_SMBUS_READ_I2C_BLOCK | \
	 		    			                     BT_I2C_FUNC_SMBUS_WRITE_I2C_BLOCK)

	#define BT_I2C_FUNC_SMBUS_EMUL				(BT_I2C_FUNC_SMBUS_QUICK | \
					                             BT_I2C_FUNC_SMBUS_BYTE | \
					                             BT_I2C_FUNC_SMBUS_BYTE_DATA | \
					                             BT_I2C_FUNC_SMBUS_WORD_DATA | \
					                             BT_I2C_FUNC_SMBUS_PROC_CALL | \
					                             BT_I2C_FUNC_SMBUS_WRITE_BLOCK_DATA | \
					                             BT_I2C_FUNC_SMBUS_I2C_BLOCK | \
					                             BT_I2C_FUNC_SMBUS_PEC)

	BT_u32		(*pfnMasterTransfer)	(BT_HANDLE hI2C, BT_I2C_MESSAGE *msgs, BT_u32 num, BT_ERROR *pError);

} BT_DEV_IF_I2C;

/*
 *	Define the unified API for I2C devices in BitThunder
 */
BT_u32		BT_I2C_Transfer			(const BT_I2C_BUS *pBus, BT_I2C_MESSAGE *pMessages, BT_u32 ulMessages, BT_ERROR *pError);
BT_u32		BT_I2C_MasterSend		(BT_I2C_CLIENT *pClient, const BT_u8 *pucSource, BT_u32 ulLength, BT_ERROR *pError);
BT_u32		BT_I2C_MasterReceive	(BT_I2C_CLIENT *pClient, BT_u8 *pucDest, BT_u32 ulLength, BT_ERROR *pError);


#endif
