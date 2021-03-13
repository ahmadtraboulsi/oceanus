#compile wsprsim.c as follow 

gcc wsprsim.c wsprsim_utils.c fano.c tab.c wsprd_utils.c nhash.c -o wsprsm

wsprsim -cds -28 -o 150613_1920.c2 "K1ABC FN42 33"

wsprsim -cds 50 -o  30_test.c2 "VE3EMB FN25 30"

building DLL
gcc -c -m64 wsprsim.c wsprsim_utils.c fano.c tab.c wsprd_utils.c nhash.c
gcc -shared -m64 -o wsprsim.dll *.o




Channel Symbols for VE3EMB FN25 30

my code



original code
 Message: VE3EMB FN25 30
 Data is :D4 2C 73 EB 3A 77 80 00 00 00 00
Channel symbols:
1 1 0 0 0 0 0 0 1 0 0 0 1 1 1 0 0 0 1 0 0 1 0 1 1 1 1 0 0 0 0 0 0 0 1 0 0 1 0 1 0 0 0 0 0 0 1 0 1 1 0 0 1 1 0 1 0 0 0 1 1 0 1 0 0 0 0 1 1 0 1 0 1 0 1 0 1 0 0 1 0 0 1 0 1 1 0 0 0 1 1 0 1 0 1 0 0 0 1 0 0 0 0 0 1 0 0 1 0 0 1 1 1 0 1 1 0 0 1 1 0 1 0 0 0 1 1 1 0 0 0 0 0 1 0 1 0 0 1 1 0 0 0 0 0 0 0 1 1 0 1 0 1 1 0 0 0 1 1 0 0 0
Writing 30_test.c2


IN matlab

loadlibrary('wsprsim')
libfunctions wsprsim -full
 p = libpointer('doublePtr', zeros(1,45000));  % initialize array of 10 elements
get(p)








