################################################
# Name: Alexander Tir
# ONID: tira
# Assignment: Program Py - Python Exploration
# Sources: 
# https://stackoverflow.com/questions/2257441/random-string-generation-with-upper-case-letters-and-digits/2257449
# https://stackoverflow.com/questions/3996904/generate-random-integers-between-0-and-9
################################################

import string
import random

stringLength = 10

# function used to generate and return a string of 10 random lowercase letters
def randomLowerCase(length):
	randomLetters = ""
	for i in range(length):
		#append each random character to randomLetters
		randomLetters = randomLetters + (random.choice(string.ascii_lowercase))
	return randomLetters

# 3 pre-determine file names to use
names = ["myfile1", "myfile2", "myfile3"]

# x is the iterator, holds the string
for x in names:
	fileName = x
	# open and write random string to file
	with open(fileName, 'w') as file:
		file.write(randomLowerCase(stringLength) + "\n")
		file.close()

# print the files to stdout
for x in names:
	fileName = x
	# open in read mode
	with open(fileName, 'r') as file:
		print(file.read())
		file.close()

# generate random integer in range 1 to 42 inclusive
intOne = random.randint(1, 42)
intTwo = random.randint(1, 42)

# store product of the two random ints
product = intOne * intTwo

# print the two random integers and product on separate lines
print(intOne)
print(intTwo)
print(product)