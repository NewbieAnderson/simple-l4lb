#ifndef TABLE_H
#define TABLE_H

#include <sys/time.h>

#define ERROR_INVALID_IP -1
#define ERROR_INVALID_PORT -2
#define ERROR_INVALID_ARGS -3

struct lb_server_pair {
    unsigned int lb_ipv4_addr;
    unsigned int server_ipv4_addr;
    unsigned short lb_port;
    unsigned short server_port;
    struct timespec latest_processed_packet_timestamp;
};

static struct lb_server_pair *g_table;

static int g_table_size;

static int g_session_count;

int init_table(int table_size);

int destroy_table(void);

int insert_to_table(unsigned int lb_ipv4_addr, unsigned int server_ipv4_addr,
                    unsigned short lb_port, unsigned short server_port);

int remove_from_table(unsigned int lb_ipv4_addr, unsigned int server_ipv4_addr,
                      unsigned short lb_port, unsigned short server_port);

static int remove_from_table_by_index(int index);

int get_table(const struct lb_server_pair *lb_info_ptr);

#endif /* TABLE_H */