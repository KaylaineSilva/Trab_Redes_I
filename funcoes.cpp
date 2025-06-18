#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "pacotes.cpp"

/*Protótipos*/
PACOTE_SLOW Enviar_ACK_sem_dados(PACOTE_SLOW setup, addrinfo *res, int sock, socklen_t addrlen);


/*
Entrada: Nenhuma
Saída: Um array de uint8_t com 32 bytes representando o pacote de conexão
Descrição: Cria um pacote de conexão SLOW com as flags apropriadas e retorna um buffer contendo os dados do pacote.
*/
array<uint8_t, 32> connect_buffer() {
    //Cria um pacote slow com as flags de conexão

    PACOTE_SLOW pacote_conexao = PACOTE_connect();

    auto buffer = PACOTE_connect_disconnect_buffer(pacote_conexao);

    return buffer;
}

/*
Entrada: Nenhuma
Saída: Uma tupla contendo:
- sock: Descritor do socket UDP criado
- res: Ponteiro para a estrutura addrinfo contendo as informações do endereço do servidor
- addrlen: Tamanho do endereço do servidor
- setup: Estrutura PACOTE_SLOW contendo as informações de configuração da conexão
Descrição: Conecta ao servidor central do protocolo SLOW, que está rodando na porta 7033 do host slow.gmelodie.com
*/

tuple<int, addrinfo *, socklen_t, PACOTE_SLOW> Connect(){
    // Criar socket UDP     
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Erro ao criar socket");
    }

    //Etapa 2: Resolver o endereço do servidor
    struct addrinfo hints{}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int err = getaddrinfo("slow.gmelodie.com", "7033", &hints, &res);
    if (err != 0) {
        cerr << "Erro em getaddrinfo: " << gai_strerror(err) << endl;
    }
    
    auto buffer = connect_buffer();

    // Etapa 3: Enviar o pacote de conexão
    ssize_t enviados = sendto(sock, buffer.data(), buffer.size(), 0, res->ai_addr, res->ai_addrlen);
    if (enviados < 0) {
        perror("Erro ao enviar pacote");
    }

    cout << "Pacote CONNECT enviado com sucesso!\n";

    // Etapa 5: Aguardar resposta do servidor
    vector<uint8_t> resposta(1472);
    socklen_t addrlen = res->ai_addrlen;
    ssize_t recebidos = recvfrom(sock, resposta.data(), resposta.size(), 0, res->ai_addr, &addrlen);

    resposta.resize(recebidos);

    if (recebidos < 0) {
        perror("Erro ao receber resposta");
    } else {
        cout << "Conexão estabelecida com sucesso!\n";
    }

    PACOTE_SLOW setup = PACOTES_criar_struct_buffer(resposta);
    cout << "Pacote recebido do setup" << endl;
    imprimir(setup);

    setup = Enviar_ACK_sem_dados(setup, res, sock, addrlen);

    cout << "Pacote recebido do setup do ACK sem dados" << endl;
    imprimir(setup);

    return make_tuple(sock, res, addrlen, setup);
}

/**/
PACOTE_SLOW Enviar_ACK_sem_dados(PACOTE_SLOW setup, addrinfo *res, int sock, socklen_t addrlen) {
    // Enviar ACK sem dados
    uint32_t sttl = (setup.sttl_flags >> 5); // Extrai o TTL original
    uint8_t flags = 0x04; // Sempre com ACK ligado
    vector<uint8_t> data{}; // Dados vazios, pois é um ACK sem dados

    PACOTE_SLOW pacote_ack = PACOTE_preencher_struct(
        setup.sid,
        (sttl << 5) | flags, // Mantém os bits de sttl (limpando os bits da flag) e adiciona a flag de envio 
        setup.seqnum + 1, // Incrementa o seqnum para o ACK
        setup.seqnum, // Usa o seqnum atual como acknum
        1024, // Tamanho da janela (window) fixo em 1024
        0, // Identificador do fragmento (fo) fixo em 0, pois não há fragmentação
        0, // Identificador do fragmento (fo) fixo em 0, pois não há fragmentação
        data // Dados vazios, pois é um ACK sem dados
    );


    cout << "Pacote de ACK sem dados enviado: " << endl;
    imprimir(pacote_ack); // Imprime o pacote de ACK sem dados PARA depuração

    auto buffer = PACOTES_criar_envio(pacote_ack);

    cout << "Tamanho do buffer enviado: " << buffer.size() << endl;
    ssize_t enviados = sendto(sock, buffer.data(), buffer.size(), 0, res->ai_addr, res->ai_addrlen);
    if (enviados < 0) {
        perror("Erro ao enviar ACK sem dados");
    }

    cout << "ACK sem dados enviado com sucesso!\n";
    // Aguardar resposta do servidor para o ACK sem dados
    vector<uint8_t> resposta(1472);
    ssize_t recebidos = recvfrom(sock, resposta.data(), resposta.size(), 0, res->ai_addr, &addrlen);
    if (recebidos < 0) {
        perror("Erro ao receber resposta do ACK sem dados");
    } else {
        resposta.resize(recebidos);
        cout << "Resposta recebida para o ACK sem dados com sucesso!\n";

        PACOTE_SLOW setup_recebido = PACOTES_criar_struct_buffer(resposta);
        return setup_recebido;
    }

    return setup; // Retorna o setup original em caso de erro
}

