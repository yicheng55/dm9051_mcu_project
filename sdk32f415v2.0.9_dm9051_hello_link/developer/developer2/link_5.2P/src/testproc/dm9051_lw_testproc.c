#if 0

#include "dm9051_lw.h" //#include "dm9051opts.h"
#include "dm9051_lw_conf.h" //#include "dm9051_lw_decl.h"
#include "dm9051_lw_testproc_type.h"
#include "dm9051_lw_testproc.h"
//#include "dm9051_lw_sync.h"
	
#if 1 
/* [arch ctype "arch.h" solution. ] */
#include <ctype.h>
#define lwip_isdigit(c)           isdigit((unsigned char)(c))
#define lwip_isxdigit(c)          isxdigit((unsigned char)(c))
#define lwip_islower(c)           islower((unsigned char)(c))
#define lwip_isspace(c)           isspace((unsigned char)(c))
#endif

static const struct uip_eth_addr uip_ethbroadcast = {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};
static const struct uip_eth_addr uip_ethzero = {{0, 0, 0, 0, 0, 0}};
	
/** ip address */
#define ip4_addr_set_u32(dest_ipaddr, src_u32) ((dest_ipaddr)->addr = (src_u32))
#define ip4_addr_get_u32(src_ipaddr) ((src_ipaddr)->addr)

#define SMEMCPY(dst,src,len)            memcpy(dst,src,len)
#define IPADDR_WORDALIGNED_COPY_FROM_IP4_ADDR_T(dest, src) SMEMCPY(dest, src, sizeof(ip4_addr_t))

//macro definition
//#ifndef PP_HTONL
//#endif
#define TPP_HTONL(x) ((((x) & (uint32_t)0x000000ffUL) << 24) | \
                     (((x) & (uint32_t)0x0000ff00UL) <<  8) | \
                     (((x) & (uint32_t)0x00ff0000UL) >>  8) | \
                     (((x) & (uint32_t)0xff000000UL) >> 24))

//macro definition
//#ifndef PP_HTONS
//#endif
#define TPP_HTONS(x) ((uint16_t)((((x) & (uint16_t)0x00ffU) << 8) | (((x) & (uint16_t)0xff00U) >> 8)))

#define LWIP_ASSERT(message, assertion)
					 
uint32_t
lwip_htonl_1(uint32_t n) {
  return TPP_HTONL(n);
}
uint16_t
lwip_htons_1(uint16_t n)
{
  return TPP_HTONS(n);
}
	
int ip4addr_aton1(const char *cp, ip4_addr_t *addr)
{
  uint32_t val;
  uint8_t base;
  char c;
  uint32_t parts[4];
  uint32_t *pp = parts;

  c = *cp;
  for (;;) {
    /*
     * Collect number up to ``.''.
     * Values are specified as for C:
     * 0x=hex, 0=octal, 1-9=decimal.
     */
    if (!lwip_isdigit(c)) {
      return 0;
    }
    val = 0;
    base = 10;
    if (c == '0') {
      c = *++cp;
      if (c == 'x' || c == 'X') {
        base = 16;
        c = *++cp;
      } else {
        base = 8;
      }
    }
    for (;;) {
      if (lwip_isdigit(c)) {
        val = (val * base) + (uint32_t)(c - '0');
        c = *++cp;
      } else if (base == 16 && lwip_isxdigit(c)) {
        val = (val << 4) | (uint32_t)(c + 10 - (lwip_islower(c) ? 'a' : 'A'));
        c = *++cp;
      } else {
        break;
      }
    }
    if (c == '.') {
      /*
       * Internet format:
       *  a.b.c.d
       *  a.b.c   (with c treated as 16 bits)
       *  a.b (with b treated as 24 bits)
       */
      if (pp >= parts + 3) {
        return 0;
      }
      *pp++ = val;
      c = *++cp;
    } else {
      break;
    }
  }
  /*
   * Check for trailing characters.
   */
  if (c != '\0' && !lwip_isspace(c)) {
    return 0;
  }
  /*
   * Concoct the address according to
   * the number of parts specified.
   */
  switch (pp - parts + 1) {

    case 0:
      return 0;       /* initial nondigit */

    case 1:             /* a -- 32 bits */
      break;

    case 2:             /* a.b -- 8.24 bits */
      if (val > 0xffffffUL) {
        return 0;
      }
      if (parts[0] > 0xff) {
        return 0;
      }
      val |= parts[0] << 24;
      break;

    case 3:             /* a.b.c -- 8.8.16 bits */
      if (val > 0xffff) {
        return 0;
      }
      if ((parts[0] > 0xff) || (parts[1] > 0xff)) {
        return 0;
      }
      val |= (parts[0] << 24) | (parts[1] << 16);
      break;

    case 4:             /* a.b.c.d -- 8.8.8.8 bits */
      if (val > 0xff) {
        return 0;
      }
      if ((parts[0] > 0xff) || (parts[1] > 0xff) || (parts[2] > 0xff)) {
        return 0;
      }
      val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
      break;
    default:
      LWIP_ASSERT("unhandled", 0);
      break;
  }
  if (addr) {
    ip4_addr_set_u32(addr, lwip_htonl_1(val));
  }
  return 1;
}

