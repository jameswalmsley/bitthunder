#ifndef _PL310_H_
#define _PL310_H_

#include <bt_struct.h>

typedef struct _PL310_REGS {
	BT_u32 reg0_cache_id;
	BT_u32 reg0_cache_type;

	BT_STRUCT_RESERVED_u32(0, 0x4, 0x100);

	BT_u32 reg1_control;
	BT_u32 reg1_aux_control;
	BT_u32 reg1_tag_ram_control;
	BT_u32 reg1_data_ram_control;

	BT_STRUCT_RESERVED_u32(1, 0x10C, 0x200);

	BT_u32 reg2_ev_counter_control;
	BT_u32 reg2_ev_counter1_cfg;
	BT_u32 reg2_ev_counter0_cfg;
	BT_u32 reg2_ev_counter1;
	BT_u32 reg2_ev_counter0;
	BT_u32 reg2_int_mask;
	BT_u32 reg2_int_mask_status;
	BT_u32 reg2_int_raw_status;
	BT_u32 reg2_int_clear;

	BT_STRUCT_RESERVED_u32(2, 0x220, 0x730);

	BT_u32 reg7_cache_sync;

	BT_STRUCT_RESERVED_u32(3, 0x730, 0x770);

	BT_u32 reg7_inv_pa;

	BT_STRUCT_RESERVED_u32(4, 0x770, 0x77C);

	BT_u32 reg7_inv_way;

	BT_STRUCT_RESERVED_u32(5, 0x77C, 0x7B0);

	BT_u32 reg7_clean_pa;

	BT_u32 reserved00;

	BT_u32 reg7_clean_index;
	BT_u32 reg7_clean_way;

	BT_STRUCT_RESERVED_u32(6, 0x7BC, 0x7F0);

	BT_u32 reg7_clean_inv_pa;

	BT_u32 reserved01;

	BT_u32 reg7_clean_inv_index;
	BT_u32 reg7_clean_inv_way;

	BT_STRUCT_RESERVED_u32(7, 0x7FC, 0x900);

	BT_u32 reg9_d_lockdown0;
	BT_u32 reg9_i_lockdown0;
	BT_u32 reg9_d_lockdown1;
	BT_u32 reg9_i_lockdown1;
	BT_u32 reg9_d_lockdown2;
	BT_u32 reg9_i_lockdown2;
	BT_u32 reg9_d_lockdown3;
	BT_u32 reg9_i_lockdown3;
	BT_u32 reg9_d_lockdown4;
	BT_u32 reg9_i_lockdown4;
	BT_u32 reg9_d_lockdown5;
	BT_u32 reg9_i_lockdown5;
	BT_u32 reg9_d_lockdown6;
	BT_u32 reg9_i_lockdown6;
	BT_u32 reg9_d_lockdown7;
	BT_u32 reg9_i_lockdown7;

	BT_STRUCT_RESERVED_u32(8, 0x93C, 0x950);

	BT_u32 reg9_lock_line_en;
	BT_u32 reg9_unlock_way;

	BT_STRUCT_RESERVED_u32(9, 0x954, 0xC00);

	BT_u32 reg12_addr_filtering_start;
	BT_u32 reg12_addr_filtering_end;

	BT_STRUCT_RESERVED_u32(10, 0xC04, 0xF40);

	BT_u32 reg15_debug_ctrl;

	BT_STRUCT_RESERVED_u32(11, 0xF40, 0xF60);

	BT_u32 reg15_prefetch_ctrl;

	BT_STRUCT_RESERVED_u32(12, 0xF60, 0xF80);

	BT_u32 reg15_power_ctrl;

} PL310_REGS;















#endif
