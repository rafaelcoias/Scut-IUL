#!/bin/bash 

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2021/2022
##
## Aluno: Nº: 105361      Nome: Rafael Cóias
## Nome do Módulo: faturacao.sh
## Descrição/Explicação do Módulo: 
##
##
###############################################################################

if ! [[ -f relatorio_utilizacao.txt ]]; then 
    ./error 1 relatorio_utilizacao.txt && exit
fi

if [[ -f faturas.txt ]]; then 
    rm -f faturas.txt
fi

touch faturas.txt

if [[ -s relatorio_utilizacao.txt ]]; then
    for LINE in $(cat relatorio_utilizacao.txt); do
        INFO=$(echo $LINE)
        ID=$(grep $INFO relatorio_utilizacao.txt | cut -d ":" -f3 | head -1)
        NMB=$(echo $ID | cut -d "D" -f2)
        NAME=$(grep $NMB pessoas.txt | cut -d ":" -f2)
        if [[ $(grep $NMB faturas.txt | wc -l) == 0 ]]; then
            echo "Cliente: $NAME" >> faturas.txt
            CRD=0
            for LINEE in $(grep "$NMB" relatorio_utilizacao.txt); do
                echo $LINEE >> faturas.txt
                SUM=0
                SUM=$(echo $LINEE | cut -d ":" -f5)
                CRD=$(($CRD+$SUM))
            done
            echo "Total: $CRD créditos" >> faturas.txt
            echo "" >> faturas.txt
        fi
    done
fi

./success 5 faturas.txt