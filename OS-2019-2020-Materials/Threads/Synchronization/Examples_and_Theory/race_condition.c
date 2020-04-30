#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int main() {
	// race condition appears when two threads are ran simultaneously with heavier work routines
	// race condition appears in summation example because the summation operation isn't atomic
	// i.e. it isn't one command and the thread continues to run
	// ex: sum += x;
	// register1 = sum;
	// register1 += x1 (1)
	// result keeps being 6 while it should be 7 since the two threads ought to work separately
	// atom instructions -> one processor instruction
	
	// race condition - situations wherein multiple threads manipulate shared data
	// critical section - a couple of processes race for the shared data
	// this critical section needs to have several attributes:
		// mutex (mutual exclusion) - no other process should have the right to go into the critical section
		// a process 
		// there shouldn't be a jam when a process goes into the critical section
		
		
	// entry into critical section
	// critical section
	// exit from critical section
	
	// Synchronization algorithms:
		// 1. using a global turn variable - one process waits and does nothing while turn is different from current process while other works in the critical section and then switches its value, breaking the process from the loop 
		// (we hope that turn=j for Processj is atom)
		// 2. Peterson algorithm - using a flag array and setting the element at the processor index to true (also adding a boolean condition)
		// we avoid the strict ordering with this algo (busy working with the slower processes) while only one process is going inside the critical section
		// 3. Disallowing software traps - leads to context switching for processor; isn't optimal for one core with shared memory
		// 4. Special atom instructions from processor for synchronization
		// 5. Test and Set - it's always an atom function in the processor context; we check and set a shared variable by which we lock processess which should and shouldn't be inside the critical section
		// this uses a variable lock (when lock is true,  there is a process in the critical section, else not)
		// returns false and while(!testandset(&lock)) -> !false = true ==> process waits
		// if lock is false, it sets it to true and testandset() returns true, which negates the condition above 
		// and allows for the thread to continue in the critical section
		// 6. Swap - using the same but with swap (as an atomic operation; needs to be defined by the processor and not written)
		// this using a second variable called val and it mingles between lock (which is probably a thread-local var)
		// All of these algorithmns use busy waiting (while loop waiting); this is bad
		// and they're hard for hardware maintenance (uniform memory access)
		// !7. Semaphores - special kind of variables used for interprocess communication. Using semaphores, a given process can send a signal to another process and/or expect a result from a process
		// Whenever a given semaphor waits to receive a signal, it becomes "blocked"
		// every semaphore is associated with a whole variable and has two operations which can be performed - wait() and signal()
		// wait() and signal() are system calls to the processor
		// wait() decrements the semaphore amount with 1 (before critical section)
		// signal() increments the semaphore amount with 1 (after critical section)
		// each semaphore has a queue of blocked processes and one whole value for an indicator
		// if the semaphore value reaches negative, the current process becomes blocked (on OS level) and gets assigned to the queue
		// using signal(), one of the blocked threads/processes becomes unblocked and is popped from the queue
		// wait() and signal() are atom operations
		// binary semaphor is often used instead of normal semaphors; or the so-called mutex (it varies between 0 and 1)
		// if there are no blocked processes, the semaphor changes to one (signal), otherwise to zero (wait)
		// since this happens on OS level, there is no need for an infinite while loop for other threads.
}
