#ifndef _BT_ARCH_TYPES_H_
#define _BT_ARCH_TYPES_H_

typedef unsigned long long	BT_u64;
typedef long long			BT_i64;
typedef signed long long	BT_s64;

typedef unsigned long		BT_u32;
typedef signed long			BT_s32;
typedef long				BT_i32;

typedef unsigned short		BT_u16;
typedef signed short		BT_s16;
typedef short				BT_i16;

typedef unsigned char		BT_u8;
typedef signed char			BT_s8;
typedef	char				BT_i8;

/*
 *	Kernel Mode types used for MM code.
 */
typedef BT_u32				bt_paddr_t;		///< Physical address type.
typedef	BT_u32				bt_vaddr_t;		///< Virtual address type.
typedef BT_u32				*bt_pgd_t;		///< Page Global Directory type. (Opaque pointer).
typedef BT_u32				*bt_pte_t;		///< Page table entry type.

#endif
