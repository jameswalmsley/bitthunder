#ifndef _BT_INTERFACES_H_
#define _BT_INTERFACES_H_

#include "interfaces/bt_if_device.h"
#include "interfaces/bt_if_module.h"

typedef BT_ERROR (*BT_HANDLE_CLEANUP)	(BT_HANDLE hHandle);

#define BT_MODULE_NAME					g_m_ModuleName
#define BT_MODULE_DESCRIPTION			g_m_ModuleDescription
#define BT_MODULE_AUTHOR				g_m_ModuleAuthor
#define BT_MODULE_EMAIL					g_m_ModuleEmail

#define BT_DEF_MODULE_NAME(title)		static const BT_s8 BT_MODULE_NAME[] 		= title;
#define BT_DEF_MODULE_DESCRIPTION(info)	static const BT_s8 BT_MODULE_DESCRIPTION[] 	= info;
#define BT_DEF_MODULE_AUTHOR(name) 		static const BT_s8 BT_MODULE_AUTHOR[] 		= name;
#define BT_DEF_MODULE_EMAIL(email) 		static const BT_s8 BT_MODULE_EMAIL[] 		= email;

typedef enum _BT_HANDLE_TYPE {
	BT_HANDLE_T_INVALID 	= 0,
	BT_HANDLE_T_MODULE,
	BT_HANDLE_T_DEVICE,
} BT_HANDLE_TYPE;

typedef struct _BT_HANDLE_INTERFACE *BT_HANDLE_INTERFACE;

typedef union _BT_IF_INTERFACES {
	BT_HANDLE_INTERFACE	p;
#ifdef BT_CONFIG_OS
	const BT_IF_DEVICE *pDevIF;
#endif
} BT_UN_IFS;

typedef struct _BT_IF_HANDLE {
	const BT_s8		   *szpModuleName;
	const BT_s8		   *szpDescription;
	const BT_s8		   *szpAuthor;
	const BT_s8		   *szpEmail;

	const BT_UN_IFS	   *pIfs;

	BT_HANDLE_TYPE 		eType;
	BT_HANDLE_CLEANUP	pfnCleanup;
} BT_IF_HANDLE;


#endif
