/* Memory Simulation Program
 * memSim.c
 *
 * Developed by Austen Rozanski
 * Last modified 12/05/2017
 *
 * View included readme for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// Used for tokenizing each line of input from input file
#define SEPARATORS " \t\n"

// Maximum size of the name of the processes
int MAX_PROCESS_NAME_SIZE = 16;

// Size of the buffers used
int BUFFER_SIZE = 1024;

int main (int argc, char ** argv) {

	//////////////////////////////////////////////////
	// VARIABLES
	//////////////////////////////////////////////////

	// each line of input will be seperated by spaces and placed into the
	// arg array
	char * args[BUFFER_SIZE];
	char ** arg;

	// pointer to the file to open
	FILE * fp;

	// size of memory
	unsigned long sizeOfMemory;

	// used with next fit algorithm.
	// Stores the index of the last process inserted.
	int indexLastInserted = -1;

	// names of the processes in order
	char memory[MAX_PROCESS_NAME_SIZE][BUFFER_SIZE];

	// position of processes in memory and size of processes
	// processes[process #][0] = position of process in memory
	// processes[process #][1] = size of process
	unsigned long processes[BUFFER_SIZE][2];

	// the current number of processes stored in memory
	int numProcesses = 0;

	// Algorithm type cooresponds to the type of algorithm to use
	// 0 => First Fit
	// 1 => Best Fit
	// 2 => Next Fit
	// 3 => Buddy System
	int algorithmType = 0;

	// name of the file to read commands from
	char fileName [BUFFER_SIZE];

	//////////////////////////////////////////////
	// Preparation for reading input
	//////////////////////////////////////////////

	// List of cmd line arguments:
	// argv[0] = executable
	// argv[1] = algorithm type
	// argv[2] = size of memory
	// argv[3] = input text file

	// Check to make sure the number of parameters equals 4
	if (argc <  4) {
		printf("To few cmd line parameters\n");
		return 0;
	}
	else if (argc >= 5) {
		printf("To many cmd line parameters\n");
		return 0;
	}


	// set the algorithmType variable based on value of argv[1]
	if (strcmp(argv[1], "FIRSTFIT") == 0) {
	}
	else if (strcmp(argv[1], "BESTFIT") == 0) {
		algorithmType = 1;
	}
	else if (strcmp(argv[1], "NEXTFIT") == 0) {
		algorithmType = 2;
	}
	else if (strcmp(argv[1], "BUDDY") == 0) {
		algorithmType = 3;
	}
	else {
		printf("ALGORITHM TYPE NOT VALID.\n");
		return 0;
	}

	// set the size of memory based on value of argv[2]
	if (sscanf(argv[2], "%ld", &sizeOfMemory) != 1) {
		printf("Error setting size of memory\n");
		return 1;
	}
	sizeOfMemory *= 8; // convert to bits

	// set the file name based on value of argv[3]
	strcpy(fileName, argv[3]);

	// open the file
	fp = fopen(fileName, "r");

	// Check if file failed to open
	if (fp == NULL) {
		printf("Error opening file\n");
		return 1;
	}

	//////////////////////////////////////////
	// Read from file until EOF reached
	//////////////////////////////////////////

	while (!feof(fp)) {

		// create a string to store the next line
		char line[BUFFER_SIZE];

		// set line equal to the next line of input from file fp
		if (fgets(line, BUFFER_SIZE, fp)) {

			// tokenize input into arg array.
			arg = args;
			*arg++ = strtok(line, SEPARATORS);
			while ((*arg++ = strtok(NULL, SEPARATORS)));
			// last entry will be NULL

			// Determine the command and set the variable below
			// according to the key below
			// 0 => REQUEST
			// 1 => RELEASE
			// 2 => LIST AVAILABLE
			// 3 => LIST ASSIGNED
			// 4 => FIND
			// 5 => # (Comment)
			// -1 => error with line
			int command = -1;

			// assign the command number using the args from the current
			// line in the file.
			if (args[0]) {
				if (strcmp(args[0], "REQUEST") == 0) {
					command = 0;
				}
				else if (strcmp(args[0], "RELEASE") == 0) {
					command = 1;
				}
				else if (strcmp(args[0], "LIST") == 0) {
					if (args[1]) {
						if (strcmp(args[1], "AVAILABLE") == 0) {
							command = 2;
						}
						else if (strcmp(args[1], "ASSIGNED") == 0) {
							command = 3;
						}
					}
				}
				else if (strcmp(args[0], "FIND") == 0) {
					command = 4;
				}
				else if (strcmp(args[0], "#") == 0) {
					command = 5;
				}
			}


			//*********************************************************//
			/////////////////////////////////////////////////////////////
			// COMMANDS
			/////////////////////////////////////////////////////////////
			//*********************************************************//


			/////////////////////////////////////////////////////////////
			// REQUEST COMMAND
			// args[0] = "REQUEST"
			// args[1] = processName
			// args[2] = processSize
			//
			// Insert the process in args[1] into the memory arrays using
			// the algorithm specified.
			/////////////////////////////////////////////////////////////
			if (command == 0) {

				// variable to store the process' name
				char processName[MAX_PROCESS_NAME_SIZE];

				// variable to store the process' size
				unsigned long processSize = 0;

				if (args[1] && args[2]) {

					// assign processName variable
					strcpy(processName, args[1]);

					// assign processSize variable and ensure
					// that no error occured
					if ( sscanf(args[2], "%lu", &processSize) == 1) {

						processSize *= 8; // convert to bits

						// output command being executed
						//printf("Request process %s of size %ld bytes\n", processName, processSize/8);

						
						//////////////////////////////////////////////////
						// FIRST FIT ALGORITHM
						//
						// Store the variable in the first available slot
						// in memory that is large enough to store it.
						/////////////////////////////////////////////////
						if (algorithmType == 0) {

							// used in loop to point to the next spot in
							// memory after the previous process
							unsigned long curLoc = 0;

							// toggle to 1 if process set during loop below
							int processAssigned = 0;

							// loop through all processes until space is found
							// between them for the new process
							for (int i = 0; i < numProcesses; i++) {

								// if there is space for the new process...
								if (processes[i][0] - curLoc  >= processSize) {
									
									// make space in the array by moving all
									// elements up one in the array
									for (int j = numProcesses; j > i; j--) {
										strcpy(memory[j], memory[j-1]);
										processes[j][0] = processes[j-1][0];
										processes[j][1] = processes[j-1][1];
									}

									// -Insert process at index i in the memory
									// array and processes array.
									// -Store the variable at spot curLoc in
									// memory..
									strcpy(memory[i], processName);
									processes[i][0] = curLoc;
									processes[i][1] = processSize;

									// increment number of processes and
									// toggle processAssigned to 1 (true)
									numProcesses++;
									processAssigned = 1;

									// output success
									printf("ALLOCATED %s %lu\n", processName, curLoc);

									break;
								}

								// set curLoc to the next spot in memory after the
								// current process (process[i])
								curLoc = processes[i][0] + processes[i][1];
							}

							// check the end of the memory if process was not assigned
							// in loop above
							if (processAssigned == 0) {
								// if there is room for the process, insert it
								// at the end of the arrays
								if (sizeOfMemory - curLoc >= processSize) {

									strcpy(memory[numProcesses], processName);
									processes[numProcesses][0] = curLoc;
									processes[numProcesses][1] = processSize;

									numProcesses++;

									// output success
									printf("ALLOCATED %s %lu\n", processName, curLoc);
								}
								else {
									printf("FAIL REQUEST %s %lu bytes\n", processName, processSize/8);
								}
							}
						}// end of next fit


						////////////////////////////////////////////////////////
						// BEST FIT ALGORITHM
						//
						// Store the process in the smallest gap in memory that
						// can still fit the entire process
						////////////////////////////////////////////////////////
						else if (algorithmType == 1) {
							
							// used in loop to point to the next spot in
							// memory after the previous process
							unsigned long curLoc = 0;

							// Store the value of curLoc when it was at the
							// best index up to that point in the loop
							unsigned long bestLoc = 0;//-1;

							// the starting space in memory when the index
							// was at its best up the that point in the loop
							unsigned long bestLocMem = 0;

							// Used to store the optimal index to place the process
							// at that point in the loop
							long bestIndex = -1;

							// test space before and between processes
							for (int i = 0; i < numProcesses; i++) {
								// if there is space for the process...
								if (processes[i][0] - curLoc >= processSize) {
									// if the amount of space is less than the previous best
									// place in memory to place the process, update bestLoc,
									// bestLocMem, and bestIndex to represent this
									if (bestIndex == -1 || processes[i][0] - curLoc < bestLocMem) {
										bestLoc = curLoc;
										bestLocMem = processes[i][0] - curLoc;
										bestIndex = i;
									}
								}
								// increment curLoc to the end of the current process
								curLoc = processes[i][0] + processes[i][1];
							}

							// test space after processes
							if (sizeOfMemory - curLoc >= processSize) {
								if (bestIndex == -1 || sizeOfMemory - curLoc < bestLocMem) {
									bestLoc = curLoc;
									bestLocMem = sizeOfMemory - curLoc;
									bestIndex = numProcesses;
								}
							}

							// insert the process if space was found
							if (bestIndex != -1) {

								// make space for the new process by incrementing all
								// values after it in the array by 1 index
								for (int i = numProcesses; i > bestIndex; i--) {
									strcpy(memory[i], memory[i-1]);
									processes[i][0] = processes[i-1][0];
									processes[i][1] = processes[i-1][1];
								}

								// copy the new process into the arrays
								strcpy(memory[bestIndex], processName);
								processes[bestIndex][0] = bestLoc;
								processes[bestIndex][1] = processSize;

								// increment the number of processes
								numProcesses++;

								// Output success
								printf("ALLOCATED %s %lu\n", processName, bestLoc);
							}
							else {
								printf("FAIL REQUEST %s %lu bytes\n", processName, processSize/8);
							}

						}// end of best fit


						/////////////////////////////////////////////////////
						// NEXT FIT ALGORITHMt
						//
						// Similar to the first fit algorithm except the 
						// loop starts looking after the last process placed
						/////////////////////////////////////////////////////
						else if (algorithmType == 2) {

							// if there are currently no processes, insert the process
							// at the beginning of the memory arrays if there is enough space.
							if (numProcesses == 0) {
								if (sizeOfMemory >= processSize) {
									strcpy(memory[0], processName);
									processes[0][0] = 0;
									processes[0][1] = processSize;
									numProcesses++;
									indexLastInserted = 0;

									printf("ALLOCATED %s %lu\n", processName, 0);
								}
								else {
									printf("FAIL REQUEST %s %lu bytes\n", processName, processSize/8);
								}
							}

							// if there are currently processes in memory...
							else {

								// set curLoc to the location of the end of the last
								// process inserted
								unsigned long curLoc = processes[indexLastInserted][0] + processes[indexLastInserted][1];

								// toggle to 1 if process assigned
								int processAssigned = 0;

								/////////////////////////////////////////////////
								// loop from last index inserted to end of memory
								/////////////////////////////////////////////////
								for (int i = indexLastInserted+1; i < numProcesses; i++) {

									// if there is room for process...
									if (processes[i][0] - curLoc >= processSize) {

										// make room in array for new process by
										// incrementing all indexes of other
										// processes after new process by 1
										for (int j = numProcesses; j > i; j--) {
											strcpy(memory[j], memory[j-1]);
											processes[j][0] = processes[j-1][0];
											processes[j][1] = processes[j-1][1];
										}

										// insert the new process in memory arrays
										strcpy(memory[i], processName);
										processes[i][0] = curLoc;
										processes[i][1] = processSize;

										// increment number of processes
										numProcesses++;

										// toggle processAssigned to 1
										processAssigned = 1;

										// update the value of the last inserted index
										indexLastInserted = i;

										// output success
										printf("ALLOCATED %s %lu\n", processName, curLoc);

										break;
									}

									// Update the curLoc variable
									curLoc = processes[i][0] + processes[i][1];

								} // end of loop from last index to end of memory loop


								///////////////////////////////
								// Check space after processes
								///////////////////////////////
								if (processAssigned == 0) {
									// if there is room for the process, insert it
									// at the end of the arrays
									if (sizeOfMemory - curLoc >= processSize) {

										strcpy(memory[numProcesses], processName);
										processes[numProcesses][0] = curLoc;
										processes[numProcesses][1] = processSize;

										// update the value of the last inserted index
										indexLastInserted = numProcesses;

										// increment the number of processes
										numProcesses++;

										// toggle the processAssigned to 1
										processAssigned = 1;

										// output success
										printf("ALLOCATED %s %lu\n", processName, curLoc);
									}
								}

								///////////////////////////////////////////////////////////////
								// Check from beginning of memory to the index last inserted at
								///////////////////////////////////////////////////////////////
								if (processAssigned == 0) {

									// store value of the end of spot in memory of last
									// process checked
									curLoc = 0;

									// loop from beginning of memory to indexLastInserted
									for (int i = 0; i <= indexLastInserted; i++) {

										// if there is space for the process...
										if (processes[i][0] - curLoc >= processSize) {

											// make space for the new process
											for (int j = numProcesses; j > i; j--) {
												strcpy(memory[j], memory[j-1]);
												processes[j][0] = processes[j-1][0];
												processes[j][1] = processes[j-1][1];
											}

											// Update the curLoc to the end of the current process
											//curLoc = processes[i][0] + processes[i][1];

											// insert the new process
											strcpy(memory[i], processName);
											processes[i][0] = curLoc;
											processes[i][1] = processSize;

											// increment number of processes
											numProcesses++;

											// update the index last inserted
											indexLastInserted = i;

											// output success
											printf("ALLOCATED %s %lu\n", processName, curLoc);

											processAssigned = 1;
											break;
										}

										// Update the curLoc value to the end of the current process
										curLoc = processes[i][0] + processes[i][1];
									}
								}

								if (processAssigned == 0) {
									printf("FAIL REQUEST %s %lu bytes\n", processName, processSize/8);
								}
							}
						}// end of NEXT FIT


						/////////////////////////////////////////////////
						// BUDDY SYSTEM ALGORITHM
						//
						// Partition memory into halves until just large
						// enough to store process to be inserted. Read
						// the readme for more in-depth description.
						/////////////////////////////////////////////////
						else if (algorithmType == 3) {
							// Calculate the smallest possible block of memory
							// that will still fit the process. Blocks must be
							// of the form blockSize = sizeOfMemory / 2^n where
							// n is a real number.
							unsigned long blockSize = sizeOfMemory;
							while (blockSize >= processSize) {
								blockSize /= 2;
							}
							blockSize *= 2;

							// the total number of blocks that could fit in memory
							unsigned long numBlocks = sizeOfMemory / blockSize;

							// the index of the current block being tested. The memory is
							// being treated like it is divided into blocks of size blockSize
							// and all these blocks are treated like an array.
							unsigned long curBlockIndex = 0;

							// index to insert the process into the memory arrays
							int processIndex = -1;

							// check before processes and between processes
							for (int i = 0; i < numProcesses; i++) {

								// if there is room for a 'block' of size blockSize before
								// process i, set processIndex to i to have the new process
								// inserted at this index
								if (processes[i][0] >= ((curBlockIndex+1) * blockSize)) {
									processIndex = i;
									break;
								}
								else {
									// find the index of the next block available after process i
									while (curBlockIndex * blockSize < processes[i][0] + processes[i][1]) {
										curBlockIndex++;
									}
								}
							}

							// check after processes
							if (curBlockIndex < numBlocks && processIndex == -1) {
								processIndex = numProcesses;
							}


							// if the processIndex variable was set above, insert the process
							// at that index
							if (processIndex != -1) {
								// Make space for the new process
								for (int i = numProcesses; i > processIndex; i--) {
									processes[i][0] = processes[i-1][0];
									processes[i][1] = processes[i-1][1];
									strcpy(memory[i], memory[i-1]);
								}

								// place process at position curBlockIndex * blockSize
								// and index processIndex in the array
								strcpy(memory[processIndex], processName);
								processes[processIndex][0] = curBlockIndex * blockSize;
								processes[processIndex][1] = processSize;

								// increment the number of processes
								numProcesses++;

								// Output success
								printf("Allocated %s %lu\n", processName, curBlockIndex * blockSize);
							}
							else {
								printf("FAIL REQUEST %s %lu bytes\n", processName, processSize/8);
							}
						}// end of buddy system

					}// end of if( sscanf(args[2], "%ld", &processSize) == 1) 

				}// end of if (args[1] && args[2]) {

			} // end of REQUEST command


			/////////////////////////////////////////////////////
			// RELEASE COMMAND
			// args[0] = "RELEASE"
			// args[1] = processName
			//
			// Removes process from args[1] from memory arrays
			/////////////////////////////////////////////////////
			else if (command == 1) {
				char processName[MAX_PROCESS_NAME_SIZE];

				// check if args[1] exists (will be obtained from args[1])
				if (args[1]) {
			
					// assign process name using args[1]
					strcpy(processName, args[1]);

					// Output command
					//printf("Release process %s\n", processName);

					// store the index in memory of the process
					// leave as -1 if not found
					int processIndex = -1;

					// loop through memory to find the process
					// and assign processIndex to index of the process
					for (int i = 0; i < numProcesses; i++) {
						if (strcmp(processName, memory[i]) == 0) {
							processIndex = i;
							break;
						}
					}

					// release the process if found
					if (processIndex != -1) {
						
						// FOR NEXT FIT ALGORITHM:
						// Modify reference to the index of the last process inserted
						// for when next fit algorithm is used
						if (algorithmType == 2) {
							if (indexLastInserted > processIndex) {
								indexLastInserted--;
							}
							else if (indexLastInserted == processIndex && indexLastInserted > 0) {
								indexLastInserted--;
							}
						}

						unsigned long processAddress = processes[processIndex][0];
						unsigned long processSize = processes[processIndex][1];

						// move elements in memory arrays down one index
						for (int i = processIndex+1; i < numProcesses; i++) {
							strcpy(memory[i-1], memory[i]);
							processes[i-1][0] = processes[i][0];
							processes[i-1][1] = processes[i][1];
						}

						// decrement number of processes
						numProcesses = numProcesses - 1;

						// output success
						printf("FREE %s %lu bytes\n", processName, processSize/8);
					}
					// Output if process not found
					else {
						printf("FAIL RELEASE %s\n", processName);
					}
				}


			}

			////////////////////////////////////////////////////////////////
			// LIST AVAILABLE COMMAND
			// args[0] = "LIST"
			// args[1] = "AVAILABLE"
			//
			// List the available portions of memory that are not currently
			// occupied by processes
			//
			// Output is in the following format:
			// (size of empty space, address of empty space) (...) ...
			////////////////////////////////////////////////////////////////
			else if (command == 2) {

				printf("Available space:\n");

				int isFull = 1;

				// store the end location of the last process
				// inspected in the loop below.
				unsigned long curLoc = 0;

				// Loop through all processes to look for space
				// between them.
				for (int i = 0; i < numProcesses; i++) {

					// if there is space between the last process
					// inspected and current process, output the
					// space.
					if (curLoc < processes[i][0]) {
						unsigned long emptySize = processes[i][0] - curLoc;
						printf("(%lu bytes, %lu) ", emptySize/8, curLoc);
						isFull = 0;
					}

					// update curLoc to equal the location at the
					// end of the process
					curLoc = processes[i][0] + processes[i][1];
				}

				//Output space after processes
				if (curLoc < sizeOfMemory) {
					unsigned long emptySize = sizeOfMemory - curLoc;
					printf("(%lu bytes, %lu)", emptySize/8, curLoc);
					isFull = 0;
				}

				if (isFull == 1) {
					printf("FULL");
				}

				printf("\n");
			}

			////////////////////////////////////////////////////////////
			// LIST ASSIGNED COMMAND
			// args[0] = "LIST"
			// args[1] = "ASSIGNED"
			//
			// Outputs all the processes currently stored in memory
			// along with the size and location of the process.
			//
			// Output is in the following format:
			// (Process name, process size, process address) (...) ...
			////////////////////////////////////////////////////////////
			else if (command == 3) {

				printf("Processes assigned in memory:\n");

				// Output (processName, size, address) of all
				// processes stored in memory
				for (int i = 0; i < numProcesses; i++) {
					printf("(%s, %lu bytes, %lu) ", memory[i], processes[i][1]/8, processes[i][0]);
				}

				if (numProcesses == 0) {
					printf("NONE");
				}

				printf("\n");
			}

			///////////////////////////////////////////////////////////
			// FIND COMMAND
			// args[0] = "FIND"
			// args[1] = process name
			//
			// Searches the memory for the first occurrence of a process
			// with the specified name. If found, will output the name,
			// size, and address of the process.
			//
			// Output is in the following format:
			// (Process name, process size, process address)
			///////////////////////////////////////////////////////////
			else if (command == 4) {

				// Name of the process (will be obtained from args[1]
				char processName[MAX_PROCESS_NAME_SIZE];

				//check if args[1] exists
				if (args[1]) {

					// assign processName from args[1]
					strcpy(processName, args[1]);

					// Output command
					printf("Finding process %s...\n");

					// store the index in memory of the process
					// leave as -1 if not found
					int processIndex = -1;

					// loop through memory to find the process
					// and assign processIndex to index of the process
					for (int i = 0; i < numProcesses; i++) {
						if (strcmp(processName, memory[i]) == 0) {
							processIndex = i;
							break;
						}
					}

					// is process found,
					// Output (processName, size, process address)
					if (processIndex != -1) {
						printf("(%s, %lu bytes, %lu)\n", processName,	processes[processIndex][1]/8, processes[processIndex][0]);

					}

					// Output if process not found
					else {
						printf("Process not found\n");
					}
				}
			}// end of FIND command

			///////////////////////////////////////////////////////////
			// ERROR
			//
			// If an error occurred while trying to decipher the line,
			// output to console. Usually this is caused by a spelling
			// error.
			///////////////////////////////////////////////////////////
			else if (command == -1) {
				printf("Error while reading line in file\n");
			}
		} // end of if (fgets(line, BUFFER_SIZE, fp)) {

		printf("\n");

	} // end of input (EOF reached)

	printf("End of file reached. Removing  processes...\n");

	//End all processes
	for (int i = 0; i < numProcesses; i++) {
		processes[i][0] = 0;
		processes[i][1] = 0;
		strcpy(memory[i], "");
	}
	numProcesses = 0;
	printf("Processes removed\n");

	// close the file
	fclose(fp);

	return 0;
}
