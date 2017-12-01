//SERVIDOR.C - LUCIANO OTONI MILEN [2012079754]

#include "common.h"

void closeServer(){ //finaliza o servidor
        printf("Resent: %d packets\nSent: %d packets\nReceived: %d packets\nDisposed: %d packets\nSent: %d ACKs\nReceived: %d ACKs\n",resentPacks, sentPacks, recPacks, dispPacks, sentACKs, recACKs);
        close(sockID);
}


int sendBuff(char *buffer, int buffSize){

        int ACKs = -1, SENT = 0, pack_id; //ACKs começa como -1 pra ser incrementado pra 1 logo em seguida

        unsigned MSG_SIZE = HEADER + buffSize; // SEMPRE devemos somar o tamanho do cabeçalho ao tamanho da mensagem

        char pack[MSG_SIZE];
        char ACKMSG[MSG_SIZE];
        char RESENT[MSG_SIZE];

        createPack(pack, packID, DATA_TYPE, buffer, buffSize); //cria um pacote do tipo DATA_TYPE para enviar dados ao cliente

        ACKs = tp_recvfrom(sockID, ACKMSG, MSG_SIZE, (so_addr*)&addr); // recebe os ACKs do cliente

        //nenhum ACK foi recebido. ou seja, o primeiro pacote deve ser reenviado
        while(ACKs == -1) {
                pack_id = 0; //com o id 0, o primeiro
                createPack(RESENT, pack_id, DATA_TYPE, HEADER + recBuff, buffSize);
                tp_sendto(sockID, RESENT, buffSize + HEADER, (so_addr*)&addr);
                sentPacks++;
                resentPacks++; //incrementa os reenvios
                ACKs = tp_recvfrom(sockID, ACKMSG, MSG_SIZE, (so_addr*)&addr);
        }
        recACKs++;

        //o ACK recebido veio com ID errado. desta forma, o último ACK recebido deve ser atualizado
        if(getPackID(ACKMSG) != lastACKID) {
                lastACKID = getPackID(ACKMSG);
        }

        lastACKID++;

        strcpy(recBuff, pack); //grava no buffer o pacote recebido

        // manda o pacote pro cliente, com o timeout registrado!!!
        SENT = tp_sendto(sockID, pack, MSG_SIZE, (so_addr*)&addr);
        sentPacks++;
        packID++;
        timer(1); // parte importante

        return SENT; // retorna quantos Bytes foram enviados
}

int signalEOF(int buffSize){ // função simples que envia o sinal de que acabou a transferencia

        char pack[HEADER + buffSize];

        createPack(pack, packID, FINAL_TYPE, NULL, 0);

        tp_sendto(sockID, pack, HEADER, (so_addr*)&addr);
        sentPacks++;

        do
        {
                tp_recvfrom(sockID, pack, HEADER+buffSize, (so_addr*)&addr);
                recACKs++;
        } while(pack[HEADER - 2] != FINAL_TYPE);

        timer(1); // com timeout no pacote
        return 1;
}

int startServer(int serverPort, int buffSize){ //init o servidor com as funções da lib fornecidas

        tp_init();

        sockID = tp_socket(serverPort);

        if(sockID < 0) {
                printf("Unable to create server socket\n");
                return -1;
        }

        packID = 0;
        lastACKID = 0;
        recACKs = 0;
        sentACKs = 0;
        recPacks = 0;
        sentPacks = 0;
        dispPacks = 0;
        resentPacks = 0; // as variáveis de estatística definidas no common.h são inicializadas

        recBuff = (char*) malloc(sizeof(char) * (HEADER + buffSize));

        return sockID; //cria o socket e retorna pro main informar o cliente
}

int getFileName(char *buffer, int buffSize){

        char fileNamePack[HEADER + buffSize]; // o tamanho deve considerar o tamanho do cabeçalho sempre...
        int rec_packs = tp_recvfrom(sockID, fileNamePack, HEADER + buffSize, (so_addr*)&addr);
        recPacks++; // tem que incrementar o numero de pacotes recebidos
        strncpy(buffer, HEADER + fileNamePack, buffSize); //joga no buffer com o tamanho correto
        timer(1); //timeout
        return rec_packs - HEADER; // conta somente quantos bytes do nome do arquivo foram recebidos
}

int main(int argc, char *argv[]){

        int serverPort, buffSize, readBytes;
        char *buffer;

        if(argc < 3) { //tem que rodar com os argumentos corretos!
                printf("Usage: ./servidor <serverPort> <buffSize>\n");
                exit(1);
        }

        serverPort = atoi(argv[1]);
        buffSize = atoi(argv[2]);

        //inicializa o servidor, com o buffSize fornecido como parametro
        startServer(serverPort, buffSize);
        printf("Waiting client connection...\n");

        // inicializa o buffer com a memoria definida no buffSize
        buffer = (char*) malloc(sizeof(char) * buffSize);
        if(getFileName(buffer, buffSize) < 0) { // verifica se recebeu direito o nome do arquivo
                printf("Unable to get file name\n");
                return 0;
        }
        // tenta abrir o arquivo que o cliente pediu
        FILE *fp = fopen(buffer, "r");

        if(fp == NULL) { //tratamento de erro
                printf("Unable to open file\n");
                closeServer();
                return 0;
        }

        // contabiliza o numero de bytes lidos
        readBytes = 0;

        while((readBytes = fread(buffer, 1, buffSize, fp)) != 0) {
                sendBuff(buffer, readBytes); //enquanto tiver byte pra ler manda pro cliente
        }
        //acabou, pode avisar o cliente
        if(signalEOF(buffSize) == 1) {
                printf("Arquivo enviado ao cliente!\n");
        }

        //fecha tudo
        free(buffer);
        fclose(fp);
        closeServer();

        printf("\nConnection successfully terminated\n");

        return 0;
}
