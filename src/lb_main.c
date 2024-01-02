#include <linux/init.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/inet.h>
#include <linux/skbuff.h>
#include <net/tcp.h>

#include "./table.h"

#define DEFAULT_NAT_PORT 3157
#define DEFAULT_SERVER_PORT 4895
#define ADDR_LOCAL_STRING "127.0.0.1"

static int g_lb_port;
static int g_server_port;
static int g_lb_ipv4_addr_int32;

static struct nf_hook_ops g_incoming_nfho;
static struct nf_hook_ops g_outgoing_nfho;

static unsigned int forward_incoming_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *iphdr;
    struct tcphdr *tcphdr;
    iphdr = ip_hdr(skb);
    if (skb_is_nonlinear(skb))
        skb_linearize(skb);
    if (iphdr->protocol == IPPROTO_TCP) {
        tcphdr = tcp_hdr(skb);
        if (ntohs(tcphdr->dest) != DEFAULT_NAT_PORT)
            return NF_ACCEPT;
        tcphdr->dest = htons(DEFAULT_SERVER_PORT);
        skb->csum = 0;
        unsigned int tcplen = ntohs(iphdr->tot_len) - iphdr->ihl * 4;
        tcphdr->check = 0;
        tcphdr->check = tcp_v4_check(tcplen, iphdr->saddr, iphdr->daddr, csum_partial((char *)tcphdr, tcplen, 0));
    }
    return NF_ACCEPT;
}

static unsigned int forward_outgoing_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *iphdr;
    struct tcphdr *tcphdr;
    iphdr = ip_hdr(skb);
    if (skb_is_nonlinear(skb))
        skb_linearize(skb);
    if (iphdr->protocol == IPPROTO_TCP) {
        tcphdr = tcp_hdr(skb);
        if (ntohs(tcphdr->source) != DEFAULT_SERVER_PORT)
            return NF_ACCEPT;
        tcphdr->source = htons(DEFAULT_NAT_PORT);
        skb->csum = 0;
        int tcplen = ntohs(iphdr->tot_len) - iphdr->ihl * 4;
        tcphdr->check = 0;
        tcphdr->check = tcp_v4_check(tcplen, iphdr->saddr, iphdr->daddr, csum_partial((char *)tcphdr, tcplen, 0));
    }
    return NF_ACCEPT;
}

static int __init lb_init(void)
{
    printk("trying to load...\n");
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
    printk("loaded!\n");
    return 0;
}

static void __exit lb_exit(void)
{
    nf_unregister_net_hook(&init_net, &g_incoming_nfho);
    nf_unregister_net_hook(&init_net, &g_outgoing_nfho);
    printk("unloaded!\n");
}

module_init(lb_init);
module_exit(lb_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NewbieAnderson");
MODULE_DESCRIPTION("[simple-l4lb], Layer 4 Loadbalancer operated by Netfilter Hook");