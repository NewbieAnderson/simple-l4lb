#include "./table.h"

int init_table(int table_size)
{
    return 0;
}

int destroy_table(void)
{
    return 0;
}

int insert_to_table(unsigned int lb_ipv4_addr, unsigned int server_ipv4_addr,
                    unsigned short lb_port, unsigned short server_port)
{
    return 0;
}

int remove_from_table(unsigned int lb_ipv4_addr, unsigned int server_ipv4_addr,
                      unsigned short lb_port, unsigned short server_port)
{
    return 0;
}

static int remove_from_table_by_index(int index)
{
    return 0;
}

int get_table(const struct lb_server_pair *lb_info_ptr)
{
    return 0;
}