/* 
 * Copyright (C) 2004 Andrew Beekhof <andrew@beekhof.net>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef STONITH_NG__H
#define STONITH_NG__H

#include <crm/common/ipc.h>
#include <crm/common/xml.h>

enum stonith_state {
    stonith_connected_command,
    stonith_connected_query,
    stonith_disconnected
};

enum stonith_call_options {
    st_opt_none            = 0x00000000,
    st_opt_verbose         = 0x00000001,
    st_opt_allow_suicide   = 0x00000002,
    st_opt_local_first     = 0x00000004,
    st_opt_discard_reply   = 0x00000010,
    st_opt_scope_local     = 0x00000100,
    st_opt_sync_call       = 0x00001000,
};

#define stonith_default_options = stonith_none

enum stonith_errors {
    stonith_ok				=  0,
    st_err_generic			= -1,
    st_err_internal			= -2,
    st_err_not_supported		= -3,
    st_err_connection			= -4,
    st_err_missing			= -5,
    st_err_exists			= -6,
    st_err_timeout			= -7,
    st_err_ipc				= -8,
    st_err_peer				= -9,
    st_err_unknown_operation		= -10,
    st_err_unknown_device		= -11,
    st_err_unknown_port			= -12,
    st_err_none_available		= -13,
    st_err_authentication		= -14,
    st_err_signal			= -15,
};

#define F_STONITH_CLIENTID		"st_clientid"
#define F_STONITH_CALLOPTS		"st_callopt"
#define F_STONITH_CALLID		"st_callid"
#define F_STONITH_CALLDATA		"st_calldata"
#define F_STONITH_OPERATION		"st_op"
#define F_STONITH_TARGET		"st_target"
#define F_STONITH_REMOTE		"st_remote_op"
#define F_STONITH_RC			"st_rc"
#define F_STONITH_TIMEOUT		"st_timeout"
#define F_STONITH_CALLBACK_TOKEN	"st_async_id"
#define F_STONITH_CLIENTNAME		"st_clientname"
#define F_STONITH_NOTIFY_TYPE		"st_notify_type"
#define F_STONITH_NOTIFY_ACTIVATE	"st_notify_activate"
#define F_STONITH_NOTIFY_DEACTIVATE	"st_notify_deactivate"
#define F_STONITH_DELEGATE		"st_delegate"

#define T_STONITH_NG		"stonith-ng"
#define T_STONITH_REPLY		"st-reply"

#define F_STONITH_DEVICE	"st_device_id"
#define F_STONITH_ACTION	"st_device_action"


#define T_STONITH_NOTIFY		"st_notify"
#define T_STONITH_NOTIFY_DISCONNECT	"st_notify_disconnect"

#define STONITH_ATTR_ARGMAP	"pcmk_arg_map"
#define STONITH_ATTR_HOSTMAP	"pcmk_host_map"
#define STONITH_ATTR_HOSTLIST	"pcmk_host_list"
#define STONITH_ATTR_HOSTCHECK	"pcmk_host_check"

#define STONITH_ATTR_ACTION_OP	"option" /* To be replaced by 'action' at some point */

#define STONITH_ATTR_LIST_OP	"pcmk_list_cmd"
#define STONITH_ATTR_STATUS_OP	"pcmk_status_cmd"
#define STONITH_ATTR_MONITOR_OP	"pcmk_monitor_cmd"

#define STONITH_OP_EXEC		"st_execute"
#define STONITH_OP_QUERY	"st_query"
#define STONITH_OP_FENCE	"st_fence"
#define STONITH_OP_DEVICE_ADD	"st_device_register"
#define STONITH_OP_DEVICE_DEL	"st_device_remove"
#define STONITH_OP_DEVICE_METADATA "st_device_metadata"

#define stonith_channel			"st_command"
#define stonith_channel_callback	"st_callback"

typedef struct stonith_s stonith_t;

typedef struct stonith_api_operations_s
{
	int (*free) (stonith_t *st);
	int (*connect) (stonith_t *st, const char *name, int *async_fd, int *sync_fd);
	int (*disconnect)(stonith_t *st);

	int (*remove_device)(
	    stonith_t *st, int options, const char *name);
	int (*register_device)(
	    stonith_t *st, int options, const char *id,
	    const char *namespace, const char *agent, GHashTable *parameters);

	int (*metadata)(stonith_t *st, int options,
			const char *device, const char *namespace, char **output, int timeout);
	int (*call)(stonith_t *st, int options, const char *id,
		    const char *action, const char *port, int timeout);

	int (*query)(stonith_t *st, int options, const char *node, GListPtr *devices, int timeout);
	int (*fence)(stonith_t *st, int options, const char *node, GHashTable *parameters, const char *action, int timeout);
		
	int (*register_notification)(
	    stonith_t *st, const char *event,
	    void (*notify)(stonith_t *st, const char *event, xmlNode *msg));
	int (*remove_notification)(stonith_t *st, const char *event);

	int (*register_callback)(
	    stonith_t *st, int call_id, int timeout, gboolean only_success,
	    void *userdata, const char *callback_name,
	    void (*callback)(stonith_t *st, const xmlNode *msg, int call, int rc, xmlNode *output, void *userdata));
	int (*remove_callback)(stonith_t *st, int call_id, gboolean all_callbacks);
	
} stonith_api_operations_t;

struct stonith_s
{
	enum stonith_state	state;

	int   call_id;
	int   call_timeout;
	void  *private;
	
	GList *notify_list;

	stonith_api_operations_t *cmds;
};

/* Core functions */
extern stonith_t *stonith_api_new(void);
extern void stonith_api_delete(stonith_t *st);

extern const char *stonith_error2string(enum stonith_errors return_code);
extern void stonith_dump_pending_callbacks(stonith_t *st);

/* internal details - move elsewhere */

typedef struct async_command_s 
{

	int id;
	int stdout;
	int options;

	char *op;
	char *origin;
	char *client;
	char *remote;

	char *victim;
	char *action;
	char *device;
	
	GListPtr device_list;
	GListPtr device_next;

	ProcTrack_ops *pt_ops;
	GHashTable *node_attrs;

} async_command_t;

extern int run_stonith_agent(
    const char *agent, GHashTable *dev_hash, GHashTable *node_hash, const char *action, const char *victim,
    int *agent_result, char **output, async_command_t *track);

extern gboolean is_redhat_agent(const char *agent);
extern const char *get_stonith_provider(const char *agent, const char *provider);

#endif
