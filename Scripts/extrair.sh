#!/bin/bash

cont=0
#hs1=( 33 66 133 200 266 333 400) #Numero de hosts slice 1
#hs2=( 67 134 267 400 534 667 800) #Numero de hosts slice 2
#hs1=( 50 100 200 300 400 500 600) #Numero de hosts slice 1
#hs2=( 50 100 200 300 400 500 600) #Numero de hosts slice 2
hs1=( 20 40 80 120 160 200 240) #Numero de hosts slice 1
hs2=( 80 160 320 480 640 800 960) #Numero de hosts slice 2
len=${#hs1[@]}
rng=1

while [ $rng -lt 5 ]
	    do
		while [ $cont -lt $len ]
		    do
			echo "Lendo arquivo: flowmonitor${hs1[$cont]}_${hs2[$cont]}_rng$rng.xml"
			python scriptresultado.py resultados-hosts-1-5/flowmonitor${hs1[$cont]}_${hs2[$cont]}_rng$rng.xml ${hs1[$cont]} ${hs2[$cont]} $rng
			echo "Finalizado"
			echo
			cont=$(( cont+1 ))
		done
		cont=0
		rng=$(( rng+1 ))
done
