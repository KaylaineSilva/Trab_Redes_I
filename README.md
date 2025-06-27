# Projeto SLOW Client - Trabalho de Redes I

Trabalho realizado para: Disciplina de Redes de Computadores (2025)

Autoras:
- Giovanna Lopes de Andrade – NUSP: 14574772
- Kaylaine Bessa da Silva – NUSP: 14747506


Repositório: [https://github.com/KaylaineSilva/Trab_Redes_I.git](https://github.com/KaylaineSilva/Trab_Redes_I.git)

---

## Sobre o projeto

Este projeto implementa um **cliente (peripheral)** que segue o protocolo **SLOW** (um protocolo ad hoc de transporte) para comunicação com a **central fornecida pela disciplina de Redes** ofertada no semestre 01/2025.

O cliente realiza todas as etapas fundamentais do protocolo:

- Conexão com o servidor (CONNECT)  
- Envio de dados (sem fragmentação)
- Envio de dados fragmentados (quando o payload excede o limite de 1440 bytes)

Servidor da central: slow.gmelodie.com:7033

---

##  Como clonar o projeto

Abra o terminal e execute:

git clone https://github.com/KaylaineSilva/Trab_Redes_I.git

## Para compilar

g++ main.cpp -o slow_client
g++ main.cpp pacotes.cpp -o slow_client

## Para executar

./slow_client

## Fluxo de execução do programa
- Conecta com a central (CONNECT)

- Envia uma mensagem de dados simples

- Envia dados fragmentados (exemplo com payload > 1440 bytes)

- Finaliza a conexão

Todo o fluxo é realizado via socket UDP, respeitando o formato de pacotes e as regras de controle de fluxo definidas no protocolo SLOW.

## Pré-requisitos
- Sistema Linux ou WSL (se estiver em Windows)

- Compilador g++

- Acesso à internet
