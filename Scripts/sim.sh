#!/bin/bash
echo "Entrando na pasta"
cd opt-ns-3.29/ns-3.29/

cont=0
hs1=( 50 100 200 300 400 500 600) #Numero de hosts slice 1
hs2=( 50 100 200 300 400 500 600) #Numero de hosts slice 2
len=${#hs1[@]}
rng=$1

while [ $cont -lt $len ]
	do
		echo "Esperando 3 segundos para começar a simulação"
		sleep 3s
		echo
		echo "Começando simulação - RNG = $rng"
		./waf --run "slice2 --RngRun=$rng --trace=true --simTime=700 --hostsSlice1=${hs1[$cont]} --hostsSlice2=${hs2[$cont]}"
		echo "Simulação finalizada"
		echo
		cont=$(( cont+1 ))
done
