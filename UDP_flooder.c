#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <arpa/inet.h>
#define PCKT_LEN 8192
//function to calculate control sum
unsigned short csum(unsigned short *buf, int nwords)
{
	unsigned long sum;
	for (sum = 0; nwords > 0; nwords--)
		sum += *buf++;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return (unsigned short)(~sum);
}

int main(int argc, char const *argv[])
{
	if (argc != 5)
	{
		printf("Invalid arguments passed!\n");
		printf("Usage: %s [source ipheader] [source port] [target ipheader] [target port]\n", argv[0]); //if user gave incorrect arguments
		exit(1);
	}

	uint16_t src_port, dst_port; 
	uint32_t src_addr, dst_addr; //binary ipheader adress
	src_addr = inet_addr(argv[1]);
	dst_addr = inet_addr(argv[3]);
	src_port = atoi(argv[2]);
	dst_port = atoi(argv[4]);

	int mysock = 0;																 
	char buffer[PCKT_LEN], data[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";				 
	struct iphdr *ipheader = (struct iphdr *)buffer;							 
	struct udphdr *udpheader = (struct udphdr *)(buffer + sizeof(struct iphdr));                                   ^       отправляемый пакет      ^

	struct sockaddr_in sin; 
	int one = 1;
	const int *val = &one; 

	memset(buffer, 0, PCKT_LEN); 

	// Create RAW-socket with UDP
	mysock = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);
	if (mysock < 0)
	{ 
		perror("socket() error");
		exit(2);
	}
	printf("RAW socket created\n");

	if (setsockopt(mysock, IPPROTO_IP, IP_HDRINCL, val, sizeof(int)) < 0)
	{ 
		perror("setsockopt() error");
		exit(2);
	}
	sin.sin_family = AF_INET;		
	sin.sin_port = htons(dst_port); 
	sin.sin_addr.s_addr = dst_addr;
	ipheader->ihl = 5;				
	ipheader->version = 4;			
	ipheader->tos = 0;				
	ipheader->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data); 
	ipheader->id = 0;
	ipheader->ttl = 64;			  
	ipheader->protocol = IPPROTO_UDP; 
	ipheader->saddr = src_addr;
	ipheader->daddr = dst_addr;
	udpheader->source = htons(src_port);
	udpheader->dest = htons(dst_port);
	udpheader->len = htons(sizeof(struct udphdr));

	// calculate control sum
	ipheader->check = csum((unsigned short *)buffer,
						   sizeof(struct iphdr) + sizeof(struct udphdr));
	memcpy(buffer + sizeof(struct udphdr) + sizeof(struct iphdr), data, strlen(data)); //copy data to packet
	while (1)
	{
		if (sendto(mysock, buffer, ipheader->tot_len, 0, 
				   (struct sockaddr *)&sin, sizeof(sin)) < 0)
		{
			perror("sendto()");
			exit(3);
		}
	}
	return 0;
}