/**
 *	Bitops for BitThunder:
 *
 *	@ref Linux kernel.
 *
 *	This api is intended to mimic the linux kernel's bitops api,
 * 	and therefore the implementation, although simplified, also closely
 *	resembles that of the linux kernel.
 *
 **/

#ifndef _BT_BITOPS_H
#define _BT_BITOPS_H

#include <bt_kernel.h>

#define BT_BITS_PER_LONG		(sizeof(BT_u32) * 8)
#define BT_BIT(nr)				(1UL << (nr))
#define BT_BIT_MASK(nr)			(1UL << ((nr) % BT_BITS_PER_LONG))
#define BT_BIT_WORD(nr)			((nr) / BT_BITS_PER_LONG)
#define BT_BITS_PER_BYTE		8
#define BT_BITS_TO_LONG(nr)		BT_DIV_ROUND_UP(nr, BT_BITS_PER_BYTE * sizeof(BT_u32))

static inline void __bt_set_bit(BT_u32 nr, volatile BT_u32 *addr) {
	BT_u32 mask = BT_BIT_MASK(nr);
	BT_u32 *p = ((BT_u32 *)addr) + BT_BIT_WORD(nr);

	*p  |= mask;
}

static inline void __bt_clear_bit(BT_u32 nr, volatile BT_u32 *addr) {
	BT_u32 mask = BT_BIT_MASK(nr);
	BT_u32 *p = ((BT_u32 *)addr) + BT_BIT_WORD(nr);

	*p &= ~mask;
}

static inline void __bt_change_bit(BT_u32 nr, volatile BT_u32 *addr) {
	BT_u32 mask = BT_BIT_MASK(nr);
	BT_u32 *p = ((BT_u32 *)addr) + BT_BIT_WORD(nr);

	*p ^= mask;
}

static inline int __bt_test_and_set_bit(BT_u32 nr, volatile BT_u32 *addr) {
	BT_u32 mask = BT_BIT_MASK(nr);
	BT_u32 *p = ((BT_u32 *)addr) + BT_BIT_WORD(nr);
	BT_u32 old = *p;

	*p = old | mask;
	return (old & mask) != 0;
}

static inline int __bt_test_and_clear_bit(BT_u32 nr, volatile BT_u32 *addr) {
	BT_u32 mask = BT_BIT_MASK(nr);
	BT_u32 *p = ((BT_u32 *)addr) + BT_BIT_WORD(nr);
	BT_u32 old = *p;

	*p = old & ~mask;
	return (old & mask) != 0;
}

static inline int __bt_test_and_change_bit(BT_u32 nr, volatile BT_u32 *addr) {
	BT_u32 mask = BT_BIT_MASK(nr);
	BT_u32 *p = ((BT_u32 *)addr) + BT_BIT_WORD(nr);
	BT_u32 old = *p;

	*p = old ^ mask;
	return (old & mask) != 0;
}

static inline int bt_test_bit(BT_u32 nr, const volatile BT_u32 *addr) {
	return 1UL & (addr[BT_BIT_WORD(nr)] >> (nr & (BT_BITS_PER_LONG-1)));
}

#ifndef BT_CONFIG_SMP
#define  _bitops_lock() 	do { BT_kEnterCritical();} while (0)
#define	 _bitops_unlock() 	do { BT_kExitCritical(); } while (0)
#else
#error "Bitops does not currently support SMP systems."
#endif

static inline void bt_set_bit(BT_u32 nr, volatile BT_u32 *addr) {
        BT_u32 mask = BT_BIT_MASK(nr);
        BT_u32 *p = ((BT_u32 *)addr) + BT_BIT_WORD(nr);

        _bitops_lock();
        *p  |= mask;
        _bitops_unlock();
}

static inline void bt_clear_bit(BT_u32 nr, volatile BT_u32 *addr) {
        BT_u32 mask = BT_BIT_MASK(nr);
        BT_u32 *p = ((BT_u32 *)addr) + BT_BIT_WORD(nr);

        _bitops_lock();
        *p &= ~mask;
        _bitops_unlock();
}

static inline void bt_change_bit(BT_u32 nr, volatile BT_u32 *addr) {
        BT_u32 mask = BT_BIT_MASK(nr);
        BT_u32 *p = ((BT_u32 *)addr) + BT_BIT_WORD(nr);

        _bitops_lock();
        *p ^= mask;
        _bitops_unlock();
}

static inline int bt_test_and_set_bit(BT_u32 nr, volatile BT_u32 *addr) {
        BT_u32 mask = BT_BIT_MASK(nr);
        BT_u32 *p = ((BT_u32 *)addr) + BT_BIT_WORD(nr);
        BT_u32 old;

        _bitops_lock();
        old = *p;
        *p = old | mask;
        _bitops_unlock();

        return (old & mask) != 0;
}

static inline int bt_test_and_clear_bit(BT_u32 nr, volatile BT_u32 *addr) {
        BT_u32 mask = BT_BIT_MASK(nr);
        BT_u32 *p = ((BT_u32 *)addr) + BT_BIT_WORD(nr);
        BT_u32 old;

        _bitops_lock();
        old = *p;
        *p = old & ~mask;
        _bitops_unlock();

        return (old & mask) != 0;
}

static inline int bt_test_and_change_bit(BT_u32 nr, volatile BT_u32 *addr) {
        BT_u32 mask = BT_BIT_MASK(nr);
        BT_u32 *p = ((BT_u32 *)addr) + BT_BIT_WORD(nr);
        BT_u32 old;

        _bitops_lock();
        old = *p;
        *p = old ^ mask;
        _bitops_unlock();

        return (old & mask) != 0;
}

#endif
