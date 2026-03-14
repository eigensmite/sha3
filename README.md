compile with `gcc ./sha3.c -o sha3.o -lgmp -lm`
run with `./sha.o <../path/to/file.txt>`
or `cat <../path/to/file.txt> | ./sha3.o`
or `echo -n "text string abc" | ./sha3.o`

You will have to have the math library and gnu multi precision library installed
