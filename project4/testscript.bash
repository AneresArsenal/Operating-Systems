
#use the standard version of echo
echo=/bin/echo

${echo} '#---------------------TESTSCRIPT STARTS--------------------#'
${echo}
${echo}
compileall
${echo} '-------------------PLAINTEXT1----------------------------'
${echo}
${echo}
keygen 50 > mykey
otp_enc_d 23456 &
otp_enc plaintext1 mykey 23456 > ciphertext1
${echo}
${echo}
otp_dec_d 67890 &
otp_dec ciphertext1 mykey 67890 > finaltext1
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
otp_enc_d 23456 &
otp_enc plaintext2 mykey 23456 > ciphertext2
${echo}
${echo}
otp_dec_d 67890 &
otp_dec ciphertext2 mykey 67890 > finaltext2
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
otp_enc_d 23456 &
otp_enc plaintext3 mykey 23456 > ciphertext3
${echo}
${echo}
otp_dec_d 67890 &
otp_dec ciphertext3 mykey 67890 > finaltext3
${echo}
${echo} '----------------------RESULTS-----------------------------'
${echo}
${echo} 'plaintext3 contains'
cat -A plaintext1
${echo} 'finaltext3 contains'
cat -A finaltext1
${echo}
wc < plaintext3
wc < finaltext3
${echo}
${echo} '-------------------PLAINTEXT5----------------------------'
${echo}
${echo}
otp_enc_d 23456 &
otp_enc plaintext5 mykey 23456 > ciphertext5
${echo}
${echo}
otp_dec_d 67890 &
otp_dec ciphertext5 mykey 67890 > finaltext5
${echo}
${echo} '----------------------RESULTS-----------------------------'
${echo}
${echo} 'plaintext5 contains'
cat -A plaintext5
${echo} 'finaltext5 contains'
cat -A finaltext5
${echo}
wc < plaintext5
wc < finaltext5
${echo}
${echo} '#---------------------SCRIPT COMPLETE--------------------#'
