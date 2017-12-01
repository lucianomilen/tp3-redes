//COMMON.C - LUCIANO OTONI MILEN [2012079754]

#include "common.h"

int getPackID(char *pack){ //retorna o id do pacote que recebeu

								int pack_id;
								memcpy(&pack_id, &pack[0], 4); //o pacote deve ser atribuido desta forma, para pegar a posição correta do id do pacote que tá vindo
								return pack_id;
}

char checksum(char *buffer, int buffSize){ // faz o checksum. função que achei pronta

								int i;
								int sum = 0;
								for(i = 0; i < buffSize; i++) {
																sum += buffer[i];
																sum = sum % 128;
								}
								return sum;
}

void timer(unsigned int sec){ // foi desta forma que consegui marcar o temporizador,

								struct timeval tv;
								tv.tv_sec=sec;
								tv.tv_usec=0;
								//sinalizando o socket com o tempo de timeout
								setsockopt(sockID,SOL_SOCKET,SO_RCVTIMEO,(struct timeval*)&tv, sizeof(struct timeval));
}

void createPack(char *pack, int packID, int packType, char *buffer, int buffSize){

								memcpy(pack, &packID, 4);         //4 bytes pro id do pacote
								pack[4] = packType;         //1 Byte pro tipo do pacote
								pack[5] = checksum(buffer,buffSize);         // checksum é atribuido aqui
								strncpy(HEADER+pack,buffer,buffSize);         // os dados são colocados na ultima posição do pacote
}
