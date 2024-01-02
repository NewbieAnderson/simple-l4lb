#ifndef TABLE_H
#define TABLE_H

#include <linux/string.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/types.h>

#define ERROR_INVALID_IP -1
#define ERROR_INVALID_PORT -2
#define ERROR_INVALID_ARGS -3

struct lb_server_session {
    u32 client_ipv4_addr;
    u16 client_port;
    u16 forward_server_port;
    //struct timespec latest_processed_packet_timestamp;
};

static struct lb_server_session *g_session_table;

static int g_max_table_size;

static int g_session_count;

static int g_table_index;

static int g_session_counts_by_port[65535];

static int *g_ports;

int init_table(int max_table_size);

int destroy_table(void);

struct lb_server_session *insert_to_table(u32 client_ipv4_addr, u16 client_port);

int remove_from_table(u32 client_ipv4_addr, u16 client_port);

static int remove_from_table_by_index(int index);

struct lb_server_session *find_table(u32 client_ipv4_addr, u16 client_port);

inline int update_session_timestamp(struct lb_server_session *session_ptr);

#endif /* TABLE_H */