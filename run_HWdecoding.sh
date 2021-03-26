#!/bin/bash
FILE=MFTraw.cfg
touch $FILE
echo "[input-MFT-0]">>$FILE
echo "dataOrigin = MFT" >>$FILE
echo "dataDescription = RAWDATA" >>$FILE
echo "filePath = /local/home/mc262512/alice/data/"$1 >> $FILE
echo "Creation of MFTraw.cfg : Done! "
echo "-----------------------------"
echo "Run decoder RDHv6"
timestamp=$(date +"%Y-%m-%d_%H-%M-%S")
log=/local/home/mc262512/alice/output/log/decode_$timestamp.out
ROOTFILEPATH=/local/home/mc262512/alice/output/
#o2-raw-file-reader-workflow -b --configKeyValues 'HBFUtils.nHBFPerTF=128' --delay 0.0005 --detect-tf0 --nocheck-missing-stop --nocheck-starts-with-tf --nocheck-packet-increment --nocheck-hbf-jump --nocheck-hbf-per-tf --input-conf $FILE | o2-itsmft-stf-decoder-workflow -b --runmft --digits --writeHW --no-clusters --no-cluster-patterns --decoder-verbosity 1 | o2-itsmft-digit-writer-workflow -b --runmft --writeHW --disable-mc --outfile $ROOTFILEPATH$1.root > $log
o2-raw-file-reader-workflow -b --configKeyValues 'HBFUtils.nHBFPerTF=128' --delay 0.0005 --detect-tf0 --nocheck-missing-stop --nocheck-starts-with-tf --nocheck-packet-increment --nocheck-hbf-jump --nocheck-hbf-per-tf --input-conf $FILE | o2-itsmft-stf-decoder-workflow -b --runmft --enable-calib-data --digits --no-clusters --no-cluster-patterns --decoder-verbosity 1 | o2-itsmft-digit-writer-workflow -b --runmft --disable-mc --enable-calib-data --outfile $ROOTFILEPATH$1.root > $log
rm -f $FILE
echo "Error Jump in RDH_packetCounter :  " 
grep -r "Jump in RDH_packetCounter" $log | wc -l
echo "Done !"
