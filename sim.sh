#!/bin/bash
echo "Entrando na pasta"
cd opt-ns-3.29/ns-3.29/

cont=0
tam=( 50 100 200 300 400 500 600)
len=${#tam[@]}

while [ $cont -lt $len ]
    do
        echo "Esperando 5 segundos para começar a simulação"
        sleep 5s
        echo
        echo "Começando simulação"
        ./waf --run "slice2 --trace=true --simTime=700 --hostsSlice1=${tam[$cont]} --hostsSlice2=${tam[$cont]}"
        echo "Simulação finalizada"
        echo
        cont=$(( cont+1 ))
done

