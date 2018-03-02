------------------------------------------------------------------------------------
Memory Simulation Program
developed by Austen Rozanski

last edited: 12/05/2017

-------------------------------------------------------------------------------------
DESCRIPTION:

The memory simulation program simulates the process of inserting processes into
memory using four different algorithms. It also simulates the removal of processes
from memory.

-------------------------------------------------------------------------------------
COMPILING AND RUNNING:

to compile the project, you can either use the command

gcc -o memorySim -g memSim.c

or you can use the commands from the make file. (See next section below for more
information).




To run the program, use the command:

./memorySim <algorithm type> <memory size> <input file>


for <algorithm type>, there are four possibilites:

BESTFIT
NEXTFIT
FIRSTFIT
BUDDY

each of these algoirthms is explained more in-depth below.


for <memory size>, you will enter the size of the simulated memory (in bytes). 
Because an unsigned long int is used, the maximum value for this field is
536,870,911.


for <input file>, list the path to a file that commands will be read from. For more
information see the section labeled INPUT FILE AND COMMANDS below.

------------------------------------------------------------------------------------
USING MAKEFILE:

To generate an executable file, run the command:

make

or run the command:

make all


To remove the previously executable, run the command:

make clean

------------------------------------------------------------------------------------
INPUT FILE AND COMMANDS

The input file is a list of commands seperated by new lines that will be read from
to manipulate the simulated memory. The list of commands are:

------------------------------------------------------------------------------------|
| command                          | description                                    |
------------------------------------------------------------------------------------|
| REQUEST processName processSize  | Allocates processSize bytes for a process named|
|                                  | processName. On success outputs ALLOCATED      |
|                                  | processName processSize. On failure, outputs   |
|                                  | FAIL REQUEST processName processSize           |
|                                  | Be sure to use values between 0 and 536870911  |
|                                  | for the processSize.
------------------------------------------------------------------------------------|
| RELEASE processName              | Deallocates memory belonging to process named  |
|                                  | processName. On success outputs FREE           |
|                                  | processName n x where n is the amount of memory|
|                                  | reclaimed and x is the address of the released |
|                                  | memory.                                        |
------------------------------------------------------------------------------------|
| LIST AVAILABLE                   | Outputs the location of all available memory.  |
|                                  | Format of output is (n1, x1) (n2, x2) ... where|
|                                  | n represents the amount of memory available and|
|                                  | x represents the starting location.            |
------------------------------------------------------------------------------------|
| LIST ASSIGNED                    | Outputs the size and location of all processes |
|                                  | in memory. Format of output is                 |
|                                  | (A1, n1, x1) (A2, n2, x2) ...                  |
|                                  | where A is the process name, n is the size of  |
|                                  | the process in bytes and x is the address.     |
------------------------------------------------------------------------------------|
| FIND processName                 | Searches for processName in memory and outputs |
|                                  | the size and address of the process if found.  |
|                                  | Format of the output is (A, n, x) where A is   |
|                                  | the name of the process, n is the size of A    |
|                                  | int bytes and x is the address of the process. |
------------------------------------------------------------------------------------|
| # (Comment)                      | Anything on the same line after the # sign will|
|                                  | be ignored.                                    |
------------------------------------------------------------------------------------|

Note: name of process can not be any more than 16 characters long.

------------------------------------------------------------------------------------
FIRST FIT ALGORITHM

The first fit algorithm will look through the memory and place a process in the first
spot in memory that is large enough to store it. It has a pretty simple implementation
and works faster than the best fit algorithm, but can be wasteful of space in the worst
cases.

------------------------------------------------------------------------------------
NEXT FIT ALGORITHM

Next fit is very similar to first fit, with the exception that instead of looking
through memory starting at the beginning, it begins at the index of the process
last inserted into memory. Also similar to the first fit algorithm, it has a pretty
straight forward implementation and works quickly, but can also be wasteful of space in
the worst cases.

------------------------------------------------------------------------------------
BEST FIT ALGORITHM

The best fit algorithm looks through every available spot in memory and chooses an
open spot that can fit the process that is closest in size to the process. Because
it looks through every spot in memory, in can run much slower than the other three
algorithms but it optimizes the amount of space used.

------------------------------------------------------------------------------------
BUDDY SYSTEM ALGORITHM

The buddy system is pretty different from the other three algorithms listed here.
For the buddy system, memory is divided by powers of two until it reaches the smallest
possible size that can still store the process that needs to be inserted. Then, the process
is inserted into a potion of memory of the size of that value obtained from dividing the
memory. The simplest way to visualize this process is to use a tree diagram, where each
level you go down the tree the memory gets divided in half. Processes can be placed at
the nodes in this tree and no process can be the child of another process (this includes
grand child, great grand child, etc.) 

-------------------------------------------------------------------------------------
IMPLEMENTATION

The memory is stored using two arrays. The first array, called memory, is used to
store the names of all the processes. It is a 2d char array which could be thought of
as a 1d array of strings. The processes in this array are ordered in there position in
memory in order for their indexes to match their respective values in the second array.

The second array (named processes) is a 2d unsigned long int array in the following format:

processes[i][n] where i = the index of the process and n can equal 0 or 1.

processes[i][0] => The position in memory of the process.
processes[i][1] => The size of the process in bits.


For the first fit, best fit, and next fit algorithms, the program simply checks the
memory before, between, and after the processes in a loop according to what is needed
for that algoithm and starting at the index required by the algorithm.

For the buddy system, the smallest size that still fits the process when dividing the
memory by a power of two is labeled as a 'block of memory'. The algorithm goes through
the memory and divides it into an even number of blocks that can be checked to see if
they contain any processes. Once a block is found that does not contain a process, the
current process we are trying to place in memory is put into that block.
