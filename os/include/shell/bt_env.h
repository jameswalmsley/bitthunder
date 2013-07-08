#ifndef _BT_ENV_H_
#define _BT_ENV_H_

#include <collections/bt_list.h>

typedef enum _BT_ENV_TYPE {
	BT_ENV_T_INTEGER,
	BT_ENV_T_STRING,
} BT_ENV_TYPE;

typedef struct _BT_ENV_INT {
	BT_u32 i;
} BT_ENV_INT;

typedef struct _BT_ENV_STRING {
	BT_u32	length;
	BT_i8	s[1];
} BT_ENV_STRING;

typedef struct _BT_ENV_VARIABLE {
	struct bt_list_head list;
	BT_ENV_TYPE eType;
	union {
		BT_ENV_INT		*integer;
		BT_ENV_STRING 	*string;
	} o;
	BT_u32	len;
	BT_i8	s[1];
} BT_ENV_VARIABLE;


BT_ERROR BT_ShellSetEnv(const char *name, const char *value, BT_ENV_TYPE eType);
BT_ENV_VARIABLE *BT_ShellGetEnv(const char *name);

#endif
