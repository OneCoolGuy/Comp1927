#!/bin/bash

>ptest.txt
>random.txt
randomstring=`NEW_UUID=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 32 | head -n 1)`
random=$(($RANDOM%1000))

echo "$randomstring" | sed -e 's/\(.\)/\1\n/g' >random.txt

for k in {1..100}
do
	randomstring=`cat /dev/urandom | tr -dc 'a-z' | fold -w 32 | head -n 1`
	echo "$randomstring" | sed -e 's/\(.\)/\1\n/g' >random.txt
	>ptest.txt
	while read line
	do
		echo "+ $line $random" >> ptest.txt
		# echo "!" >> ptest.txt
		# echo "#" >> ptest.txt
		random=$(($RANDOM%1000))
	done < random.txt



	shuf random.txt > srandom.txt
	

	while read line
	do
		echo "- $line" >> ptest.txt
		# echo "!" >> ptest.txt
		# echo "#" >> ptest.txt
	done < srandom.txt
	# echo "!" >> ptest.txt
	echo "#" >> ptest.txt


	./vlad quiet <ptest.txt 
done

rm random.txt
rm srandom.txt
rm ptest.txt
