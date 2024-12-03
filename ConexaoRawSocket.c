#include "ConexaoRawSocket.h"


int ConexaoRawSocket(char *device)
{
  int soquete;
  struct ifreq ir;
  struct sockaddr_ll endereco;
  struct packet_mreq mr;

  soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));  	/*cria socket*/
  if (soquete == -1) {
    printf("Erro no Socket\n");
    exit(-1);
  }

  memset(&ir, 0, sizeof(struct ifreq));  	/*dispositivo eth0*/
  memcpy(ir.ifr_name, device, strlen(device));
  if (ioctl(soquete, SIOCGIFINDEX, &ir) == -1) {
    printf("Erro no ioctl\n");
    exit(-1);
  }
	

  memset(&endereco, 0, sizeof(endereco)); 	/*IP do dispositivo*/
  endereco.sll_family = AF_PACKET;
  endereco.sll_protocol = htons(ETH_P_ALL);
  endereco.sll_ifindex = ir.ifr_ifindex;
  if (bind(soquete, (struct sockaddr *)&endereco, sizeof(endereco)) == -1) {
    printf("Erro no bind\n");
    exit(-1);
  }


  memset(&mr, 0, sizeof(mr));          /*Modo Promiscuo*/
  mr.mr_ifindex = ir.ifr_ifindex;
  mr.mr_type = PACKET_MR_PROMISC;
  if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)	{
    printf("Erro ao fazer setsockopt\n");
    exit(-1);
  }

  return soquete;
}

int conexaoDebug(char *device, char *ip_destino)
{
  int soquete;
  struct ifreq ir;
  struct sockaddr_ll endereco;
  struct packet_mreq mr;
  struct sockaddr_in dest_addr;
  
  // Cria o socket raw
  soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)); 
  if (soquete == -1) {
    printf("Erro no Socket\n");
    exit(-1);
  }
  if(device == NULL) {
	  printf("Dispositivo não fornecido.\n");
	  exit(-1);
  }
  // Define o dispositivo (por exemplo, eth0)
  memset(&ir, 0, sizeof(struct ifreq));   
  memcpy(ir.ifr_name, device, strlen(device));
  if (ioctl(soquete, SIOCGIFINDEX, &ir) == -1) {
    printf("Erro no ioctl\n");
    exit(-1);
  }

  // Configura a estrutura de sockaddr_ll
  memset(&endereco, 0, sizeof(endereco));  
  endereco.sll_family = AF_PACKET;
  endereco.sll_protocol = htons(ETH_P_ALL);
  endereco.sll_ifindex = ir.ifr_ifindex;

  // Associa o socket à interface de rede
  if (bind(soquete, (struct sockaddr *)&endereco, sizeof(endereco)) == -1) {
    printf("Erro no bind\n");
    exit(-1);
  }

  // Ativa o modo promiscuo
  memset(&mr, 0, sizeof(mr));          
  mr.mr_ifindex = ir.ifr_ifindex;
  mr.mr_type = PACKET_MR_PROMISC;
  if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {
    printf("Erro ao fazer setsockopt\n");
    exit(-1);
  }

  // Configura o endereço IP de destino local
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(12345);  // Porta de destino (exemplo)
  dest_addr.sin_addr.s_addr = inet_addr(ip_destino);  // IP de destino

  // Agora você pode enviar pacotes para o IP local
  // Por exemplo, para enviar um pacote de teste:
  // sendto(soquete, buffer, tamanho, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

  return soquete;
}
