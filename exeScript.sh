#!/bin/bash
MODELJPG="/home/linh/Desktop/editedImages/md_images/size1/Md 054.JPG"
MODELTPS="/home/linh/Desktop/editedImages/md_landmarks/size1/Md 054.TPS"
SCENEJPGFOLDER="/home/linh/Desktop/editedImages/md_images/size1/*"
SCENETPSFOLDER="/home/linh/Desktop/editedImages/md_landmarks/size1/*"
#METHOD: 1 for template maching, 2 for SIFT
METHOD="1"
SAVEFOLDER="/home/linh/Desktop/results/2017/md/6mars_tm/"
FIRSTSIZE="100"
SECONDSIZE="300"
EXECUTE="./MAELab_CI"
#/home/linh/Desktop/rsImages/*
#/home/linh/Desktop/editedImages/md_images/size1/*
#TOTAL= ls -lR $SCENEJPGFOLDER | wc -l
#echo $TOTAL
#for i in $SCENEJPGFOLDER; do
#	echo item: $img
#done
#files=$( ls $SCENEJPGFOLDER )
#for ((i=0;i<${TOTAL};i++));
#do
#	echo ${SCENEJPGFOLDER[$i]}
#done
jpgarray=(${SCENEJPGFOLDER})
tpsarray=(${SCENETPSFOLDER})
total=${#jpgarray[@]}
for (( i=0; i< $total; i++))
do
	SCENEJPG="${jpgarray[$i]}"
	img=${SCENEJPG:${#imagepath}-10}
	SAVEJPG="$SAVEFOLDER$img"
	echo $SCENEJPG
	#echo "${jpgarray[$i]}"
	SCENETPS="${tpsarray[$i]}"	
	tps=${SCENETPS:${#tpspath}-10}
	SAVETPS="$SAVEFOLDER$tps"
	#echo ${tpsarray[$i]}
	$EXECUTE "$MODELJPG" "$MODELTPS" "$SCENEJPG" "$SCENETPS" "$METHOD" "$FIRSTSIZE" "$SECONDSIZE" "$SAVEJPG" "$SAVETPS"
done

#EXECUTE="./MAELab_CI"
#$EXECUTE

