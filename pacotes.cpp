#include <bits/stdc++.h>
using namespace std;


#define TAM_MAX 1472
#define DATA_MAX 1440

typedef struct pacote_slow PACOTE_SLOW;


//Protótipos Funções
array<uint8_t, 1472> PACOTES_criar_envio(PACOTE_SLOW pac);
PACOTE_SLOW PACOTES_criar_struct_buffer(array<uint8_t,1472> &pacote_recebido);
PACOTE_SLOW PACOTE_preencher_struct(array<uint8_t, 16> &sid, uint32_t sttl_flags, uint32_t seqnum, uint32_t acknum, uint16_t window, uint8_t fid, uint8_t fo, vector<uint8_t> &data);


struct pacote_slow {
    array<uint8_t, 16> sid; //Armazena o id da sessão
    uint32_t sttl_flags; 
    uint32_t seqnum;
    uint32_t acknum;
    uint16_t window;
    uint8_t fid;
    uint8_t fo;
    vector<uint8_t> data; // Flexível
};

/*
Entrada: ponteiro para a struct para a construção do vetor que representa o pacote a ser enviado
Saída: um array que representa um pacote
Descrição: A partir de uma struct fornecida, a função cria um pacote representado por um array
*/
array<uint8_t, 1472> PACOTES_criar_envio(PACOTE_SLOW pac){
    array<uint8_t, 1472> buffer{};
    
    // Copiar sid (16 bytes)
    copy(pac.sid.begin(), pac.sid.end(), buffer.begin());

    // Inserir sttl_flags (4 bytes)
    memcpy(&buffer[16], &pac.sttl_flags, 4);

    // Inserir seqnum (4 bytes)
    memcpy(&buffer[20], &pac.seqnum, 4);

    // Inserir acknum (4 bytes)
    memcpy(&buffer[24], &pac.acknum, 4);

    // Inserir window (2 bytes)
    memcpy(&buffer[28], &pac.window, 2);

    // Inserir fid (1 byte)
    buffer[30] = pac.fid;

    // Inserir fo (1 byte)
    buffer[31] = pac.fo;

    // Inserir data (do byte 32 em diante, até o tamanho real de pac.data)
    size_t data_size = std::min<size_t>(pac.data.size(), DATA_MAX); //garante que o limite do tamanho dos dados não será ultrapassado
    copy(pac.data.begin(), pac.data.begin() + data_size, buffer.begin() + 32);

    return buffer;
}

/*
*/
PACOTE_SLOW PACOTE_connect() {
    // Cria um pacote SLOW com as flags de conexão
    PACOTE_SLOW pac;

    pac.sid = {}; // todos os bytes inicializados com 0
    pac.sttl_flags = (0 << 5) | 0x10; // sttl = 0, flags = Connect (bit 4) = 0x10
    pac.seqnum = 0;
    pac.acknum = 0;
    pac.window = 1024;
    pac.fid = 0;
    pac.fo = 0;
    
    return pac;
}


/**/
array<uint8_t, 32> PACOTE_connect_buffer(PACOTE_SLOW pac_connect) {
    array<uint8_t, 32> buffer{};
    
    // Copiar sid (16 bytes)
    copy(pac_connect.sid.begin(), pac_connect.sid.end(), buffer.begin());

    // Inserir sttl_flags (4 bytes)
    memcpy(&buffer[16], &pac_connect.sttl_flags, 4);

    // Inserir seqnum (4 bytes)
    memcpy(&buffer[20], &pac_connect.seqnum, 4);

    // Inserir acknum (4 bytes)
    memcpy(&buffer[24], &pac_connect.acknum, 4);

    // Inserir window (2 bytes)
    memcpy(&buffer[28], &pac_connect.window, 2);

    // Inserir fid (1 byte)
    buffer[30] = pac_connect.fid;

    // Inserir fo (1 byte)
    buffer[31] = pac_connect.fo;

    return buffer;
}

/*
Entrada: array contendo os campos do pacote recebido
Saída: struct com cada campo do pacote 
Descrição: A partir de um array representando o pacote recebido, a função preenche uma struct separando cada campo do protocolo SLOW
*/
PACOTE_SLOW PACOTES_criar_struct_buffer(array<uint8_t,1472> &pacote_recebido){
    PACOTE_SLOW pac;

    copy(pacote_recebido.begin(), pacote_recebido.begin()+16, pac.sid.begin()); //Copiar para sid os primeiros 16 bytes do pacote recebido
    
    // Bytes 16 a 19: sttl_flags (uint32_t, little endian)
    memcpy(&pac.sttl_flags, &pacote_recebido[16], 4);

    // Bytes 20 a 23: seqnum
    memcpy(&pac.seqnum, &pacote_recebido[20], 4);

    // Bytes 24 a 27: acknum
    memcpy(&pac.acknum, &pacote_recebido[24], 4);

    // Bytes 28 a 29: window
    memcpy(&pac.window, &pacote_recebido[28], 2);

    // Byte 30: fid
    pac.fid = pacote_recebido[30];

    // Byte 31: fo
    pac.fo = pacote_recebido[31];

    // Bytes 32 em diante: data (restante dos dados)
    pac.data.assign(pacote_recebido.begin() + 32, pacote_recebido.end());

    return pac;
}


/*
Entrada: Campos que compõem a struct pacote_slow
Saída: Uma struct contendo os campos passados
Descrição: A partir dos campos fornecidos, a função preenche e retorna uma struct
*/
PACOTE_SLOW PACOTE_preencher_struct(array<uint8_t, 16> &sid, uint32_t sttl_flags, uint32_t seqnum, uint32_t acknum, uint16_t window, uint8_t fid, uint8_t fo, vector<uint8_t> &data){
    PACOTE_SLOW pac;

    pac.sid = sid;
    pac.sttl_flags = sttl_flags;
    pac.seqnum = seqnum;
    pac.acknum = acknum;
    pac.window = window;
    pac.fid = fid;
    pac.fo = fo;
    pac.data = data;

    return pac;
}