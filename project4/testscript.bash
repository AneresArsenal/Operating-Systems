
#use the standard version of echo
echo=/bin/echo

encPort=$(( $RANDOM % 2000 + 1000 ))
decPort=$(( $RANDOM % 2000 + 5000 ))

${echo} '#---------------------TESTSCRIPT STARTS--------------------#'
${echo}
${echo}
compileall
${echo} '-------------------PLAINTEXT1----------------------------'
${echo}
${echo}
keygen 50 > mykey
otp_enc_d $encPort &
otp_enc plaintext1 mykey $encPort > ciphertext1
${echo}
${echo}
otp_dec_d $decPort &
otp_dec ciphertext1 mykey $decPort > finaltext1
${echo}
${echo} '----------------------RESULTS-----------------------------'
${echo}
${echo} 'plaintext1 contains'
cat -A plaintext1
${echo} 'finaltext1 contains'
cat -A finaltext1
${echo}
wc < plaintext1
wc < finaltext1
${echo}
${echo} '-------------------PLAINTEXT2----------------------------'
${echo}
${echo}
keygen 350 > mykey
otp_enc_d $encPort &
otp_enc plaintext2 mykey $encPort > ciphertext2
${echo}
${echo}
otp_dec_d $decPort &
otp_dec ciphertext2 mykey $decPort > finaltext2
${echo}
${echo} '----------------------RESULTS-----------------------------'
${echo}
${echo} 'plaintext2 contains'
cat -A plaintext2
${echo} 'finaltext2 contains'
cat -A finaltext2
${echo}
wc < plaintext2
wc < finaltext2
${echo}
${echo} '-------------------PLAINTEXT3----------------------------'
${echo}
${echo}
keygen 50 > mykey
otp_enc_d $encPort &
otp_enc plaintext3 mykey $encPort > ciphertext3
${echo}
${echo}
otp_dec_d $decPort &
otp_dec ciphertext3 mykey $decPort > finaltext3
${echo}
${echo} '----------------------RESULTS-----------------------------'
${echo}
${echo} 'plaintext3 contains'
cat -A plaintext3
${echo} 'finaltext3 contains'
cat -A finaltext3
${echo}
wc < plaintext3
wc < finaltext3
${echo}
${echo} '-------------------PLAINTEXT4----------------------------'
${echo}
${echo}
keygen 70000 > mykey
otp_enc_d $encPort &
otp_enc plaintext4 mykey $encPort > ciphertext4
${echo}
${echo}
otp_dec_d $decPort &
otp_dec ciphertext4 mykey $decPort > finaltext4
${echo}
${echo} '----------------------RESULTS-----------------------------'
${echo}
${echo} 'plaintext4 contains'
cat -A plaintext4
${echo} 'finaltext4 contains'
cat -A finaltext4
${echo}
wc < plaintext4
wc < finaltext4
${echo}
${echo} '-------------------PLAINTEXT5----------------------------'
${echo}
${echo}
otp_enc_d $encPort &
otp_enc plaintext5 mykey $encPort > ciphertext5
${echo}
${echo}
otp_dec_d $decPort &
otp_dec ciphertext5 mykey $decPort > finaltext5
${echo}
${echo} '----------------------RESULTS-----------------------------'
${echo}
# ${echo} 'plaintext5 contains'
# cat -A plaintext5
# ${echo} 'finaltext5 contains'
# cat -A finaltext5
${echo}
# wc < plaintext5
# wc < finaltext5
${echo}
${echo} '#---------------------SCRIPT COMPLETE--------------------#'
