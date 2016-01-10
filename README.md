#Count no. of prime numbers
CountingPrime.cc is a simple program that takes in an integer n, and computes the number of prime numbers in [1...n] using multi-threading. The program also prints out the time taken for the computation.

##Compilation
```bash
g++ CountingPrime.cc -o CountingPrime -pthread
```

##Running the program
```bash
./CountingPrime n
```
where n is the upper bound
