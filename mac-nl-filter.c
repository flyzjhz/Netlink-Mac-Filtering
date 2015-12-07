/*This program achieves MAC-Address list filtering at Driver level using nl80211 architecture*/

#include <libnl3/netlink/netlink.h>
#include <libnl3/netlink/genl/genl.h>
#include <libnl3/netlink/genl/ctrl.h>
#include <libnl3/netlink/socket.h>
#include <net/if.h>

#include <linux/nl80211.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

struct mac_addr_local {
	u8 addr[ETH_LEN];
}

//Put a structure here to hold mac addresses
struct mac_maintain {
 	u8 mac_acl_policy;	
	int num_of_macs;
	const char *mac_list[NL80211_ATTR_MAC_ACL_MAX];
	
};


static int macCallback(struct nl_msg *msg, void *arg)
{
	struct nlmsghdr* ret_hdr = nlmsg_hdr(msg);


}

int main(int argc, char *argv[])
{
	int i,ret = 0, mac_policy;

	if(argc < 3) {
		fprintf(stderr ,"Usage:- ./exe <Mac-Address-List> <Policy -- allow/deny>\n");
		exit(1);
	}

	
	//Get the mac policy
	if(argv[2] == "allow")
		mac_policy = 1;
	else if(argv[2] == "deny")
		mac_policy = 0;	
	else {
		fprintf(stderr, "Undefined Mac Policy\n");
		exit(1);
	}
	
	
	//Read mac-addresses from a file and store in our structure 
	

	struct mac_maintain mac_info;

	//Store the mac's in our list
	for(i = 0)

	
	//Allocate a netlink socket
	nl_sock *sk = nl_socket_alloc();

	//Connect sk to generic netlink
	if(!genl_connect(sk))
		fprintf(stderr ,"Failed to connect/bind to generic netlink socket\n");
		nl_socket_free(sk);
		exit(1);
	}


	//Find the nl80211 driver ID
	nl80211_driverId = genl_ctrl_resolve(sk, "nl80211");

	//Attach a return callback
	nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM,
			macCallback, NULL);

	
	//Form a message
	nl_msg *acl = nlmsg_alloc();
	
	if(!acl)
		return -ENOMEM;
	
	//Add number of mac-addresess to message
	for(i = 0, i < mac_list->num_of_mac_acl ;i++) {
		if(nla_put(acl, i + 1, ETH_ALEN, mac_list->arr[i].addr)) {
			nlmsg_free(acl);
		}

	//Form the final netlink message
	//Setup the message
	nl_msg *final_msg = nlmsg_alloc();
	
	nl80211_commands cmd = NL80211_CMD_SET_MAC_ACL;
	
	if(!(genlmsg_put(final_msg, 0, 0, nl80211_driverId, 0, flags, cmd, 0) ||
	nla_put_32(final_msg, NL80211_ATTR_ACL_POLICY, mac_policy ? NL80211_ACL_POLICY_DENY_UNLESS_LISTED : NL80211_ACL_POLICY_ACCEPT_UNLESS_LISTED) ||
	nla_put_nested(final_msg, NL80211_ATTR_MAC_ADDRS,acl))) {
		nlmsg_free(final_msg);
		nlmsg_free(acl);
		return -ENOMEM;
	}
	
	nlmsg_free(acl);
	
	//Try to send the message to nl80211 and wait for it to finish and in return invoke our callback
	ret = nl_send_auto(sk,final_msg);
	if(ret)
		fprintf(stderr,"Failed to send the netlink message for setting MAC ACL's\n");
	
	//Block for message to return and invoke our handler
	nl_recvmsg_default(sk);

	return ret;
	
}
