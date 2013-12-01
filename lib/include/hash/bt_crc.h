/**
 *	
 *
 **/

#ifndef _BT_CRC_H_
#define _BT_CRC_H_

struct bt_crc32_context {
	BT_u32 crc32;
};


void bt_crc32(const void *data, BT_u32 nbytes, BT_u8 digest[4]);	// Simple block hash api.

void bt_crc32_init(struct bt_crc32_context *ctx);
void bt_crc32_append(struct bt_crc32_context *ctx, const void *data, BT_u32 nbytes);
void bt_crc32_finish(struct bt_crc32_context *ctx, BT_u8 digest[4]);

#endif
