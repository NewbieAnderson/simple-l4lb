#include "./table.h"

int init_table(int max_table_size, int server_size)
{
    if (max_table_size <= 0)
        return -1;
    g_session_table = kzalloc(sizeof(struct lb_server_session) * max_table_size, GFP_KERNEL);
    if (g_session_table == NULL) {
        printk("[L4LB] init_table() - failed to allocate memory using kzalloc(..., GFP_KERNEL)\n");
        kfree(g_session_table);
        return -1;
    }
    g_max_table_size = max_table_size;
    g_session_count = 0;
    g_table_index = 0;
    memset(g_session_counts_by_port, 0, sizeof(int) * 65535);
    g_ports = kzalloc(sizeof(int) * server_size, GFP_KERNEL);
    if (g_ports == NULL) {
        printk("[L4LB] init_table() - failed to allocate memory using kzalloc(..., GFP_KERNEL)\n");
        kfree(g_ports);
        return -1;
    }
    return 0;
}

int destroy_table(void)
{
    memset(g_session_table, 0, sizeof(struct lb_server_session) * g_max_table_size);
    kfree(g_session_table);
    g_session_table = NULL;
    g_max_table_size = 0;
    g_session_count = 0;
    g_table_index = 0;
    return 0;
}

struct lb_server_session *insert_to_table(u32 client_ipv4_addr, u16 client_port)
{
    if (g_session_count >= g_max_table_size)
        return -1;
    while (g_session_table[g_table_index].forward_server_port != 0)
        g_table_index = (g_table_index + 1) % g_max_table_size;
    g_session_table[g_table_index].client_ipv4_addr = client_ipv4_addr;
    g_session_table[g_table_index].client_port = client_port;
    g_session_table[g_table_index].forward_server_port = ; //forward_server_port;
    update_session_timestamp(&g_session_table[g_table_index]);
    ++g_session_count;
    ++g_table_index;
    return 0;
}

int remove_from_table(u32 client_ipv4_addr, u16 client_port)
{
    int i;
    for (i = 0; i < g_max_table_size; ++i) {
        if (g_session_table[i].client_ipv4_addr != client_ipv4_addr)
            continue;
        if (g_session_table[i].client_port != client_port)
            continue;
        g_session_table[i].forward_server_port = 0;
        return 0;
    }
    return -1;
}

static int remove_from_table_by_index(int index)
{
    if (index < 0 || index >= g_max_table_size)
        return -1;
    g_session_table[index].forward_server_port = 0;
    --g_session_count;
    return 0;
}

struct lb_server_session *find_table(u32 client_ipv4_addr, u16 client_port)
{
    return NULL;
}

inline int update_session_timestamp(struct lb_server_session *session_ptr)
{
    if (session_ptr == NULL || session_ptr < g_session_table ||
        session_ptr >= g_session_table + sizeof(struct lb_server_session) * g_max_table_size)
        return -1;
    //getnstimeofday(&session_ptr->latest_processed_packet_timestamp);
    return 0;
}