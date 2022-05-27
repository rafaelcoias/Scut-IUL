#!/bin/bash 

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2021/2022
##
## Aluno: Nº: 105361      Nome: Rafael Cóias
## Nome do Módulo: menu.sh
## Descrição/Explicação do Módulo: 
##
##
###############################################################################

OPC=1 

while [ $OPC == "0" ] || [ $OPC == "1" ] || [ $OPC == "2" ] || [ $OPC == "3" ] || [ $OPC == "4" ]; do
    echo ""
    echo -e "===== \e[1;32m MENU \e[0m ====="
    echo ""
    echo "1. Listar Condutores"
    echo "2. Altera taxa de portagem"
    echo "3. Stats"
    echo "4. Faturação"
    echo "0. Sair"
    echo ""
    read -p "Opção: " OPC
    echo ""

    if [[ $OPC != "1" ]] && [[ $OPC != "2" ]] && [[ $OPC != "3" ]] && [[ $OPC != "4" ]] && [[ $OPC != "0" ]]; then
        ./error 3 $OPC
        OPC=1
        continue
    fi

    if [[ $OPC == "1" ]]; then
        ./lista_condutores.sh
        elif [[ $OPC == "2" ]]; then
            echo ""
            echo -e "===== \e[1;31m Altera taxa de portagem \e[0m ====="
            echo ""
            echo "Insira o Lanço:           "
            read LANCO
            echo "Insira a Auto-Estrada:    " 
            read AUTO
            echo "Insira o valor de Taxa:   "
            read TAXA
            ./altera_taxa_portagem.sh $LANCO $AUTO $TAXA
        elif [[ $OPC == "3" ]]; then
            OPC2=1
            while [ $OPC2 == "0" ] || [ $OPC2 == "1" ] || [ $OPC2 == "2" ] || [ $OPC2 == "3" ]; do
                echo ""
                echo -e "===== \e[1;31m Stats \e[0m ====="
                echo ""
                echo "1. Nome de todas as Auto-Estradas"
                echo "2. Registos de utilização"
                echo "3. Listagem de condutores"
                echo "0. Voltar"
                echo ""
                read -p "Opção: " OPC2
                echo ""
                if [[ $OPC2 != "1" ]] && [[ $OPC2 != "2" ]] && [[ $OPC2 != "3" ]] && [[ $OPC2 != "0" ]]; then
                    ./error 3 $OPC2
                    OPC2=1
                    break
                fi
                if [[ $OPC2 == "1" ]]; then
                    ./stats.sh "listar"
                    elif [[ $OPC2 == "2" ]]; then
                        echo "Minimo de registos    : "
                        read REG
                        echo ""
                        ./stats.sh "registos" $REG
                    elif [[ $OPC2 == "3" ]]; then
                        ./stats.sh "condutores"
                    else
                        break
                fi
            done
        elif [[ $OPC == "4" ]]; then
            ./faturacao.sh
        else [[ $OPC == "0" ]];
            ./success 1 && exit
    fi
done