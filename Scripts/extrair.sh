#!/bin/bash

cont=0
hs2=( 33 66 133 200 266 333 400) #Numero de hosts slice 1
hs1=( 67 134 267 400 534 667 800) #Numero de hosts slice 2
len=${#hs1[@]}
rng=1

while [ $rng -lt 5 ]
	    do
		while [ $cont -lt $len ]
		    do
			echo "Esperando 1 segundo"
			sleep 1s
			echo
			echo "Lendo arquivo: flowmonitor${hs1[$cont]}_${hs2[$cont]}_rng$rng.xml"
			python scriptresultado.py Hosts-diferentes/flowmonitor${hs1[$cont]}_${hs2[$cont]}_rng$rng.xml ${hs1[$cont]} ${hs2[$cont]} $rng
			echo "Finalizado"
			echo
			cont=$(( cont+1 ))
		done
		cont=0
		rng=$(( rng+1 ))
done
