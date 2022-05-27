#!/bin/bash

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2021/2022
##
## Aluno: Nº: 105361      Nome: Rafael Cóias
## Nome do Módulo: lista_condutores.sh
## Descrição/Explicação do Módulo: 
##
##
###############################################################################

if ! [[ -f pessoas.txt ]]; then 
    ./error 1 pessoas.txt && exit
    else
        touch condutores.txt
        awk -F ':' '{print "ID"$3 "-" $2";" $1";" $4";" $3";150"}' pessoas.txt > condutores.txt
        ./success 2 condutores.txt
fi