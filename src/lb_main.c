#include <linux/fs.h>
#include <linux/init.h>
#include <linux/inet.h>
#include <linux/ip.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/tcp.h>
#include <linux/uaccess.h>
#include <linux/udp.h>
#include <net/tcp.h>
#include <linux/ip.h>

#include "./table.h"

/** 
 * balancing loads using round-robin method
*/

#define DEFAULT_INTERNAL_SERVER_PORT 2200

#define ADDR_LOCAL_STRING "127.0.0.1"

static int g_lb_ipv4_addr_int32;
static int g_lb_port;
static int *g_forward_server_ports;
static int g_forward_server_count;

static struct nf_hook_ops g_incoming_nfho;
static struct nf_hook_ops g_outgoing_nfho;

module_param(g_lb_port, int, S_IRUGO);

static int try_reserve_lb_port(u16 port)
{
    struct net *net = &init_net;
    struct inet_hashinfo *hashinfo = inet_gethashinfo(net, IPPROTO_TCP);
    struct sock *sk;
    sk = inet_lookup(net, hashinfo, 0, htonl(INADDR_ANY), 0, htons(port), htonl(INADDR_ANY), 0);
    if (sk) {
        printk(KERN_ERR "Port %u is already in use.\n", port);
        return -EADDRINUSE;
    }
    sk = sock_alloc(net, AF_INET, GFP_KERNEL);
    if (!sk) {
        printk(KERN_ERR "Failed to allocate socket.\n");
        return -ENOMEM;
    }
    sk->sk_protocol = IPPROTO_TCP;
    sk->sk_type = SOCK_STREAM;
    sk->sk_reuse = 1;
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(port),
    };
    inet_bind((struct socket *)sk, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    printk(KERN_INFO "Port %u reserved successfully.\n", port);
    return 0;
}

static unsigned int forward_incoming_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *iph;
    struct tcphdr *tcph;
    u32 tcplen;
    iph = ip_hdr(skb);
    if (skb_is_nonlinear(skb))
        skb_linearize(skb);
    if (iph->protocol == IPPROTO_TCP) {
        tcph = tcp_hdr(skb);
        if (ntohs(tcph->dest) != g_lb_port)
            return NF_ACCEPT;
        tcph->dest = htons(DEFAULT_INTERNAL_SERVER_PORT); // TODO : change here
        skb->csum = 0;
        tcplen = ntohs(iph->tot_len) - iph->ihl * 4;
        tcph->check = 0;
        tcph->check = tcp_v4_check(tcplen, iph->saddr, iph->daddr, csum_partial((u8 *)tcph, tcplen, 0));
    }
    return NF_ACCEPT;
}

static unsigned int forward_outgoing_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *iph;
    struct tcphdr *tcph;
    u32 tcplen;
    iph = ip_hdr(skb);
    if (skb_is_nonlinear(skb))
        skb_linearize(skb);
    if (iph->protocol == IPPROTO_TCP) {
        tcph = tcp_hdr(skb);
        if (ntohs(tcph->source) != DEFAULT_INTERNAL_SERVER_PORT) // TODO : change here
            return NF_ACCEPT;
        tcph->source = htons(g_lb_port);
        skb->csum = 0;
        tcplen = ntohs(iph->tot_len) - iph->ihl * 4;
        tcph->check = 0;
        tcph->check = tcp_v4_check(tcplen, iph->saddr, iph->daddr, csum_partial((u8 *)tcph, tcplen, 0));
    }
    return NF_ACCEPT;
}

static int __init lb_init(void)
{
    int port;
    int ret;
    if (try_reserve_lb_port(g_lb_port) == 0)
        goto continue_reserve_lb_port;
    for (port = 1000; port < 65535; ++port) {
        ret = try_reserve_lb_port(port);
        if (ret == 0)
            break;
    }
    if (ret != 0) {
        printk(KERN_CRIT "[simple-l4lb] All ports are using");
        return -1;
    }
continue_reserve_lb_port:
    g_lb_ipv4_addr_int32 = in_aton(ADDR_LOCAL_STRING);
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