/**
 * Ascii internet address interpretation routine.
 * The value returned is in network order.
 *
 * @param cp IP address in ascii representation (e.g. "127.0.0.1")
 * @return ip address in network order
 */
uint32_t
ipaddr_addr_1(const char *cp)
{
  ip4_addr_t val;

  if (ip4addr_aton1(cp, &val)) {
    return ip4_addr_get_u32(&val);
  }
  return (IPADDR_NONE);
}

/* ARP message types (opcodes) */
enum etharp_opcode {
  ARP_REQUEST = 1,
  ARP_REPLY   = 2
};
enum lwip_iana_hwtype {
  /** Ethernet */
  LWIP_IANA_HWTYPE_ETHERNET = 1
};
enum lwip_ieee_eth_type {
  /** Internet protocol v4 */
  ETHTYPE_IP        = 0x0800U,
  /** Address resolution protocol */
  ETHTYPE_ARP       = 0x0806U,
  /** Wake on lan */
  ETHTYPE_WOL       = 0x0842U,
  /** RARP */
  ETHTYPE_RARP      = 0x8035U,
  /** Virtual local area network */
  ETHTYPE_VLAN      = 0x8100U,
  /** Internet protocol v6 */
  ETHTYPE_IPV6      = 0x86DDU,
  /** PPP Over Ethernet Discovery Stage */
  ETHTYPE_PPPOEDISC = 0x8863U,
  /** PPP Over Ethernet Session Stage */
  ETHTYPE_PPPOE     = 0x8864U,
  /** Jumbo Frames */
  ETHTYPE_JUMBO     = 0x8870U,
  /** Process field network */
  ETHTYPE_PROFINET  = 0x8892U,
  /** Ethernet for control automation technology */
  ETHTYPE_ETHERCAT  = 0x88A4U,
  /** Link layer discovery protocol */
  ETHTYPE_LLDP      = 0x88CCU,
  /** Serial real-time communication system */
  ETHTYPE_SERCOS    = 0x88CDU,
  /** Media redundancy protocol */
  ETHTYPE_MRP       = 0x88E3U,
  /** Precision time protocol */
  ETHTYPE_PTP       = 0x88F7U,
  /** Q-in-Q, 802.1ad */
  ETHTYPE_QINQ      = 0x9100U
};

union {
	uint8_t rx;
	uint8_t tx;
} TestProcBuff[100];
uint16_t TestProcBuff_Len;

/**
 * Send a raw ARP packet (opcode and all addresses can be modified)
 *
 * @param netif the lwip network interface on which to send the ARP packet
 * @param ethsrc_addr the source MAC address for the ethernet header
 * @param ethdst_addr the destination MAC address for the ethernet header
 * @param hwsrc_addr the source MAC address for the ARP protocol header
 * @param ipsrc_addr the source IP address for the ARP protocol header
 * @param hwdst_addr the destination MAC address for the ARP protocol header
 * @param ipdst_addr the destination IP address for the ARP protocol header
 * @param opcode the type of the ARP packet
 * @return ERR_OK if the ARP packet has been sent
 *         ERR_MEM if the ARP packet couldn't be allocated
 *         any other err_t on failure
 */
