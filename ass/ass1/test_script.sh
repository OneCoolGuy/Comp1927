#!/bin/bash

>ptest.txt
>random.txt
y=a
randomstring=`NEW_UUID=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 32 | head -n 1)`
random=$(($RANDOM%100))

echo "$randomstring" | sed -e 's/\(.\)/\1\n/g' >random.txt

for k in {1..100}
do
	randomstring=`cat /dev/urandom | tr -dc 'a-z' | fold -w 32 | head -n 1`
	echo "$randomstring" | sed -e 's/\(.\)/\1\n/g' >random.txt
	# >youtput.txt
	>ptest.txt
	while read line
	do
		echo "+ $line $random" >> ptest.txt
		# echo "!" >> ptest.txt
		# echo "#" >> ptest.txt
		# y=$(echo "$y" | tr "a-y" "b-z")
		random=$(($RANDOM%100))
	done < random.txt



	shuf random.txt > srandom.txt
	

	while read line
	do
		echo "- $line" >> ptest.txt
		# echo "!" >> ptest.txt
		# echo "#" >> ptest.txt
		# y=$(echo "$y" | tr "b-z" "a-y")
	done < srandom.txt
	echo "!" >> ptest.txt
	echo "#" >> ptest.txt


	./vlad quiet <ptest.txt #>>youtput.txt

	# if tail -n 3 youtput.txt | egrep -q "size 4096" ; then
	# 	echo ptest.txt > "$ktest.txt"
	# 	echo youtput.txt >> "$ktest.txt"
	# fi
done

rm random.txt
rm srandom.txt
rm ptest.txt
