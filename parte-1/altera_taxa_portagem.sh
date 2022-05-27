#!/bin/bash

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2021/2022
##
## Aluno: Nº: 105361      Nome: Rafael Cóias
## Nome do Módulo: altera_taxa_portagem.sh
## Descrição/Explicação do Módulo: 
##
##
###############################################################################

if [ $# -lt 3 ]; then
    ./error 2 && exit
fi

if [[ $1 == *[-]* ]]; then
    FIRST=$(echo $1 | cut -d "-" -f1)
    SND=$(echo $1 | cut -d "-" -f2)
    if ! [[ $FIRST==[A-Z][a-z]* ]]; then
        ./error 3 $1 && exit
    fi
    if ! [[ $SND==[A-Z][a-z]* ]]; then
        ./error 3 $1 && exit
    fi
    else
        ./error 3 $1 && exit
fi

n='^[0-9]+$'
if ! [[ $3 =~ $n ]]; then
    ./error 3 $3 && exit
fi

if [[ $3 == [0]* ]]; then
    ./error 3 $3 && exit
fi

if ! [[ -f portagens.txt ]]; then
    echo "1:$1:$2:$3" > portagens.txt
    ./success 3 $1
    else
        if grep -q $1 portagens.txt; then 
            ID=$(grep "$1" portagens.txt | cut -d ":" -f1)
            TAXA=$(grep "$1" portagens.txt | cut -d ":" -f4)
            sed -i "s/$ID:$1:$2:$TAXA/$ID:$1:$2:$3/" portagens.txt
            ./success 3 $1
            else
                ID=$(cat portagens.txt | cut -d ":" -f1 | sort -rn | head -1)
                echo "$((ID+1)):$1:$2:$3" >> portagens.txt
                ./success 3 $1
        fi
    
fi

cat portagens.txt | sort -t ':' -k3,3 -k2,2 | ./success 4 