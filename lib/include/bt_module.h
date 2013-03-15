#ifndef _BT_MODULE_H_
#define _BT_MODULE_H_

#define BT_CONFIG_NO_MODULE_STRINGS

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


#endif
