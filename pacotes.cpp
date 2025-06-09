#include "pacotes.h"

struct pacote_slow {
    array<uint8_t, 16> sid; //Armazena o is da sessão
    uint32_t sttl_flags; 
    uint32_t seqnum;
    uint32_t acknum;
    uint16_t window;
    uint8_t fid;
    uint8_t fo;
    vector<uint8_t> data; // Flexível
};

array<uint8_t, 1472> PACOTES_criar_envio(){

}

PACOTE_SLOW PACOTES_criar_buffer(uint8_t pacote_recebido[1472]){
    PACOTE_SLOW pac;

    pac.sid = ; //Primeiros 16 bytes
    pac.sttl_flags = pacote_recebido[17]; //16ºbyte-19ºbyte
    pac.seqnum = seqnum; //20ºbyte-23ºbyte
    pac.acknum = acknum; //24ºbyte-27ºbyte
    pac.window = window; //28ºbyte-29ºbyte
    pac.fid = fid; //30ºbyte
    pac.fo = fo; //31ºbyte
    pac.data = data; //todo o resto do vetor

    return pac;
}

PACOTE_SLOW PACOTE_preencher_struct(){

    
}