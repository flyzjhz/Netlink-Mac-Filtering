/*This program achieves MAC-Address list filtering at Driver level using nl80211 architecture*/

#include <libnl3/netlink/netlink.h>
#include <libnl3/netlink/genl/genl.h>
#include <libnl3/netlink/genl/ctrl.h>
#include <libnl3/netlink/socket.h>
#include <net/if.h>
#include <linux/if_ether.h>

#include <linux/nl80211.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/types.h>

#define MAC_FILE_PATH "file_mac"
#define MAC_ADDR_SIZE 19 //18 characters

struct mac_addr_local {
	uint8_t addr[ETH_ALEN];
};

//Put a structure here to hold mac addresses
struct mac_maintain {
 	uint8_t mac_acl_policy;	
	unsigned int num_of_mac_for_acl;
	struct mac_addr_local mac_acl[0];
	char *mac_list[MAC_ADDR_SIZE];
	
};


struct local_nl {
	struct nl_sock *sk;
	int nl80211_driverId;
}local_nl;
	

static int macCallback_handler(struct nl_msg *msg, void *arg)
{
	struct nlmsghdr* ret_hdr = nlmsg_hdr(msg);
	printf("Call back invoked - indicating we got something back from kernel\n");
	return NL_SKIP;
}
static int macCallback_finish_handler(struct nl_msg *msg, void *arg)
{
	struct nlmsghdr* ret_hdr = nlmsg_hdr(msg);
	printf("Call back invoked - indicating we got something back from kernel finish-one\n");
	return NL_SKIP;
}

int main(int argc, char *argv[])
{
	int i = 0,ret = 0,  err = 1;
	FILE *fp;
	char *policy;
	char *mac_addr;

	if(argc < 3) {
		fprintf(stderr ,"Usage:- ./exe <Mac-Address-file> <Policy -- allow/deny>\n");
		exit(1);
	}
	
	policy = argv[2];

	mac_addr = argv[1];
	
	/*Get the mac policy
	if(!strcmp(policy,"allow"))
		mac_policy = 1;
	else if(!strcmp(policy,"deny"))
		mac_policy = 0;	
	else {
		fprintf(stderr, "Undefined Mac Policy\n");
		exit(1);
	}*/
	
	
	/*Read mac-addresses from a file and store in our structure 
	fp = fopen(MAC_FILE_PATH, "r");
	if(!fp){
		fprintf(stderr, "Failed to open mac address file\n");
		exit(1);
	}
	else
		printf("Mac-Address file opened successfully with FD=%d\n",fp);



	struct mac_maintain mac_info;

	//Store the mac's in our list
	while(fgets(mac_info.mac_list, MAC_ADDR_SIZE,fp) != NULL) { 
		printf("Mac addresses read from File is %s",mac_info.mac_list);
		mac_info.num_of_mac_for_acl = i++;
	//	mac_info.mac_acl[i] = mac_info.mac_list;
	}*/


	/*/Print the mac addresses stored in our array
	for(i = 0; i < mac_info.num_of_mac_for_acl; i++){
		printf("Mac addresses stored are %s",mac_info.mac_acl[i]);
	}*/
	
	//Allocate a netlink socket
	local_nl.sk = nl_socket_alloc();

	//Connect sk to generic netlink
	if(genl_connect(local_nl.sk) != 0){
		fprintf(stderr ,"Failed to connect/bind to generic netlink socket\n");
		nl_socket_free(local_nl.sk);
		exit(1);
	}


	//Find the nl80211 driver ID
	local_nl.nl80211_driverId = genl_ctrl_resolve(local_nl.sk, "nl80211");

	//Attach a return callback
	/*nl_socket_modify_cb(local_nl.sk, NL_CB_VALID, NL_CB_CUSTOM,
			macCallback, NULL);*/


	//Register a callback for return netlink message
	struct nl_cb *cb = nl_cb_alloc(NL_CB_DEFAULT);
	if(!cb){
		fprintf(stderr, "Failed to allocate nl callback\n");
		nl_socket_free(local_nl.sk);
		fclose(fp);
		return -ENOMEM;
	}
	
//	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, macCallback_handler,NULL);//why ??

	
	//Form a message
	struct nl_msg *acl_msg = nlmsg_alloc();
	
	if(!acl_msg)
		return -ENOMEM;
	
	/*Add number of mac-addresess to message
	for(i = 0; i < mac_info.num_of_mac_for_acl; i++) {
		if(nla_put(acl_msg, i + 1, ETH_ALEN, mac_info.mac_acl[i].addr))
			nlmsg_free(acl_msg);
		printf("NLA_PUT failed\n");
	}*/
	if(nla_put(acl_msg, i + 1, ETH_ALEN, mac_addr) != 0){
		printf("NLA_PUT Failed\n");
		nlmsg_free(acl_msg);
		}
		
		

	//Form the final netlink message
	//Setup the message
	struct nl_msg *drv_msg = nlmsg_alloc();
	
	enum nl80211_commands cmd = NL80211_CMD_SET_MAC_ACL;
	
	if(!(genlmsg_put(drv_msg, NL_AUTO_PORT, NL_AUTO_SEQ, local_nl.nl80211_driverId, sizeof(acl_msg), 0, cmd, 0))){
//	if(!(genlmsg_put(drv_msg, 0, 0, local_nl.nl80211_driverId, 0, 0, NL80211_CMD_SET_MAC_ACL , 0))){
		printf("genlmsg_put Failed\n");
		nlmsg_free(acl_msg);
		}
	enum nl80211_acl_policy mac_policy = NL80211_ACL_POLICY_DENY_UNLESS_LISTED;
	if((nla_put_u32(drv_msg, NL80211_ATTR_ACL_POLICY, mac_policy )) != 0){
		printf("NLA_PUT Failed \n");
		nlmsg_free(acl_msg);
		}

/*	enum nl80211_iftype iftype = NL80211_IFTYPE_AP;
	if((nla_put_u32(drv_msg, NL80211_ATTR_IFTYPE, iftype)) != 0){
		printf("NLA_put for iftype failed\n");
		nlmsg_free(acl_msg);
		}*/
 
/*	if(!(nla_put_nested(drv_msg, NL80211_ATTR_MAC_ADDRS,acl_msg))) {
		nlmsg_free(drv_msg);
		fprintf(stderr, "Failed to nest message\n");
		nlmsg_free(acl_msg);
		return -ENOMEM;
	}*/
	
	nlmsg_free(acl_msg);

	
	//Try to send the message to nl80211 and wait for it to finish and in return invoke our callback
	ret = nl_send_auto(local_nl.sk,drv_msg);
	//ret = nl_send_auto(local_nl.sk,drv_msg);
	
	if(ret < 0)
		fprintf(stderr,"Failed to send the netlink message for setting MAC ACL's\n");
	else {
		printf("Sent the message for MAC ACL to nl80211\n");
		//Before sending the message, install finishing handler
		nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, macCallback_finish_handler, &err);
		//Block for message to return and invoke our handler
		while(err > 0) {
			nl_recvmsgs(local_nl.sk, cb);
		}
	}

	return ret;
	
}
