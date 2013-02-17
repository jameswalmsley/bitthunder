.globl _bt_startup_init_hook
_bt_startup_init_hook:
	ldr	r0,=0xDEADBEEF
							@ note, this bypasses the usual BitThunder boot sequence for now,
							@ but it will call main, which is the BitThunder main system entry,
							@ once the low-level configuration is completed.

							@ Eventually the requirement for this will be removed as BitThunder assumes
							@ more and more of the facilities that we are borrowing from libxil :(

