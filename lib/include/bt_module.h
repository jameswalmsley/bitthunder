#ifndef _BT_MODULE_H_
#define _BT_MODULE_H_

#define BT_MODULE_NAME					g_m_ModuleName
#define BT_DEF_MODULE_NAME(title)		static const BT_i8 BT_MODULE_NAME[] 		= title;

#ifdef BT_CONFIG_NO_MODULE_STRINGS
	#define BT_MODULE_DESCRIPTION
	#define BT_MODULE_AUTHOR
	#define BT_MODULE_EMAIL

	#define BT_DEF_MODULE_DESCRIPTION(info)
	#define BT_DEF_MODULE_AUTHOR(name)
	#define BT_DEF_MODULE_EMAIL(email)
#else
	#define BT_MODULE_DESCRIPTION			g_m_ModuleDescription
	#define BT_MODULE_AUTHOR				g_m_ModuleAuthor
	#define BT_MODULE_EMAIL					g_m_ModuleEmail

	#define BT_DEF_MODULE_DESCRIPTION(info)	static const BT_i8 BT_MODULE_DESCRIPTION[] 	= info;
	#define BT_DEF_MODULE_AUTHOR(name) 		static const BT_i8 BT_MODULE_AUTHOR[] 		= name;
	#define BT_DEF_MODULE_EMAIL(email) 		static const BT_i8 BT_MODULE_EMAIL[] 		= email;
#endif


#ifdef BT_CONFIG_NO_MODULE_STRINGS
	typedef struct _BT_MODULE_INFO {
		const BT_i8		   *szpModuleName;
	} BT_MODULE_INFO;

	#define BT_MODULE_DEF_INFO							\
		{												\
			BT_MODULE_NAME,								\
		}
#else
	typedef struct _BT_MODULE_INFO {
		const BT_i8		   *szpModuleName;
		const BT_i8		   *szpDescription;
		const BT_i8		   *szpAuthor;
		const BT_i8		   *szpEmail;
	} BT_MODULE_INFO;

	#define BT_MODULE_DEF_INFO							\
		{												\
			BT_MODULE_NAME,								\
			BT_MODULE_DESCRIPTION,						\
			BT_MODULE_AUTHOR,							\
			BT_MODULE_EMAIL,							\
		}
#endif

struct bt_kernel_symbol {
	void 			*value;
	const BT_i8 	*name;
};

#define BT_MODULE_SYMBOL_PREFIX	""

#define __BT_EXPORT_SYMBOL(sym, sec)						\
	extern typeof(sym) sym;								\
    static const char __bt_kstrtab_##sym[]				\
   __attribute__((section(".bt.ksymtab_strings"), aligned(1))) \
   = BT_MODULE_SYMBOL_PREFIX #sym;	\
	static const struct bt_kernel_symbol __bt_ksymtab_##sym	\
	__attribute__((__used__))								\
	__attribute__((section(".bt.ksymtab" sec), unused))	\
	= { (void *) &sym, __bt_kstrtab_##sym }


#ifdef BT_CONFIG_KERNEL_SYMBOLS
#define BT_EXPORT_SYMBOL(sym)				__BT_EXPORT_SYMBOL(sym, "")
#define BT_EXPORT_SYMBOL_GPL(sym)			__BT_EXPORT_SYMBOL(sym, "_gpl")
#define BT_EXPORT_SYMBOL_BSD(sym)			__BT_EXPORT_SYMBOL(sym, "_bsd")
#define BT_EXPORT_SYMBOL_PROPRIETARY(sym)	__BT_EXPORT_SYMBOL(sym, "_proprietary")
#else
#define BT_EXPORT_SYMBOL(sym)
#define BT_EXPORT_SYMBOL_GPL(sym)
#define BT_EXPORT_SYMBOL_BSD(sym)
#define BT_EXPORT_SYMBOL_PROPRIETARY(sym)
#endif

#endif
