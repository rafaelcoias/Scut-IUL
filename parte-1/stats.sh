#!/bin/bash 

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2021/2022
##
## Aluno: Nº: 105361      Nome: Rafael Cóias
## Nome do Módulo: stats.sh
## Descrição/Explicação do Módulo: 
##
##
###############################################################################

if [[ $# == 0 ]]; then
    ./error 2 && exit
fi

if [ $1 != "listar" ] && [ $1 != "registos" ] && [ $1 != "condutores" ]; then
    ./error 3 $1 && exit 
fi

if [ $1 == "registos" ] && [ $# -lt 2 ]; then
    ./error 2 && exit
fi

if [[ $1 == "listar" ]]; then
    if [[ -f autoestradas.txt ]]; then
            rm -f autoestradas.txt
    fi
    touch autoestradas.txt
    for AUTO in $(cat portagens.txt | cut -d ":" -f3); do
        if [[ $(grep $AUTO autoestradas.txt | wc -l) == 0 ]]; then
            echo $AUTO >> autoestradas.txt
        fi
    done
    ./success 6 autoestradas.txt
    rm -f autoestradas.txt
fi

if [[ $1 == "registos" ]]; then
    if [[ $2 =~ ^[0-9]+$ ]]; then
        if [[ -f registos.txt ]]; then
            rm -f registos.txt
        fi
        touch registos.txt
        for LANCO in $(cat relatorio_utilizacao.txt | cut -d ":" -f2); do
            if [ $(grep $LANCO relatorio_utilizacao.txt | wc -l) -ge $2 ] && [ $(grep $LANCO registos.txt | wc -l) == 0 ]; then 
                echo $(echo $LANCO | uniq) >> registos.txt
            fi
        done
        else
            ./error 3 $2 && exit
    fi
    ./success 6 registos.txt
    rm -f registos.txt
fi

if [[ $1 == "condutores" ]]; then
    if [[ -f nome_condutores.txt ]]; then
            rm -f nome_condutores.txt
    fi
    touch nome_condutores.txt
    for DRIVER in $(cat relatorio_utilizacao.txt | cut -d ":" -f3); do
        NMB=$(echo $DRIVER | cut -d "D" -f2)
        NAME=$(grep $NMB pessoas.txt | cut -d ":" -f2)
        if [[ $(grep "$NAME" nome_condutores.txt | wc -l) == 0 ]]; then
            echo $(echo "$NAME") >> nome_condutores.txt
        fi
    done
    ./success 6 nome_condutores.txt
    rm -f nome_condutores.txt
fi

