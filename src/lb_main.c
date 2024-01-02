#include <linux/init.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/inet.h>
#include <linux/tcp.h>
#include <linux/types.h>
#include <linux/udp.h>
#include <net/tcp.h>

#include "./table.h"

#define DEFAULT_NAT_PORT 3157
#define DEFAULT_SERVER_PORT 4895
#define ADDR_LOACL_STRING "127.0.0.1"

static int g_lb_port;
static int g_server_port;
static int g_lb_ipv4_addr_int32;

static struct nf_hook_ops g_incoming_nfho;
static struct nf_hook_ops g_outgoing_nfho;

/*
static u32 forward_incoming_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct lb_server_session *session_ptr;
    struct iphdr *iph;
    struct tcphdr *tcph;
    struct udphdr *udph;
    u32 dest_port;
    u32 client_ipv4_addr;
    iph = ip_hdr(skb);
    if (iph->daddr != g_lb_ipv4_addr_int32)
        return NF_ACCEPT;
    if (iph->protocol == IPPROTO_TCP) {
        tcph = tcp_hdr(skb);
        dest_port = ntohs(tcph->dest);
        if (dest_port != g_lb_port)
            return NF_ACCEPT;
        //session_ptr = find_table(iph->saddr, tcph->source);
        //if (session_ptr == NULL)
        //    session_ptr = insert_to_table(iph->saddr, tcph->source);
        //else
        //    update_session_timestamp(session_ptr);
        //tcph->dest = htons(session_ptr->forward_server_port);
        tcph->dest = htons(DEFAULT_SERVER_PORT);
        tcph->check = 0;
        tcph->check = tcp_v4_check(sizeof(struct tcphdr), iph->saddr, iph->daddr,
                                   csum_partial(tcph, sizeof(struct tcphdr), 0));
    } else if (iph->protocol == IPPROTO_UDP) {
        udph = udp_hdr(skb);
        dest_port = ntohs(udph->dest);
        if (dest_port != g_lb_port)
            return NF_ACCEPT;
        //session_ptr = find_table(iph->saddr, udph->source);
        //if (session_ptr == NULL)
        //    session_ptr = insert_to_table(iph->saddr, udph->source);
        //else
        //    update_session_timestamp(session_ptr);
        //udph->dest = htons(session_ptr->forward_server_port);
        udph->dest = htons(DEFAULT_SERVER_PORT);
        udph->check = 0;
        udph->check = csum_tcpudp_magic(iph->saddr, iph->daddr, sizeof(struct udphdr),
                                        IPPROTO_UDP, csum_partial(udph, sizeof(struct udphdr), 0));
        if (udph->check == 0)
            udph->check = 0xFFFF;
    } else {
        return NF_ACCEPT;
    }
    iph->check = 0;
    iph->check = ip_fast_csum(iph, iph->ihl);
    return NF_ACCEPT;
}

static u32 forward_outgoing_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct lb_server_session *session_ptr;
    struct iphdr *iph;
    struct tcphdr *tcph;
    struct udphdr *udph;
    u32 source_port;
    u32 client_ipv4_addr;
    iph = ip_hdr(skb);
    if (iph->daddr != g_lb_ipv4_addr_int32)
        return NF_ACCEPT;
    if (iph->protocol == IPPROTO_TCP) {
        tcph = tcp_hdr(skb);
        source_port = ntohs(tcph->dest);
        if (source_port != g_server_port)
            return NF_ACCEPT;
        tcph->source = htons(DEFAULT_NAT_PORT);
        tcph->check = 0;
        tcph->check = tcp_v4_check(sizeof(struct tcphdr), iph->saddr, iph->daddr,
                                    csum_partial(tcph, sizeof(struct tcphdr), 0));
    } else if (iph->protocol == IPPROTO_UDP) {
        udph = udp_hdr(skb);
        source_port = ntohs(udph->dest);
        if (source_port != g_server_port)
            return NF_ACCEPT;
        udph->source = htons(DEFAULT_NAT_PORT);
        udph->check = 0;
        udph->check = csum_tcpudp_magic(iph->saddr, iph->daddr, sizeof(struct udphdr),
                                        IPPROTO_UDP, csum_partial(udph, sizeof(struct udphdr), 0));
        if (udph->check == 0)
            udph->check = 0xFFFF;
    } else {
        return NF_ACCEPT;
    }
    iph->check = 0;
    iph->check = ip_fast_csum(iph, iph->ihl);
    return NF_ACCEPT;
}
*/

static unsigned int forward_incoming_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *iph;
    struct tcphdr *tcph;
    iph = ip_hdr(skb);
    if (ntohs(tcph->dest) != DEFAULT_NAT_PORT)
        return NF_ACCEPT;
    if (iph->protocol == IPPROTO_TCP) {
        tcph = tcp_hdr(skb);
        tcph->dest = htons(DEFAULT_SERVER_PORT);
        tcph->check = 0;
        iph->check = 0;
        tcph->check = tcp_v4_check(sizeof(struct tcphdr), iph->saddr, iph->daddr,
                                    csum_partial(tcph, sizeof(struct tcphdr), 0));
        iph->check = ip_fast_csum(iph, iph->ihl);
    } else if (iph->protocol == IPPROTO_UDP) {
        ;;
    }
    return NF_ACCEPT;
}

static unsigned int forward_outgoing_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *iph;
    struct tcphdr *tcph;
    iph = ip_hdr(skb);
    if (ntohs(tcph->source) != DEFAULT_SERVER_PORT)
        return NF_ACCEPT;
    if (iph->protocol == IPPROTO_TCP) {
        tcph = tcp_hdr(skb);
        tcph->source = htons(DEFAULT_NAT_PORT);
        tcph->check = 0;
        iph->check = 0;
        tcph->check = tcp_v4_check(sizeof(struct tcphdr), iph->saddr, iph->daddr,
                                    csum_partial(tcph, sizeof(struct tcphdr), 0));
        iph->check = ip_fast_csum(iph, iph->ihl);
    } else if (iph->protocol == IPPROTO_UDP) {
        ;;
    }
    return NF_ACCEPT;
}

static int __init lb_init(void)
{
    g_lb_ipv4_addr_int32 = in_aton(ADDR_LOACL_STRING);
    g_incoming_nfho.hook = forward_incoming_hook;
    g_incoming_nfho.hooknum = NF_INET_PRE_ROUTING;
    g_incoming_nfho.pf = PF_INET;
    g_incoming_nfho.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &g_incoming_nfho);
    g_outgoing_nfho.hook = forward_outgoing_hook;
    g_outgoing_nfho.hooknum = NF_INET_POST_ROUTING;
    g_outgoing_nfho.pf = PF_INET;
    g_outgoing_nfho.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &g_outgoing_nfho);
    return 0;
}

static void __exit lb_exit(void)
{
    nf_unregister_net_hook(&init_net, &g_incoming_nfho);
    nf_unregister_net_hook(&init_net, &g_outgoing_nfho);
}

module_init(lb_init);
module_exit(lb_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NewbieAnderson");
MODULE_DESCRIPTION("[simple-l4lb], Layer 4 Loadbalancer operated by Netfilter Hook");