/*static*/ signed char
uip_etharp_raw(/*struct netif *netif*/void *arg, const struct eth_addr *ethsrc_addr,
           const struct uip_eth_addr *ethdst_addr,
           const struct eth_addr *hwsrc_addr, const ip4_addr_t *ipsrc_addr,
           const struct uip_eth_addr *hwdst_addr, const ip4_addr_t *ipdst_addr,
           const uint16_t opcode)
{
  signed char result = 0; //=ERR_OK;
  struct etharp_hdr *hdr;

  /* SIZEOF_ETHARP_HDR is 28 */
  /*p = pbuf_alloc(PBUF_LINK, SIZEOF_ETHARP_HDR, PBUF_RAM);
  if (p == NULL) {
    ETHARP_STATS_INC(etharp.memerr);
    return ERR_MEM;
  }
  hdr = (struct etharp_hdr *)p->payload;*/

  hdr = (struct etharp_hdr *)(&TestProcBuff[0].tx + SIZEOF_ETH_HDR);
	  
	  hdr->opcode = lwip_htons_1(opcode);

	  /* Write the ARP MAC-Addresses */
	  SMEMCPY(&hdr->shwaddr, hwsrc_addr, ETH_HWADDR_LEN);
	  SMEMCPY(&hdr->dhwaddr, hwdst_addr, ETH_HWADDR_LEN);
	  /* Copy struct ip4_addr_wordaligned to aligned ip4_addr, to support compilers without
	   * structure packing. */
	  IPADDR_WORDALIGNED_COPY_FROM_IP4_ADDR_T(&hdr->sipaddr, ipsrc_addr);
	  IPADDR_WORDALIGNED_COPY_FROM_IP4_ADDR_T(&hdr->dipaddr, ipdst_addr);

	  hdr->hwtype = TPP_HTONS(LWIP_IANA_HWTYPE_ETHERNET);
	  hdr->proto = TPP_HTONS(ETHTYPE_IP);
	  /* set hwlen and protolen */
	  hdr->hwlen = ETH_HWADDR_LEN;
	  hdr->protolen = sizeof(ip4_addr_t);
  
  TestProcBuff_Len = sizeof(struct etharp_hdr); //init. content-data-len

  /*pbuf_free(p);
  p = NULL;*/
  return result;
}

//.static signed char dm_etharp_request_dst(void *arg, const ip4_addr_t *ipaddr, const struct eth_addr *hw_dst_addr)
//.{
//.  return _etharp_raw(arg, (struct eth_addr *)MACaddr, hw_dst_addr,
//.                    (struct eth_addr *)MACaddr, NULL, &ethzero,
//.                    ipaddr, ARP_REQUEST);
//.}

#if (ETHERNET_COUNT >= 2)
static void
bench_receive_period(void) {
	int i;
	uint16_t len;
	uint8_t zbuff[72];
	char disp[72];

	for (i = 0; i < ETHERNET_COUNT; i++) { //get_eth_interfaces()
		mstep_set_net_index(i); //set_pin_code(i);
		len = dm9051_rx(zbuff);
		if (len) {
			memset(disp, 0, sizeof(disp));
			memcpy(disp, zbuff, len - 4); //sizeof(zbuff)
			printf("%s, recv \"%s\" len %u\r\n", mstep_spi_conf_name(), disp, len - 4); //sizeof(zbuff)
			dm9051_rxlog_monitor_rx_all(2, zbuff, len - 4); //function_monitor_rx_all(zbuff, len - 4);
			//break;
		}
	}
	mstep_set_net_index(0);
}

void bench_test_tx_and_rx(void)
{
	int i;
	uint8_t rcr[ETHERNET_COUNT], rcrn[ETHERNET_COUNT];
	
	for (i = 0; i < ETHERNET_COUNT; i++) {
		mstep_set_net_index(i);
		rcr[i] = cspi_read_reg(DM9051_RCR);
		cspi_write_reg(DM9051_RCR, 0x33); //for the any raw data
		rcrn[i] = cspi_read_reg(DM9051_RCR);
		printf("%d. bench_test.s rcr %02x to %02x\r\n", i, rcr[i], rcrn[i]);
	}
	
	printf("\r\n");
	
	mstep_set_net_index(0);
	dm9051_tx("abcdefghijkl........abcdefghijkl........abcdefghijkl.....xxx", 60);
	printf("%s, send \"%s\" len %d\r\n", mstep_spi_conf_name(), 
		"abcdefghijkl........abcdefghijkl........abcdefghijkl.....xxx",
		strlen("abcdefghijkl........abcdefghijkl........abcdefghijkl.....xxx"));
	dm9051_txlog_monitor_tx_all(2, "abcdefghijkl........abcdefghijkl........abcdefghijkl.....xxx",
		strlen("abcdefghijkl........abcdefghijkl........abcdefghijkl.....xxx"));

	bench_receive_period();
	printf("\r\n");
	
	mstep_set_net_index(1);
	dm9051_tx("play-game", 9);
	printf("%s, send \"%s\" len %d\r\n", mstep_spi_conf_name(), 
		"play-game", 
		strlen("play-game"));
	function_monitor_tx(2, "play-game",
		strlen("play-game"));

	bench_receive_period();
	printf("\r\n");
	
	for (i = 0; i < ETHERNET_COUNT; i++) {
		mstep_set_net_index(i);
		spi_write_reg(DM9051_RCR, rcr[i]);
		rcr[i] = spi_read_reg(DM9051_RCR);
		printf("%d. bench_test.e rcr %02x to %02x\r\n", i, rcrn[i], rcr[i]);
	}
}

