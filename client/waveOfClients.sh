#!/bin/sh

max=0
touch file
while [[ $max -le $1 ]]
do

	./client files/ozoi$max 9002 192.168.34.223< creds&
	#./client files/ozoi$max 9002 127.0.0.1< creds&
	ssleep 0.5
	max=$((max +1))
	echo "Cliente spawnado"
done

echo "terminei!!!!";
