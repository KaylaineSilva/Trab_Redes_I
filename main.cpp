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
    
    /*auto result = Envio_dados(setup, res, sock, addrlen, dados, true);
    
    result.first ? cout << "Dados enviados com sucesso!" << endl : cout << "Erro ao enviar dados." << endl;
    setup = result.second;
    
    if(deuCerto) cout<< "Dados enviados e recebidos com sucesso!" << endl;
    else cout << "Erro ao enviar dados." << endl;*/

    /*auto result = Disconnect(setup, res, sock, addrlen);

    result.first ? cout << "Desconexão bem-sucedida!" << endl : cout << "Erro ao desconectar." << endl;
    setup = result.second;*/

    // Fechar o socket
    close(sock);

    return 0;
}