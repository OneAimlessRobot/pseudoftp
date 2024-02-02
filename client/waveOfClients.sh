#!/bin/sh
if [[ $# -ne 4 ]];
then
	echo "Argumentos:<# clientes> <path ficheiro de destino> <porta server> <ip server>. Foram dados $# argumentos";
	exit -1
fi
max=1
while [[ $max -le $1 ]]
do

	./client $2$max $3 $4< creds&
	sleep 0.5
	max=$((max +1))
	echo "Cliente spawnado"
done

echo "terminei!!!!";
