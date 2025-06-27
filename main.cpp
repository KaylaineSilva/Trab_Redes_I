/*
Trabalho de Redes - Parte I
Autoras:
- Giovanna Lopes de Andrade nusp 14574772
- Kaylaine Bessa da Silva nusp 14747506

Descrição: implementar um protocolo ad hoc na camada de transporte para controle de fluxo de dados: o SLOW. É implementado o peripheral, que se comunica 
com uma central disponível em: slow.gmelodie.com:7033
*/

#include "funcoes.cpp"

int main() {
    PACOTE_SLOW setup;
    addrinfo *res;
    int sock;
    socklen_t addrlen;
    bool deuCerto;

    auto resultado = Connect(); //Conectar com o servidor
    
    // Inicializando as variáveis de controle da conexão
    sock = get<0>(resultado);
    res = get<1>(resultado);
    addrlen = get<2>(resultado);
    setup = get<3>(resultado);

    vector<uint8_t> dados = {'h', 'e', 'l', 'l', 'o', 'w', ' ', 'w', 'o', 'r', 'l', 'd'}; // Dados de exemplo
    auto result = Envio_dados(setup, res, sock, addrlen, dados, false);
    
    result.first ? cout << "Dados enviados com sucesso!\n" << endl : cout << "Erro ao enviar dados.\n" << endl;
    setup = result.second;
    
    vector<uint8_t> dados_frag(3000);
    for (size_t i = 0; i < dados_frag.size(); i++) {
        dados_frag[i] = i % 256;  // Preenche com valores de 0 a 255, repetindo  
    }

    auto result2 = Envio_dados(setup, res, sock, addrlen, dados_frag, false);
    result2.first ? cout << "Dados fragmentados enviados com sucesso!\n" << endl : cout << "Erro ao enviar dados fragmentados.\n" << endl;

    // Fechar o socket
    close(sock);

    return 0;
}