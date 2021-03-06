/* 
 * Copyright (C) 2004 Andrew Beekhof <andrew@beekhof.net>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef CRM_COMMON_CLUSTER__H
#  define CRM_COMMON_CLUSTER__H

#  include <crm/common/xml.h>
#  include <crm/common/util.h>

#  if SUPPORT_HEARTBEAT
#    include <heartbeat/hb_api.h>
#    include <ocf/oc_event.h>
#  endif

extern gboolean crm_have_quorum;
extern GHashTable *crm_peer_cache;
extern GHashTable *crm_peer_id_cache;
extern unsigned long long crm_peer_seq;

#  ifndef CRM_SERVICE
#    define CRM_SERVICE PCMK_SERVICE_ID
#  endif

/* *INDENT-OFF* */
#define CRM_NODE_LOST      "lost"
#define CRM_NODE_MEMBER    "member"
#define CRM_NODE_ACTIVE    CRM_NODE_MEMBER
#define CRM_NODE_INACTIVE  CRM_NODE_LOST
#define CRM_NODE_EVICTED   "evicted"

enum crm_proc_flag {
    crm_proc_none      = 0x00000001,

    /* 3 messaging types */
    crm_proc_heartbeat = 0x01000000,
    crm_proc_plugin    = 0x00000002,
    crm_proc_cpg       = 0x04000000,

    crm_proc_lrmd      = 0x00000010,
    crm_proc_cib       = 0x00000100,
    crm_proc_crmd      = 0x00000200,
    crm_proc_attrd     = 0x00001000,

    crm_proc_pe        = 0x00010000,
    crm_proc_te        = 0x00020000,

    crm_proc_stonithd  = 0x00002000,
    crm_proc_stonith_ng= 0x00100000,

    crm_proc_mgmtd     = 0x00040000,
};
/* *INDENT-ON* */

typedef struct crm_peer_node_s {
    uint32_t id;
    uint64_t born;
    uint64_t last_seen;

    int32_t votes;
    uint32_t processes;

    char *uname;
    char *state;
    char *uuid;
    char *addr;
    char *version;
} crm_node_t;

static inline const char *
peer2text(enum crm_proc_flag proc)
{
    const char *text = "unknown";

    switch (proc) {
        case crm_proc_none:
            text = "unknown";
            break;
        case crm_proc_plugin:
            text = "ais";
            break;
        case crm_proc_heartbeat:
            text = "heartbeat";
            break;
        case crm_proc_cib:
            text = "cib";
            break;
        case crm_proc_crmd:
            text = "crmd";
            break;
        case crm_proc_pe:
            text = "pengine";
            break;
        case crm_proc_te:
            text = "tengine";
            break;
        case crm_proc_lrmd:
            text = "lrmd";
            break;
        case crm_proc_attrd:
            text = "attrd";
            break;
        case crm_proc_stonithd:
            text = "stonithd";
            break;
        case crm_proc_stonith_ng:
            text = "stonith-ng";
            break;
        case crm_proc_mgmtd:
            text = "mgmtd";
            break;
        case crm_proc_cpg:
            text = "corosync-cpg";
            break;
    }
    return text;
}

void crm_peer_init(void);
void crm_peer_destroy(void);
char *get_corosync_uuid(uint32_t id, const char *uname);
const char *get_node_uuid(uint32_t id, const char *uname);
int get_corosync_id(int id, const char *uuid);

gboolean crm_cluster_connect(char **our_uname, char **our_uuid, void *dispatch,
                                    void *destroy,
#  if SUPPORT_HEARTBEAT
                                    ll_cluster_t ** hb_conn
#  else
                                    void **unused
#  endif
    );

enum crm_ais_msg_types;
gboolean send_cluster_message(const char *node, enum crm_ais_msg_types service,
                              xmlNode * data, gboolean ordered);

void destroy_crm_node(gpointer/* crm_node_t* */ data);

crm_node_t *crm_get_peer(unsigned int id, const char *uname);

guint crm_active_peers(void);
gboolean crm_is_peer_active(const crm_node_t * node);
guint reap_crm_member(uint32_t id);
int crm_terminate_member(int nodeid, const char *uname, void* unused);
int crm_terminate_member_no_mainloop(int nodeid, const char *uname, int *connection);
gboolean crm_get_cluster_name(char **cname);

#  if SUPPORT_HEARTBEAT
gboolean crm_is_heartbeat_peer_active(const crm_node_t * node);
#  endif

#  if SUPPORT_COROSYNC
extern int ais_fd_sync;
gboolean crm_is_corosync_peer_active(const crm_node_t * node);
gboolean send_ais_text(int class, const char *data, gboolean local,
                              const char *node, enum crm_ais_msg_types dest);
gboolean get_ais_nodeid(uint32_t * id, char **uname);
#  endif

void empty_uuid_cache(void);
const char *get_uuid(const char *uname);
const char *get_uname(const char *uuid);
void set_uuid(xmlNode * node, const char *attr, const char *uname);
void unget_uuid(const char *uname);

enum crm_status_type {
    crm_status_uname,
    crm_status_nstate,
    crm_status_processes,
};

enum crm_ais_msg_types text2msg_type(const char *text);
void crm_set_status_callback(void (*dispatch) (enum crm_status_type, crm_node_t *, const void *));

/* *INDENT-OFF* */
enum cluster_type_e 
{
    pcmk_cluster_unknown     = 0x0001,
    pcmk_cluster_invalid     = 0x0002,
    pcmk_cluster_heartbeat   = 0x0004,
    pcmk_cluster_classic_ais = 0x0010,
    pcmk_cluster_corosync    = 0x0020,
    pcmk_cluster_cman        = 0x0040,
};
/* *INDENT-ON* */

enum cluster_type_e get_cluster_type(void);
const char *name_for_cluster_type(enum cluster_type_e type);

gboolean is_corosync_cluster(void);
gboolean is_cman_cluster(void);
gboolean is_openais_cluster(void);
gboolean is_classic_ais_cluster(void);
gboolean is_heartbeat_cluster(void);

#endif
