#include "pacotes.cpp"

/*
Entrada:
Saída:
Descrição: Conecta ao servidor central do protocolo SLOW, que está rodando na porta 7033 do host slow.gmelodie.com
*/
array<uint8_t, 32> connect() {
    //Cria um pacote slow com as flags de conexão

    PACOTE_SLOW pacote_conexao = PACOTE_connect();

    auto buffer = PACOTE_connect_buffer(pacote_conexao);

    return buffer;
}

/**/
vector<PACOTE_SLOW> enviar_dados(PACOTE_SLOW setup, vector<uint8_t> &dados) {
    // Definindo as variáveis iniciais
    uint32_t seqnum_ant = setup.seqnum;
    
    if(dados.size() > DATA_MAX) {
        //Fragmentação dos dados

        vector<PACOTE_SLOW> pacotes_enviar;
        size_t offset = 0;
        int cont=0;
        while (offset < dados.size()) {
            
            //Calcula a flag: se for o último fragmento recebe no campo MB 0, caso contrário recebe 1
            uint8_t mb_flag = (offset + DATA_MAX >= dados.size()) ? 0 : 1;

            size_t tamanho_dados = min<size_t>(dados.size() - offset, DATA_MAX);
            vector<uint8_t> dados_fragmentados(dados.begin() + offset, dados.begin() + offset + tamanho_dados);

            PACOTE_SLOW pacote_enviar = PACOTE_preencher_struct(
                setup.sid,
                (setup.sttl_flags & 0xFFFFFFE0) | 0x04 | mb_flag, // Mantém os bits de sttl (limpando os bits da flag) e adiciona a flag de envio 
                seqnum_ant+1, // Incrementa o seqnum para cada pacote enviado
                seqnum_ant,
                1024, // Tamanho da janela (window) fixo em 1024
                0,
                cont, // Incrementa o identificador do fragmento (fo) para cada pacote enviado
                dados_fragmentados
            );

            pacotes_enviar.push_back(pacote_enviar);
            seqnum_ant += 1;
            offset += tamanho_dados;
            cont++;
        }

    } else {
        vector<PACOTE_SLOW> pacote_enviar;

        pacote_enviar.push_back(PACOTE_preencher_struct(
            setup.sid, 
            (setup.sttl_flags & 0xFFFFFFE0) | 0x04 | 0x00, 
            seqnum_ant+1, 
            seqnum_ant, 
            1024, // Tamanho da janela (window) fixo em 1024 
            0, // Identificador do fragmento (fo) fixo em 0, pois não há fragmentação
            0, // Identificador do fragmento (fo) fixo em 0, pois não há fragmentação
            dados  
        ));

        return pacote_enviar;
    }
}