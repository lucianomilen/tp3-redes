//COMMON.H - ANDRÉ HENRIQUES RODRIGUES [2016057941] & LUCIANO OTONI MILEN [2012079754]


#ifndef COMMON_H
#define COMMON_H

#define HEADER 6
#define DATA_TYPE 0
#define ACK_TYPE 1
#define FINAL_TYPE 2

#include "tp_socket.h"
#include <sys/time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct sockaddr_in addr; //endereço do socket
int sentPacks; // o nome já diz pra que serve, estatisticas...
int recPacks;
int dispPacks;
int resentPacks;
int recACKs;
int sentACKs;
int sockID;
int packID;
int lastACKID;
char *recBuff;

char** buff_window;
int window_size;
int curr_window_size;
int win_slide;

int getPackID(char *pack);

char checksum(char *buffer, int buffSize);

void timer(unsigned int sec);

void createPack(char *pack, int packID, int packType, char *buffer, int buffSize);

#endif
