# MinimalCC
Minimal C subset compiler intended to eventually run on my calculator
However, it currently compiles to MIPS assembly

There is a folder called `tests`, which allows you to test the assembler. 

First, build the compiler by typing `make`. It should compile very quickly and produce an executable named `cc`. 

Then, run `cc tests/test.cc > tests/a.s` to produce an assembly file named `a.s` in `tests`. 

From SPIM, import all of the files now inside of the `tests` folder, and finally run the assembly program. 

You should be able to enter numbers, and the program will tell you if you are guessing too high or too low. 

To change the secret number, edit the `main.s` file, which has the role of an entry point into the `guessing_game` function in `test.cc`