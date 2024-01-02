#include <linux/init.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/tcp.h>

#include "./table.h"

#define NAT_PORT 3157
#define SERVER_PORT 4895

static struct nf_hook_ops g_incoming_nfho;
static struct nf_hook_ops g_outgoing_nfho;

static unsigned int process_incoming_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *iph;
    struct tcphdr *tcph;
    iph = ip_hdr(skb);
    if (ntohs(tcph->dest) != NAT_PORT)
        return NF_ACCEPT;
    if (iph->protocol == IPPROTO_TCP) {
        tcph = tcp_hdr(skb);
        tcph->dest = htons(SERVER_PORT);
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

static unsigned int process_outgoing_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *iph;
    struct tcphdr *tcph;
    iph = ip_hdr(skb);
    if (ntohs(tcph->source) != SERVER_PORT)
        return NF_ACCEPT;
    if (iph->protocol == IPPROTO_TCP) {
        tcph = tcp_hdr(skb);
        tcph->source = htons(NAT_PORT);
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

static int __init nat_init(void)
{
    g_incoming_nfho.hook = process_incoming_hook;
    g_incoming_nfho.hooknum = NF_INET_PRE_ROUTING;
    g_incoming_nfho.pf = PF_INET;
    g_incoming_nfho.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &g_incoming_nfho);
    g_outgoing_nfho.hook = process_outgoing_hook;
    g_outgoing_nfho.hooknum = NF_INET_POST_ROUTING;
    g_outgoing_nfho.pf = PF_INET;
    g_outgoing_nfho.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &g_outgoing_nfho);
    return 0;
}

static void __exit nat_exit(void)
{
    nf_unregister_net_hook(&init_net, &incoming_nfho);
    nf_unregister_net_hook(&init_net, &outgoing_nfho);
}

module_init(nat_init);
module_exit(nat_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("geek");
MODULE_DESCRIPTION("NAT module for port translation");