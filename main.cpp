/*
Trabalho de Redes - Parte I
Autoras:
- Giovanna Lopes de Andrade nusp 14574772
- Kaylaine Bessa da Silva nusp 14747506

Descrição: implementar um protocolo ad hoc na camada de transporte para controle de fluxo de dados: o SLOW. É implementado o peripheral, que se comunica 
com uma central disponível em: slow.gmelodie.com:7033
*/

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "funcoes.cpp"

int main() {
    // Criar socket UDP     
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Erro ao criar socket");
        return 1;
    }

    //Etapa 2: Resolver o endereço do servidor
    struct addrinfo hints{}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int err = getaddrinfo("slow.gmelodie.com", "7033", &hints, &res);
    if (err != 0) {
        cerr << "Erro em getaddrinfo: " << gai_strerror(err) << std::endl;
        return 1;
    }
    
    auto buffer = connect();

    // Etapa 3: Enviar o pacote de conexão
    ssize_t enviados = sendto(sock, buffer.data(), buffer.size(), 0, res->ai_addr, res->ai_addrlen);
    if (enviados < 0) {
        perror("Erro ao enviar pacote");
        return 1;
    }

    cout << "Pacote CONNECT enviado com sucesso!\n";

    // Etapa 5: Aguardar resposta do servidor
    array<uint8_t, 1472> resposta{};
    socklen_t addrlen = res->ai_addrlen;
    ssize_t recebidos = recvfrom(sock, resposta.data(), resposta.size(), 0, res->ai_addr, &addrlen);

    if (recebidos < 0) {
        perror("Erro ao receber resposta");
        return 1;
    }

    //Etapa 6: Desserializar resposta e imprimir
    PACOTE_SLOW setup = PACOTES_criar_struct_buffer(resposta);

    //Envio de dados
    vector<uint8_t> dados = {'h', 'e', 'l', 'l', 'o', 'w', ' ', 'w', 'o', 'r', 'l', 'd'}; // Dados de exemplo
    vector<PACOTE_SLOW> pacotes_dados_enviar = enviar_dados(setup, dados);
    
    return 0;
}