#!usr/bin/bash

>RresultsA
>AresultsA
>DresultsA

>RresultsB
>AresultsB
>DresultsB

for i in `seq 10 18`;
do
	power=$i;
	num=$((2 << $power));
	echo "------------------ INPUT SIZE $num -----------" >> RresultsA
	echo "------------------ INPUT SIZE $num -----------" >> RresultsB
	for c in `seq 1 10`
	do
		 (./gen $num R | /usr/bin/time --format="%U" ./sortA > /dev/null ) &>> RresultsA
		 (./gen "2**$i" R | /usr/bin/time --format="%U" ./sortB > /dev/null )&>> RresultsB
	done
done



for $i in `seq 10 18`;
do
	power=$i;
	num=$((2 << $power));
	echo "------------------ INPUT SIZE $(2 ** $i) -----------" >> AresultsA
	echo "------------------ INPUT SIZE $(2 ** $i) -----------" >> AresultsB
	for $c in `seq 1 10`
	do
		(./gen "2**$i" A | /usr/bin/time --format="%U" ./sortA > /dev/null )&>> AresultsA
		(./gen "2**$i" A | /usr/bin/time --format="%U" ./sortB > /dev/null )&>> AresultsB
	done
done


for $i in `seq 10 18`;
do
	power=$i;
	num=$((2 << $power));
	echo "------------------ INPUT SIZE $(2 ** $i) -----------" >> DresultsA
	echo "------------------ INPUT SIZE $(2 ** $i) -----------" >> DresultsB
	for $c in `seq 1 10`
	do
		(./gen "2**$i" D | /usr/bin/time --format="%U" ./sortA > /dev/null )&>> DresultsA
		(./gen "2**$i" D | /usr/bin/time --format="%U" ./sortB > /dev/null )&>> DresultsB
	done
done
