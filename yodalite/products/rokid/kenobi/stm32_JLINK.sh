device=$1
file=$2
addr=$3

./JLink_Linux_V644f_x86_64/JLinkExe << EOF
connect
${device}
S

4000 kHz
r
loadbin ${file} ${addr} 
exit
EOF

exit 0;
