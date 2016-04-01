/* This code filters mac-id at driver level */
#include <linux/netlink.h>
#include <libnl3/netlink/genl/genl.h>
#include <libnl3/netlink/genl/ctrl.h>
#include <libnl3/netlink/genl/family.h>
#include <linux/nl80211.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define ETH_ALEN 6

int main(int argc, char *argv[])
{
	int result = 0;

	char *mac_addr = "ac:9e:17:26:bd:23"; //this will also be a pointer to data as netlink attribute payload

	struct nl_sock *nl_sk;
	struct nl_msg *msg_nl;

	//Allocate new netlink socket
	nl_sk = nl_socket_alloc();
	if(!nl_sk){
		printf("Cannot allocate netlink socket\n");
		return -ENOMEM;
	}
	
	//Allocate a new netlink message
	msg_nl = nlmsg_alloc();
	if(!msg_nl){
		printf("Cannot allocate netlink message\n");
		nl_socket_free(nl_sk);
		return -ENOMEM;
	}

	//Make a custom nlmsg with our payload
	result = nla_put(msg_nl, 1, ETH_ALEN,(const void*) mac_addr);
	if(result){
		printf("failed to nla_put our payload to nla msg\n");
		nl_socket_free(nl_sk);
		nlmsg_free(msg_nl);
		return -ENOMEM;		
	}	
	else
		printf("nla_put is success\n");

	
	//Connect to the netlink socket
	if(genl_connect(nl_sk)){
		printf("Failed to connect to netlink socket\n");
		return -ENOMEM;
	}
	else
		printf("Connect to netlink socket\n");	


	//Send the nl message
	result = nl_send(nl_sk, msg_nl);
	if(result)
		printf("%d number of bytes sent to netlink layer\n");
	else
		printf("Failed to send nla_msg to netlink layer");
		


	nlmsg_free(msg_nl);	
	nl_socket_free(nl_sk);
	return result;

}
