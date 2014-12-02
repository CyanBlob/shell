num=5

if [ $num -lt 7 ]
 then
  echo Yahoo, 5 actually IS less than 7
 else
  echo Boo, and all this time I thought that 5 is less than 7
fi

repeatIndex1=0

while [ $repeatIndex1 -lt $num ]

do

 echo "Yes, Virginia, there IS a Santa Claus!"

repeatIndex1=$[$repeatIndex1+1]

done
