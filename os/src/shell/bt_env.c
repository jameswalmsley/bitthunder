/**
 *	BootThunder - Environment Variables.
 *
 *	@author James Walmsley
 *
 **/

#include <bitthunder.h>
#include <collections/bt_list.h>
#include <string.h>
#include <shell/bt_env.h>

static BT_LIST_HEAD(vars);

static BT_ENV_VARIABLE *find_starred_var(const char *name, BT_u32 length) {
	struct bt_list_head *pos;
	bt_list_for_each(pos, &vars) {
		BT_ENV_VARIABLE *env = (BT_ENV_VARIABLE *) pos;
		if(env->s[0] != '*') {
			continue;
		}

		if(strlen(env->s) - 1 == length) {
			if(!strncmp(&env->s[1], name, length)) {
				return env;
			}
		}
	}

	return NULL;
}

static BT_ENV_VARIABLE *find_var(const char *name, BT_u32 length) {

	struct bt_list_head *pos;
	bt_list_for_each(pos, &vars) {
		BT_ENV_VARIABLE *env = (BT_ENV_VARIABLE *) pos;
		if(strlen(env->s) == length) {
			if(!strncmp(env->s, name, length)) {
				return env;
			}
		}
	}

	return NULL;
}

BT_ERROR BT_ShellSetEnv(const char *name, const char *value, BT_ENV_TYPE eType) {

	/*
	 *	Currently just assume its a string
	 */

	BT_u32 len = strlen(value) + 1;
	BT_u32 namelen = strlen(name) + 1;

	BT_ENV_VARIABLE *var = find_var(name, namelen-1);
	if(!var) {
		var = BT_kMalloc(sizeof(BT_ENV_VARIABLE) + namelen);
		if(!var) {
			return BT_ERR_NO_MEMORY;
		}

		var->eType = BT_ENV_T_STRING;
		var->len = namelen;
		strncpy(var->s, name, namelen);

		var->o.string = (BT_ENV_STRING *) BT_kMalloc(sizeof(BT_ENV_STRING) + len);
		if(!var->o.string) {
			BT_kFree(var);
			return BT_ERR_NO_MEMORY;
		}

		var->o.string->length = len;
		strncpy(var->o.string->s, value, var->o.string->length);

		bt_list_add(&var->list, &vars);

	} else {
		if(var->eType == BT_ENV_T_STRING && var->o.string->length >= len) {
			strncpy(var->o.string->s, value, var->o.string->length);
		} else {
			BT_kFree(var->o.string);
			var->o.string = (BT_ENV_STRING *) BT_kMalloc(sizeof(BT_ENV_STRING) + len);
			if(!var->o.string) {
				bt_list_del(&var->list);
				BT_kFree(var);
				return BT_ERR_NO_MEMORY;
			}

			var->o.string->length = len;
			strncpy(var->o.string->s, value, var->o.string->length);
		}
	}

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_ShellSetEnv);

BT_ENV_VARIABLE *BT_ShellGetEnv(const char *name) {

	BT_u32 len = strlen(name);


	if(name[0] == '$' && name[1] == '{') {
		return find_var(name+2, len-3);
	}

	return find_var(name, len);
}
BT_EXPORT_SYMBOL(BT_ShellGetEnv);

BT_ENV_VARIABLE *BT_ShellGetStarredEnv(const char *name) {
	BT_u32 len = strlen(name);

	if(name[0] == '$' && name[1] == '{') {
		return find_starred_var(name+2, len-3);
	}

	return find_starred_var(name, len);
}
BT_EXPORT_SYMBOL(BT_ShellGetStarredEnv);

BT_ENV_VARIABLE *BT_ShellGetNextEnv(BT_ENV_VARIABLE *env) {
	BT_ENV_VARIABLE *ret = NULL;

	if(env == NULL) {
		ret = (BT_ENV_VARIABLE *)vars.next;
	} else {
		ret = (BT_ENV_VARIABLE *)((struct bt_list_head *)env)->next;
	}

	if(ret == (BT_ENV_VARIABLE *)&vars) ret = NULL;

	return ret;
}
BT_EXPORT_SYMBOL(BT_ShellGetNextEnv);
