seq1=`cat input.txt | sed -n '2 p'`
seq2=`cat input.txt | sed -n '3 p'`
len1=${#seq1}
len2=${#seq2}
np_var=`expr $len1 - $len2 + 2`
echo $np_var