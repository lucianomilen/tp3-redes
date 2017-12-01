//CLIENTE.C - LUCIANO OTONI MILEN [2012079754]

#include "common.h"

int getBuff(char *buffer, int buffSize){
        int nCycles = 0; // como explicado na documentação, o primeiro pacote estava sendo ignorado pelo cliente, o que atrapalhava na hora de salvar o arquivo.
        int recBytes = 0;
        char pack[HEADER + buffSize];
        char ACKMSG[HEADER + buffSize];

        recBytes = tp_recvfrom(sockID, pack, HEADER + buffSize, (so_addr*)&addr);
        recPacks++;

        if(nCycles != 0)
                // o pacote recebido tem ID diferente do que era pra vir
                while(getPackID(pack) != packID) {
                        createPack(ACKMSG, getPackID(pack), ACK_TYPE, HEADER + pack, buffSize); // cria um novo pacote com o ID do último recebido
                        tp_sendto(sockID, pack, HEADER + buffSize, (so_addr*)&addr);
                        sentACKs++;
                        recBytes = tp_recvfrom(sockID, pack, HEADER + buffSize, (so_addr*)&addr);
                        dispPacks++; //incrementa o contador de pacotes descartados
                }

        //o pacote do tipo FINAL_TYPE significa que chegou o momento de fechar a conexão
        if(pack[HEADER - 2] == FINAL_TYPE) {
                createPack(ACKMSG, getPackID(pack), FINAL_TYPE, HEADER + pack, buffSize);
                tp_sendto(sockID, ACKMSG, HEADER + buffSize, (so_addr*)&addr);
        }

        // o pacote veio como esperado. manda um pacote do tipo ACK pro servidor
        else{
                createPack(ACKMSG, getPackID(pack), ACK_TYPE, HEADER + pack, buffSize);
                tp_sendto(sockID, ACKMSG, HEADER + buffSize, (so_addr*)&addr);
        }

        sentACKs++;
        packID++; //incrementa os contadores pra estatística

        strncpy(buffer, HEADER + pack, buffSize); // coloca o pacote no buffer
        nCycles = 1;
        return recBytes - HEADER; // o tamanho de bytes obtidos deve dispensar o tamanho do cabeçalho!!!
}

int sendFileName(char *buffer, int buffSize){ // envia o nome do arquivo que o cliente quer receber do servidor

        char pack[buffSize + HEADER];
        createPack(pack, packID, DATA_TYPE, buffer, buffSize);

        int sent = 0;
        sent = tp_sendto(sockID, pack, buffSize + HEADER, (so_addr*)&addr);
        sentPacks++;

        return sent; //retorna a quantidade de bytes do nome do arquivo para verificação de erros
}

int startClient(char *host, int port){ // inicia a conexão do cliente usando as funções do tp_socket.h

        tp_init();
        sockID = tp_socket(0);

        if(sockID < 0) {
                printf("Client socket failed\n");
                return -1;
        }

        if(tp_build_addr(&addr, host, port) < 0) {
                printf("Client server bind failed\n");
                return -1;
        }

        return sockID; //retorna o ID do socket para comunicação
}

int main(int argc, char * argv[]) {
        // inicializa variaveis
        int serverPort, buffSize;
        char *buffer, *serverHost, *fileName;
        double initTime, elapsedTime, endTime, throughtput;
        unsigned int sumRecBytes = 0;
        int recBytes = 0;
        FILE *fp;

        // verifica comando para rodar o cliente
        if(argc < 5) {
                printf("Usage: ./cliente <serverHost> <serverPort> <fileName> <buffSize>\n");
                exit(1);
        }
        // aloca memorias...
        serverHost = (char*) malloc(sizeof(char) * strlen(argv[1]));
        strcpy(serverHost, argv[1]);
        serverPort = atoi(argv[2]);
        fileName = (char*) malloc(sizeof(char) * (strlen(argv[3]) + 8));
        strcpy(fileName, argv[3]);
        buffSize = atoi(argv[4]);

        // inicializa o cliente com a porta e o local do servidor
        startClient(serverHost, serverPort);

        // inicializa os contadores para calculo do tempo de execução

        struct timeval time;
        gettimeofday(&time, NULL);

        initTime = time.tv_sec + (time.tv_usec/1000000.0);

        // manda pro servidor o nome do arquivo que o cliente quer
        buffer = (char*) malloc(sizeof(char) * buffSize);
        strcpy(buffer, fileName);

        if(sendFileName(buffer, buffSize) < 0) {
                printf("Unable to send file name\n");
                return 0;
        }

        // cria um arquivo com _ no final para gravar os buffers recebidos
        strcat(fileName, "_");
        fp = fopen(fileName, "w+");

        //enquanto tiver recebendo bytes (além do tamanho do cabeçalho) pega mais buffer do servidor
        while((recBytes = getBuff(buffer, buffSize)) > 0) {
                sumRecBytes += fwrite(buffer, 1, recBytes, fp); //conta quantos bytes tão sendo escritos no arquivo
        }
        // encerra tudo
        free(buffer);
        fclose(fp);
        free(serverHost);
        free(fileName);

        // faz o calculo do tempo gasto na transferencia
        struct timeval end;
        gettimeofday(&end, NULL);
        endTime = end.tv_sec + (end.tv_usec/1000000.0);
        elapsedTime = endTime - initTime;

        // calcula a vazão e mostra estatísticas na tela
        throughtput = (sumRecBytes / elapsedTime);
        printf("\nSTATS\nbufferSize = \%5u byte(s)\nThroughtput = \%10.2f kbps (\%u Bytes in \%3.06f seconds)\n",buffSize,throughtput, sumRecBytes, elapsedTime);

        //Fecha conexão
        printf("Resent: %d packets\nSent: %d packets\nReceived: %d packets\nSent: %d ACKs\nreceived: %d ACKs\nDisposed: %d packets\n\n",resentPacks, sentPacks, recPacks, sentACKs, recACKs, dispPacks);
        close(sockID);

        return 0;
}
