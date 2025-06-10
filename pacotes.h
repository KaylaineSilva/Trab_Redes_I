#include <bits/stdc++.h>
using namespace std;


#define TAM_MAX 1472
#define DATA_MAX 1440

typedef struct pacote_slow PACOTE_SLOW;

//Funções
array<uint8_t, 1472> PACOTES_criar_envio(PACOTE_SLOW pac);
PACOTE_SLOW PACOTES_criar_buffer(array<uint8_t,1472> &pacote_recebido);
PACOTE_SLOW PACOTE_preencher_struct(array<uint8_t, 16> &sid, uint32_t sttl_flags, uint32_t seqnum, uint32_t acknum, uint16_t window, uint8_t fid, uint8_t fo, vector<uint8_t> &data);
