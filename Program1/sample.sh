#!/bin/bash
#Mariessa Logan
#CS344
#4/22/19

dims(){
	
	row=0
	col=0
	while read nextline
	do
		#Read through the lines and increment row for each line read
		row=`expr $row + 1`
		if [ $col == 0 ]
		then
			for cell in $nextline
			do
				#This should only run once, then will increment for each "cell" in row
				col=`expr $col + 1`
			done
		fi
	done <"${1:-/dev/stdin}" #Solution on stack overflow to work with files and to work with stdin
	echo "$row $col"
	
}

add(){
	#first check the dimensions of the matrices
	tempDims="TempDims1File"
	tempDims2="TempDims2File"
	dims $1 > "$tempDims"
	dims $2 > "$tempDims2"
	if  $( cmp  "$tempDims2" "$tempDims") 
	then
	 	#dimensions are equal
	 	tempAdd="tempAddFile"
	 	sum=0
	 	num2=0
	 	cols=0
	 	#get the number of columns
	 	cols=$( dims $1 | cut -d ' ' -f 2 )
	 		 		 	
	 	#Read files and input sums into temp add file
	 	while IFS= read -r lineA && IFS= read -r lineB <&3
	 	do
	 		 i=1
	 		 while (( "$i" <= "$cols" ))
	 		 do
	 			 sum=$( echo "$lineA" | cut -f "$i" )
	 			 num2=$( echo "$lineB" | cut -f "$i" )
	 			 sum=$((sum+num2))
	 			 i=$((i+1))
	 			 if (( "$i" != $((cols + 1)) ))
	 			 then
	 			 	printf '%s\t' "$sum" >> "$tempAdd"
	 			 else
	 			 	printf '%s\n' "$sum" >> "$tempAdd"
	 			 fi 
	 		done
		done <"$1" 3<"$2"
		

		cat "$tempAdd"
	 	rm $tempDims
		rm $tempDims2
		rm $tempAdd
		exit 0
	 else
	 	#dimensions are not equal
		#change output to standard error
		echo "Matrices have different dimensions!" >&2
		rm $tempDims
		rm $tempDims2
		exit 1
	fi
	
}

#transpose function
transpose(){
	
	tempCol="TempColFile"
	tempTrans="tempTransFile"
	cols=$( dims $1 | cut -d ' ' -f 2 )
	i=1
	while (( "$i" <= "$cols" ))
	do
		#cut the coloumns
		cut -f "$i" "$1" > "$tempCol"
		
		tr -s '\n' '\t' < "$tempCol" >> "$tempTrans"
		#I tried to use the hint, but it kept deleting the whole file? This gets rid of the extratab
		truncate -s-1 "$tempTrans"
		printf "\n" >> "$tempTrans"
		i=$((i+1))
	done

	cat "$tempTrans"
	rm $tempCol
	rm $tempTrans
	exit 0
}

#Start of Mean function
mean(){
	tempTrans="tempTransFile"
	tempMean="tempMeanFile"
	cols=$( dims $1 | cut -d ' ' -f 2 )
	
	i=1
	mean=0
	num=0
	remain=0
	while (( "$i" <= "$cols" ))
	do
		sum=0
		j=0
		#cut the coloumns
		cut -f "$i" "$1" > "$tempTrans"
		#read that column into the sum and increment
		while read nextline
		do
			j=$((j+1))
			sum=$((sum+nextline))
		done <"$tempTrans"
		#check for rounding up, first if statement is if the sum is above zero
		remain=$(( sum % j ))
		mean=$((sum / j))
		#check if there is remainder that needs to round away from zero
		#After running tests to see what remainders I got on a variety of matrices 20 and -20 were my outputs for 
		#remainders that should be 0.5 or -0.5. 
		if (("$remain" <= -20 ))
		then
			mean=$((mean-1))
		fi
		if (("$remain">=20))
		then
			mean=$((mean+1))
		fi
		printf "%d\t" ${mean} >> "$tempMean"
		#increment i
		i=$((i+1))

	done
	#get rid of last tab
	truncate -s-1 "$tempMean"
	printf "\n" >> "$tempMean"
	cat "$tempMean"
	rm $tempMean
	rm $tempTrans
}