/*
Entrada:
- setup: Estrutura PACOTE_SLOW contendo as informações de configuração da conexão
- dados: Vetor de bytes contendo os dados a serem enviados
- revive: Booleano indicando se é necessário reviver a conexão 
Saída:
- Retorna um vetor de PACOTE_SLOW contendo os pacotes a serem enviados
Descrição:
Esta função prepara os pacotes de dados a serem enviados, fragmentando-os se necessário, e define as flags apropriadas para o envio.
*/
vector<PACOTE_SLOW> enviar_dados(PACOTE_SLOW setup, vector<uint8_t> &dados, bool revive) {
    // Definindo as variáveis iniciais
    uint32_t seqnum_ant = setup.seqnum;

    uint32_t sttl = (setup.sttl_flags >> 5);         // Extrai o TTL original
    uint8_t flags = 0x04;                            // Sempre com ACK ligado
    
    if (revive) flags |= 0x08; // Liga o bit Revive se for necessário reviver a conexão
    
    if(dados.size() > DATA_MAX) {
        //Fragmentação dos dados

        vector<PACOTE_SLOW> pacotes_enviar;
        size_t offset = 0;
        int cont=0;

        while (offset < dados.size()) {
            flags = 0x04;

            //Calcula a flag: se for o último fragmento recebe no campo MB 0, caso contrário recebe 1
            uint8_t mb_flag = (offset + DATA_MAX >= dados.size()) ? 0 : 1;

            if (mb_flag == 1) flags |= 0x01; // Liga o bit MB se for um fragmento
        
            size_t tamanho_dados = min<size_t>(dados.size() - offset, DATA_MAX);
            vector<uint8_t> dados_fragmentados(dados.begin() + offset, dados.begin() + offset + tamanho_dados);

            PACOTE_SLOW pacote_enviar = PACOTE_preencher_struct(
                setup.sid,
                (sttl << 5) | flags, // Mantém os bits de sttl (limpando os bits da flag) e adiciona a flag de envio 
                seqnum_ant+1, // Incrementa o seqnum para cada pacote enviado
                seqnum_ant,
                1024, // Tamanho da janela (window) fixo em 1024
                0,
                cont, // Incrementa o identificador do fragmento (fo) para cada pacote enviado
                dados_fragmentados
            );

            cout << pacote_enviar.sttl_flags << endl;
            pacotes_enviar.push_back(pacote_enviar);
            seqnum_ant += 1;
            offset += tamanho_dados;
            cont++;
        }

        return pacotes_enviar;

    } else {
        vector<PACOTE_SLOW> pacote_enviar;

        pacote_enviar.push_back(PACOTE_preencher_struct(
            setup.sid, 
            (sttl << 5) | flags, 
            seqnum_ant+1, 
            seqnum_ant, 
            1024, // Tamanho da janela (window) fixo em 1024 
            0, // Identificador do fragmento (fo) fixo em 0, pois não há fragmentação
            0, // Identificador do fragmento (fo) fixo em 0, pois não há fragmentação
            dados  
        ));

        //imprimir o pacote_enviar
        cout << "Pacote de dados a ser enviado: " << endl;
        imprimir(pacote_enviar[0]);

        return pacote_enviar;
    }
}

