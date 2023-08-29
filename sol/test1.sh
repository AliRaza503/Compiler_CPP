#!/bin/bash

let count=0;
for f in $(ls ./Project3_Tests/tests/*.txt); do 
	./a.out <$f > ./Project3_Tests/tests/`basename $f .txt`.output; 
done;

for f in $(ls ./Project3_Tests/tests/*.output); do
	diff -Bw $f  ./Project3_Tests/tests/`basename $f .output`.txt.expected > ./Project3_Tests/tests/`basename $f .output`.diff;
done

for f in $(ls Project3_Tests/tests/*.diff); do
	echo "========================================================";
	echo "FILE:" `basename $f .output`;
	echo "========================================================";
	if [ -s $f ]; then
		cat ./Project3_Tests/tests/`basename $f .diff`.txt;
		echo "--------------------------------------------------------";
		cat $f
	else
		count=$((count+1));
		echo "NO ERRORS HERE!";
	fi
done

echo $count;

rm Project3_Tests/tests/*.output
rm Project3_Tests/tests/*.diff

