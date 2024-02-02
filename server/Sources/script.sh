#!/bin/bash

for i in $(ls *ht*.c)
do
cp $i /home/oar_X_I/cardDB/Sources
done

for i in $(ls ../Includes/*ht*.h)
do
cp $i /home/oar_X_I/cardDB/Includes
done


for i in $(ls d*comp*.c h*comp.c *cmd*.c hash* *iterator*.c)
do
cp $i /home/oar_X_I/cardDB/Sources
done

for i in $(ls ../Includes/d*comp*.h ../Includes/h*comp.h ../Includes/*cmd*.h ../Includes/hash*.h ../Includes/*iterator*.h)
do
cp $i /home/oar_X_I/cardDB/Includes
done