//static void test_ethernetif_txrx(void)
//{
//  bench_test_tx_and_rx();
//}
#endif

/*static uint8_t
pbuf_add_header_impl(void *nullp, size_t header_size_increment, uint8_t force)
{
	return 0;
}

uint8_t
pbuf_add_header(void *p, size_t header_size_increment)
{
  return pbuf_add_header_impl(p, header_size_increment, 0);
}*/

/*static*/ void
uip_ethernet_output(void *nullnetif, void *nullp,
                const struct eth_addr * src, const struct uip_eth_addr * dst,
                uint16_t eth_type) {
#if 1
	struct eth_hdr *ethhdr;
	
	//.pbuf_add_header(NULL, SIZEOF_ETH_HDR);
					
	ethhdr = (struct eth_hdr *)&TestProcBuff[0].tx; //p->payload;
	ethhdr->type = lwip_htons_1(eth_type);
	SMEMCPY(&ethhdr->dest, dst, ETH_HWADDR_LEN);
	SMEMCPY(&ethhdr->src,  src, ETH_HWADDR_LEN);
					
	TestProcBuff_Len += sizeof(struct eth_hdr);
#endif
	dm9051_txlog_monitor_tx_all(2, &TestProcBuff[0].tx, TestProcBuff_Len);
	dm9051_tx(&TestProcBuff[0].tx, TestProcBuff_Len);
}

#if (ETHERNET_COUNT == 1) 
static void self_test_partner_ARPa(void)
{
	//dm9051_tx("...", 60);
	//=
	ip_addr_t ipaddr;
	
	//LIKE 'ip4_addr_copy' (ipaddr, DECLARATION_PARTNER_IPADDR)
	ip4_addr_set_u32(&ipaddr, ipaddr_addr_1(DECLARATION_PARTNER_IPADDR));
	
	//=dm_etharp_request_dst(NULL, &ipaddr, &ethbroadcast);
	uip_etharp_raw(NULL, (struct eth_addr *)MACaddr, &uip_ethbroadcast,
                    (struct eth_addr *)MACaddr, /*netif_ip4_addr(netif)*/NULL, &uip_ethzero,
                    &ipaddr, ARP_REQUEST);

	/* send ARP query */
	uip_ethernet_output(NULL, NULL, (struct eth_addr *)MACaddr, &uip_ethbroadcast, ETHTYPE_ARP);
}

static void self_test_partner_ARPb(void)
{
	//dm9051_tx("play-game", 9);
	//=
	ip_addr_t ipaddr;
	ip4_addr_set_u32(&ipaddr, ipaddr_addr_1(DECLARATION_PARTNER_IPADDR));
	
	uip_etharp_raw(NULL, (struct eth_addr *)MACaddr, &uip_ethbroadcast,
                    (struct eth_addr *)MACaddr, /*netif_ip4_addr(netif)*/NULL, &uip_ethzero,
                    &ipaddr, ARP_REQUEST);
	
	uip_ethernet_output(NULL, NULL, (struct eth_addr *)MACaddr, &uip_ethbroadcast, ETHTYPE_ARP); // dm9051 tx
}
				
static uint16_t
self_receive_period(void) {
	int i;
	//uint16_t len;
	//uint8_t *zbuff = &TestProcBuff[0].rx;
	
	for (i = 0; i< 100; i++) {
		dm_delay_ms(10);
		
		//len = dm9051_rx(zbuff);
		//if (len) {
		//	printf("  %s, recv, len %u\r\n", mstep_spi_conf_name(), len - 4); //sizeof(zbuff)
		//	dm9051_rxlog_monitor_rx_all(2, zbuff, len - 4); //function_monitor_rx_all(zbuff, len - 4);
		//	return len;
		//}
	}
	return 0;
}
#endif

int test_total_send_count_setup; //.

//#ifdef AT32F437xx
int test_line7_enter = 0;
uint8_t my_debounce = 0;

uint16_t test_rx_hdlr(void)
{
	uint8_t *zbuff = &TestProcBuff[0].rx;
	uint16_t len;

printf("[rx_start.s]\r\n");
	len = dm9051_rx(zbuff);
printf("[rx_start.e] len = %d\r\n", len);
	
	if (len) {
		printf("  %s, recv, len %u\r\n", mstep_spi_conf_name(), len - 4); //sizeof(zbuff)
		dm9051_rxlog_monitor_rx_all(2, zbuff, len - 4); //dm9051_rxlog_monitor_tx_all(zbuff, len - 4);
	}
	return len;
}