#multiply function
multiply(){
	tempDims="TempDims1File"
	tempDims2="TempDims2File"
	tempMatrix="tempMatrixFile"
	tempMatrix2="tempMatrix2File"
	tempProd="tempProdFile"
	tempField2="tempField2File"
	tempMatrix="$1"
	tempMatrix2="$2"
	#get dims of both matrices
	dims $1 > "$tempDims"
	dims $2 > "$tempDims2"
	#cut the dims into easily used variables
	row1=$( cat "$tempDims" | cut -d ' ' -f 1 )
	row2=$( cat "$tempDims2" | cut -d ' ' -f 1 )
	col1=$( cat "$tempDims" | cut -d ' ' -f 2 )
	col2=$( cat "$tempDims2" | cut -d ' ' -f 2 )
	i=1
	product=0
	array=()
	#check if you can multiply the two
	if [ "$row1" == "$col2" -a "$row2" == "$col1" ]
	then
		#it passed the test so start multiplying
		while IFS= read -r lineA && IFS= read -r lineB <&3
	 	do
	 		 i=1
	 		 #for each field in the line, multiply with the cell in lineb.  This should look like:
	 		 # --    --        --  --
             # | A  B  |      | E  F |
	 		 # | C  D  |   x  | G  H |  = (AE), (AF), (BE), (BF), (CG), (CH), (DG), (DH)
 	 		 # --     --       --  -- 
 	 		 #And as each i iterates, you add the appropriate products together to get the matrix

	 		 for cell in $lineA
	 		 do
	 		 	for cell2 in $lineB
	 		 	do
	 		 		product=$((cell * cell2 ))
	 		 		sum=$((product + array[$i]))
	 		 		array["$i"]="${array["$i"]}" "$sum"

	 		 	done
	 		 done
	 		 i=$(( i + 1 ))
		done <"$tempMatrix" 3<"$tempMatrix2"

		for (( j = 1; j < "$col2"; j++ )); do
			#print the array to the tempFile

			printf "%s" "${array[$i]}" 
			if ! (("$j" % "$col2")); then
				#add a tab or newline
				printf "\n" >>"$tempProd"
			else
				printf "\t" >> "$tempProd"
			fi
		done
		cat "$tempProd"
	else
		#matrices don't match, error sent to stderr
		echo "matrices don't match" 1>&2
		exit 1 
	fi

	
	rm $tempProd

	rm $tempDims
	rm $tempDims2
}
#Call the dims function
if [ "$1" == "dims" ]
then

	#if there are multiple files
	if (("$#" > 2))
	then
		#1>&2 will throw the output to std error
		echo "Too many files" 1>&2
		exit 1
	fi
	
	dims $2
fi
#call the add function
if [ "$1" == "add" ]
then
	if (("$#" < 2 ))
	then
		echo "Not enough files" 1>&2 
		exit 1
	fi
	if (("$#" > 3 ))
	then
		echo "Too many files" 1>&2
		exit 1
	fi

	add $2 $3
fi
#call the mean function
if [ "$1" == "mean" ]
then
	if (("$#" < 2 ))
	then
		echo "Not enough files" 1>&2
		exit 1
	fi
	if (("$#" > 3 ))
	then
		echo "Too many files" 1>&2
		exit 1
	fi

	mean $2 $3
fi
#transpose command
if [ "$1" == "transpose" ]
then
	#if there are multiple files
	if (("$#" > 2))
	then
		#1>&2 will throw the output to std error
		echo "Too many files" 1>&2
		exit 1
	fi
	#check if file unreadable
	if  [ ! -r "$2" ]
	then
		echo "file is unreadable" 1>&2
		exit 1
		
	fi
	transpose $2
	
fi
#multiply command
if [ "$1" == "multiply" ]
then
	multiply $2 $3

fi
#bad command
if [ "$1" != "dims" -a "$1" != "mean" -a "$1" != "transpose" -a "$1" != "add" -a "$1" != "multiply" ]
then
	echo "Error, did you mean dims, mean, transpose, add or multiply?" 1>&2
	exit 1
fi
exit 0