/*
Entrada:
- setup: Estrutura PACOTE_SLOW contendo as informações de configuração da conexão
- res: Ponteiro para a estrutura addrinfo contendo as informações do endereço do servidor
- sock: Descritor do socket utilizado para comunicação
- addrlen: Tamanho do endereço do servidor
- dados: Vetor de bytes contendo os dados a serem enviados
- revive: Booleano indicando se é necessário reviver a conexão (default false)
Saída:
- Retorna um PACOTE_SLOW contendo o pacote de dados enviado ou o pacote de desconexão
Descrição:
Esta função envia os dados para o servidor, fragmentando-os se necessário, e aguarda a resposta do servidor para cada pacote enviado. 
Se não receber uma resposta, reenviará o pacote até um limite de tentativas.
*/
pair<bool, PACOTE_SLOW> Envio_dados(PACOTE_SLOW setup, addrinfo *res, int sock, socklen_t addrlen, vector<uint8_t> &dados, bool revive = false) {

    //Envio de dados
    vector<PACOTE_SLOW> pacotes_dados_enviar = enviar_dados(setup, dados, revive);

    vector<vector<uint8_t>> buffers_enviar;

    vector<uint8_t> resposta_dados(1472);

    for (const auto& pacote : pacotes_dados_enviar) {
        auto buffer = PACOTES_criar_envio(pacote);
        buffers_enviar.push_back(buffer);
    }

    int cont = 0;

    // Enviar pacotes de dados
    for (int i=0; i<buffers_enviar.size(); i++) {
        ssize_t enviados = sendto(sock, buffers_enviar[i].data(), buffers_enviar[i].size(), 0, res->ai_addr, res->ai_addrlen);
        if (enviados < 0) {
            perror("Erro ao enviar pacote de dados");
        }
        cout << "Pacote de dados enviado com sucesso!\n";

        // Aguardar resposta do servidor para cada pacote enviado
        
        ssize_t recebidos = recvfrom(sock, resposta_dados.data(), resposta_dados.size(), 0, res->ai_addr, &addrlen);
        if (recebidos < 0) {
            perror("Erro ao receber resposta do pacote de dados");
            
            //Reenviar o pacote de dados se não receber resposta
            i--; //O índice i não deve ser incrementado, pois o pacote não foi enviado com sucesso
            cont++;

            if(cont > 3) { // Limite de tentativas
                cerr << "Erro ao receber resposta do pacote de dados. Tentativas excedidas.\n";
            }
        } else {
            resposta_dados.resize(recebidos); 

            cont = 0;
            cout << "Resposta recebida para o pacote de dados " << i << " com sucesso!\n";

            if(revive && (resposta_dados[16] & 0x02) == 0) {
                cerr << "Erro: o servidor não confirmou a revivificação da conexão.\n";
                return make_pair(false, setup);
            }

            return make_pair(true, PACOTES_criar_struct_buffer(resposta_dados));
        }
    }

    return make_pair(false, setup);
}

/*
Entrada:
- setup: Estrutura PACOTE_SLOW contendo as informações de configuração da conexão
- res: Ponteiro para a estrutura addrinfo contendo as informações do endereço do servidor
- sock: Descritor do socket utilizado para comunicação
- addrlen: Tamanho do endereço do servidor
Saída:
- Retorna um PACOTE_SLOW contendo o pacote de desconexão enviado ou o pacote de desconexão recebido
Descrição:
Esta função envia um pacote de desconexão para o servidor, aguardando a resposta do servidor para confirmar a desconexão.
*/
pair<bool, PACOTE_SLOW> Disconnect(PACOTE_SLOW setup, addrinfo *res, int sock, socklen_t addrlen) {
    uint32_t sttl = (setup.sttl_flags >> 5);   // Extrai o TTL original
    
    // Cria um pacote SLOW com as flags de desconexão
    PACOTE_SLOW pacote_desconexao;
    
    pacote_desconexao.sid = setup.sid; // Mantém o mesmo SID da conexão
    pacote_desconexao.sttl_flags = (sttl << 5) | 0x28; // sttl = 0, flags = Ack(1) (bit 2), Connect(1) (bit 4), Revive(1) (bit 3) = 0x28
    pacote_desconexao.seqnum = setup.seqnum + 1; // Incrementa o seqnum para a desconexão
    pacote_desconexao.acknum = setup.seqnum; // Usa o seqnum atual como acknum
    pacote_desconexao.window = 1024; // Tamanho da janela (window) fixo em 1024
    pacote_desconexao.fid = 0; // Identificador do fragmento (fo) fixo em 0, pois não há fragmentação
    pacote_desconexao.fo = 0; // Identificador do fragmento (fo) fixo em 0, pois não há fragmentação

    auto buffer = PACOTE_connect_disconnect_buffer(pacote_desconexao);

    ssize_t enviados = sendto(sock, buffer.data(), buffer.size(), 0, res->ai_addr, res->ai_addrlen);
    if (enviados < 0) {
        perror("Erro ao enviar pacote de desconexão");
        
    }
    
    cout << "Pacote de desconexão enviado com sucesso!\n";

    // Aguardar resposta do servidor para o pacote de desconexão
    vector<uint8_t> resposta(1472);
    ssize_t recebidos = recvfrom(sock, resposta.data(), resposta.size(), 0, res->ai_addr, &addrlen);
    
    resposta.resize(recebidos); 
    if (recebidos < 0) {
        perror("Erro ao receber resposta do pacote de desconexão");
        
    } else {
        cout << "Resposta recebida para o pacote de desconexão com sucesso!\n";
        return make_pair(true, PACOTES_criar_struct_buffer(resposta));
    }

    return make_pair(false, setup);
}