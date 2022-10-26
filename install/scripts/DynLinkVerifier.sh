#!/bin/bash
if [ $# != 1 ]
then
   echo "usage: DynLinkVerifier.sh <input_File>";
   exit 1;
fi
missing=0;
for file in `cat $1`;
do
if [ ! -f $file ]
then
  echo "$file is missing";
  missing=1;
fi
done;
if [ "$missing" != "0" ]
then
   exit 1;
fi
exit 0;
