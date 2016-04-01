#include <linux/netlink.h>
#include <libnl3/netlink/genl/genl.h>
#include <libnl3/netlink/genl/ctrl.h>
#include <libnl3/netlink/genl/family.h>
#include <linux/nl80211.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


static struct {
	struct nl_sock *nls;
	int nl80211_id ;
}wifi;


static int list_interface_handler(struct nl_msg *msg, void *arg)
{
	printf("Invoked handler\n");
	struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));

	nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

	if(tb_msg[NL80211_ATTR_IFNAME])
		printf("Interface: %s\n", nla_get_string(tb_msg[NL80211_ATTR_IFNAME]));

	printf("Before returning from handler\n");
	return NL_SKIP;

}

static int finish_handler(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	*ret = 0;
	printf("Finishing handler\n");
	return NL_SKIP;
}

int main(int argc, char *argv[])
{
	int err = 1;

	wifi.nls = nl_socket_alloc();
	if(!wifi.nls) {
		fprintf(stderr, "Failed to allocate netlink socket\n");
		return -ENOMEM;
	}
	
//	nl_socket_set_buffer_size(wifi.nls, 8192, 8192);

	if(genl_connect(wifi.nls)) {
		fprintf(stderr, "Failed to connect to netlink socket\n");
		return -ENOLINK;
	}

	wifi.nl80211_id = genl_ctrl_resolve(wifi.nls, "nl80211");
	if(wifi.nl80211_id < 0)	{
		fprintf(stderr, "nl80211 not found\n");
		return -ENOENT;
	}


	//Allocate a netlink message
	struct nl_msg *msg = nlmsg_alloc();
	if(!msg) {
		fprintf(stderr, "cannot allocate netlink message\n");
		nlmsg_free(msg);
		return -ENOMEM;
	}

	/*Register a callback for netlink message*/
	struct nl_cb *cb = nl_cb_alloc(NL_CB_DEFAULT);
	if(!cb) {
		fprintf(stderr, "Failed to allocate nl callback\n");
		nlmsg_free(msg);
		return -ENOMEM;
	}

	//Setup callback function
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, list_interface_handler, NULL);

	//Setup the netlink generic message
	//genlmsg_put(msg, 0 , 0, wifi.nl80211_id, 0, NLM_F_DUMP, NL80211_CMD_GET_WIPHY, 0);
	genlmsg_put(msg, 0 , 0, wifi.nl80211_id, 0, NLM_F_DUMP, NL80211_CMD_SET_MAC_ACL, 0);

//	nla_put_u32(msg,NL80211_ATTR_ACL_POLICY,NL80211_ACL_POLICY_DENY_UNLESS_LISTED);
//	nla_put_nested(msg,NL80211_ATTR_MAC_ADDRS,msg);


	//Send the message
	nl_send_auto_complete(wifi.nls, msg);

	//Setup finish handler
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);

	while(err > 0) {
		printf("waiting for receive message\n");
		nl_recvmsgs(wifi.nls, cb);
		printf("Received message - Exiting\n");
	}
	
	

	return 0;
}
