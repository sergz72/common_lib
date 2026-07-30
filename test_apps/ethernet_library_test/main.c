#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <string.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/ether.h>
#include <linux/if_packet.h>
#include <eth.h>
#include <eth_queue.h>
#include <stdlib.h>
#include <eth_ntp.h>

#define IPV6_PROTOCOL 0x86DD
#define BUF_SIZ		    2048

static unsigned char buf[BUF_SIZ];
static int sockfd_sender;
static struct ifreq if_idx;

int ethernet_packet_send(const ETH_Header *buff, unsigned int length)
{
  struct sockaddr_ll socket_address;

  /* Index of the network device */
  socket_address.sll_ifindex = if_idx.ifr_ifindex;
  /* Address length*/
  socket_address.sll_halen = ETH_ALEN;
  /* Destination MAC */
  socket_address.sll_addr[0] = buff->dest_addr[0];
  socket_address.sll_addr[1] = buff->dest_addr[1];
  socket_address.sll_addr[2] = buff->dest_addr[2];
  socket_address.sll_addr[3] = buff->dest_addr[3];
  socket_address.sll_addr[4] = buff->dest_addr[4];
  socket_address.sll_addr[5] = buff->dest_addr[5];

  return sendto(sockfd_sender, buff, length, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) == -1 ? -1 : 0;
}

void eth_set_prefix_callback(void)
{
  if (!ntp_time_is_set)
    ETH_NTP_Send_Timestamp_Request();
}

int main(int argc, char **argv)
{
  if (argc != 3)
  {
    printf("Usage: ethernet_library_test interface_name mac_address");
    return 1;
  }
  int sockfd_listener;
  struct ifreq if_mac;
  struct ifreq ifopts;	/* set promiscuous mode */
  int sockopt;

  /* Open RAW socket to send on */
  if ((sockfd_sender = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1)
  {
    perror("sender: socket");
    return 2;
  }
  /* Get the index of the interface to send on */
  memset(&if_idx, 0, sizeof(struct ifreq));
  strncpy(if_idx.ifr_name, argv[1], IFNAMSIZ-1);
  if (ioctl(sockfd_sender, SIOCGIFINDEX, &if_idx) < 0)
  {
    close(sockfd_sender);
    perror("sender: SIOCGIFINDEX");
    return 3;
  }
  /* Get the MAC address of the interface to send on */
  memset(&if_mac, 0, sizeof(struct ifreq));
  strncpy(if_mac.ifr_name, argv[1], IFNAMSIZ-1);
  if (ioctl(sockfd_sender, SIOCGIFHWADDR, &if_mac) < 0)
  {
    close(sockfd_sender);
    perror("sender: SIOCGIFHWADDR");
    return 4;
  }

  /* Open PF_PACKET socket, listening for EtherType ETHER_TYPE */
  if ((sockfd_listener = socket(PF_PACKET, SOCK_RAW, htons(IPV6_PROTOCOL))) == -1)
  {
    close(sockfd_sender);
    perror("listener: socket");
    return 5;
  }

  /* Set interface to promiscuous mode - do we need to do this every time? */
  strncpy(ifopts.ifr_name, argv[1], IFNAMSIZ-1);
  ioctl(sockfd_listener, SIOCGIFFLAGS, &ifopts);
  ifopts.ifr_flags |= IFF_PROMISC;
  ioctl(sockfd_listener, SIOCSIFFLAGS, &ifopts);
  /* Allow the socket to be reused - incase connection is closed prematurely */
  sockopt = 1;
  if (setsockopt(sockfd_listener, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1)
  {
    perror("setsockopt");
    close(sockfd_sender);
    close(sockfd_listener);
    return 6;
  }
  /* Bind to device */
  if (setsockopt(sockfd_listener, SOL_SOCKET, SO_BINDTODEVICE, argv[1], IFNAMSIZ-1) == -1)
  {
    perror("SO_BINDTODEVICE");
    close(sockfd_sender);
    close(sockfd_listener);
    return 7;
  }

  unsigned long long int m = strtoull(argv[2], nullptr, 16);
  printf("My MAC: 0x%llx\n", m);
  ETH_Init((const unsigned char*)&m, printf, true);

  const struct ether_header *eh = (const struct ether_header *)buf;

  while (1)
  {
    ssize_t numbytes = recvfrom(sockfd_listener, buf, BUF_SIZ, 0, nullptr, nullptr);
    if (numbytes == -1)
    {
      perror("recvfrom");
      continue;
    }
    printf("listener: got packet %ld bytes\n", numbytes);
    ethernet_packet_received(buf, numbytes);
    ETH_Handler();
  }

  close(sockfd_sender);
  close(sockfd_listener);

  return 0;
}