int CalcModelShow(int inParam)
{
	int outParam = inParam - 1;
	printf("................................[Yes-Good]: line7_proc() CalcModel-Show tested-packets %d recved-remain %d\r\n", inParam, outParam);
	return outParam;
}

void line7_proc(void) {
  uint16_t len;
	
  uint8_t isr;
  isr = cspi_read_reg(DM9051_ISR);
  
  printf("[INFO]: line7() enter %d ... isr %02x\r\n", ++test_line7_enter, isr);
  printf("................................ line7_proc(), where test_line7_enter_check_setup is %d\r\n", test_line7_enter_check_setup);
	
  if (test_line7_enter_check_setup == 0)
	  printf("................................ [WARN]: line7_proc() encounterred illegal enter! '_test_line7_enter_check_setup' still on 0\r\n");

  if (my_debounce == 0) {
	my_debounce = 8;
	at32_led_toggle(LED4);
	//at32_led_toggle(LED2);
	//at32_led_toggle(LED3);
	//at32_led_on(LED4);
  }
  
  #if 1
  //lwip_rx_loop_handler();
  //lwip_rx_hdlr();
  do {
	len = test_rx_hdlr();
	if (len) {
		if (test_total_send_count_setup) {
			
			test_total_send_count_setup = CalcModelShow(test_total_send_count_setup);
			
		} else 
			printf("[WARN]: line7() encounterred under_flow count!\r\n");
	}
  } while(len);
  #endif
  
  my_debounce += 8; // if this only, when 256, i.e. 0
  my_debounce &= 0x3f; // when 64, i.e. 0
  
  isr = cspi_read_reg(DM9051_ISR);
  printf("[INFO]: line7() exit %d ... isr %02x\r\n", ++test_line7_enter, isr);
  printf("[INFO]: line7() exit write isr %02x\r\n", isr);
  cspi_write_reg(DM9051_ISR, isr);
}
//#endif

void display_recv_point(char *hstr)
{
  uint8_t rxpl, rxph;
  rxpl = cspi_read_reg(DM9051_RWPAL);
  rxph = cspi_read_reg(DM9051_RWPAH);
  printf("[%s] _recv_wr_point %02x%02x\r\n", hstr, rxph, rxpl);
}

#if (ETHERNET_COUNT == 1) 
int tx_and_rx_packets(int t) {
	uint16_t len;
display_recv_point("Start");	
	test_total_send_count_setup = t; //...
	printf("- TEST a trip for %d packets ...\r\n", t);

	
	printf("  %s, send ARP 1\r\n", mstep_spi_conf_name());
	self_test_partner_ARPa();
	
	len = self_receive_period();
	if (len) t--;
display_recv_point("Send1");
	
	//printf("\r\n");
	
	printf("  %s, send ARP 2\r\n", mstep_spi_conf_name());
	self_test_partner_ARPb();
	
	len = self_receive_period();
	if (len) t--;
display_recv_point("Send2");
	
	return t;
}

void self_test_tx_and_rx(void) //So can CONNECT PHY port RJ-45 a "loopback core".
{	
	uint16_t len;
	/*int tcount;*/ //= 2;

printf("[RUN1]\r\n");	
	/*tcount =*/ tx_and_rx_packets(2);
	
	if (test_total_send_count_setup) {
		
		int tloop = 8;
		uint8_t rcr, rcrn;
		
		printf("  :No satisfy receive every ARP echo! Read something in the rx fifo. Use promiscuous! [CHECK-POINT]\r\n");
		printf("  :! [CHECK-POINT]\r\n");
		printf("\r\n");
		
		rcr = cspi_read_reg(DM9051_RCR);
		cspi_write_reg(DM9051_RCR, 0x33);
		rcrn = cspi_read_reg(DM9051_RCR);
		printf(": [.m bench_test rcr %02x to %02x]\r\n", rcr, rcrn);
		
printf("[RUN2]\r\n");	
		/*tcount =*/ tx_and_rx_packets(2);
		
		while (test_total_send_count_setup && tloop--) {
			
			len = self_receive_period();
			if (len) /*tcount--*/;
			
		}
	}
	
display_recv_point("Stop");
	printf("  :Read finish: %s\r\n", test_total_send_count_setup ? "still lose some packets!" : "corresponse packets rececived!");
	printf("\r\n");
}
#endif // (ETHERNET_COUNT==1)
#endif
