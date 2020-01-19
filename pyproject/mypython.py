from random import choice, randint
from string import ascii_lowercase, digits

# program requirements
# When executed, create 3 files in the same directory as your script, 
# each named differently (the name of the files is up to you), 
# which remain there after your script finishes executing. 
# Each of these 3 files must contain exactly 10 random characters from the lowercase alphabet,
# with no spaces ("hoehdgwkdq", for example). The final (eleventh) character of each file MUST
# be a newline character. Additional runs of the script should not append to the files. 
# Thus, running wc (wordcount) against your files in the following manner must return 11


# filename variable
filename = "awesomefile"

# use for loop to create three files
for i in range(3):

	# create a random string of 10 lowercase characters
	randomStr=''.join(choice(ascii_lowercase) for j in range(10))
	
	# add mandatory newline
	randomStr=randomStr+"\n"

	# create file with iterator i as an id to satisfy the name requirement,
	# overwrite if file exist
	f = open((filename+str(i+1)), 'w')

	# write random string to file
	f.write(randomStr)
	f.close() 


# When executed, output sent to stdout should first print out the contents of the 
# 3 files it is creating in exactly the format given below.

# use for loop to read three files
for i in range(3):
	f = open((filename+str(i+1)), "r")

	# print each file, remove newline created by print function with end
	print(f.read(), end='')

# After the file contents of all three files have been printed, 
# print out two random integers each on a separate line (whose range is from 1 to 42, inclusive)

# two random integers that range from 1 to 42 inclusive
choice1 = randint(1,42)
choice2 = randint(1,42)


# on the last (sixth) line, print out the product of the two numbers.
multiple = choice1 * choice2
print(choice1)
print(choice2)
print(multiple)


# Reference sources:
# https://stackoverflow.com/questions/21632386/create-a-file-in-the-script-using-python
# https://stackoverflow.com/questions/18319101/whats-the-best-way-to-generate-random-strings-of-a-specific-length-in-python
# https://docs.python.org/2/library/random.html
# https://www.w3schools.com/python/python_file_open.asp
# https://stackoverflow.com/questions/493386/how-to-print-without-newline-or-space