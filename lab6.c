#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/sem.h>

#define SIZE 16
#define KEY 0x1111


struct sembuf waitop = { 0, -1, SEM_UNDO};
struct sembuf sigop = { 0, +1, SEM_UNDO};

int main (int argc, char* argv[])
{
   int status;
   long int i, loop, temp, *shmPtr;
   int shmId;
   pid_t pid;
   int semId;

   

   // get value of loop variable (from command-line argument)
   if (argc <= 1) {
		perror("Error: Missing Argument");
		exit(1);	
	}
	else {
		loop = atol(argv[1]);
	}

   if ((shmId = shmget (IPC_PRIVATE, SIZE, IPC_CREAT|S_IRUSR|S_IWUSR)) < 0) {
      perror ("i can't get no..\n");
      exit (1);
   }
   if ((shmPtr = shmat (shmId, 0, 0)) == (void*) -1) {
      perror ("can't attach\n");
      exit (1);
   }

   shmPtr[0] = 0;
   shmPtr[1] = 1;

   // create semaphore
  if ((semId = semget (KEY, 1, 00600 | IPC_CREAT)) < 0) {
     perror("Error: Semaphore not created");
     exit(1);
   }


  // initialize semaphore to counter 1
  
  if(semctl(semId, 0, SETVAL, 1) < 0)
  {
    perror("Error: Semaphore not initialized"); 
	  exit(1);
  }
 
   if (!(pid = fork())) {
      for (i=0; i<loop; i++) {

	      // semapohore wait operation
	      if(semop(semId, &waitop, 1)== -1) {
              perror("Error: wait semop");
              exit(EXIT_FAILURE);
	      }
               // swap the contents of shmPtr[0] and shmPtr[1]
	       temp = shmPtr[0];
	       shmPtr[0] = shmPtr[1];
	       shmPtr[1] = temp;

	       // semaphore signal operation
	       if (semop(semId, &sigop, 1) == -1) {
               perror("Error: signal semop");
               exit(EXIT_FAILURE);     
	       }
      }
      
	 if (shmdt (shmPtr) < 0) {
         perror ("just can't let go\n");
         exit (1);
      }
      exit(0);
   }
   else
      for (i=0; i<loop; i++) {

	      //semaphore wait opearation
              if (semop(semId, &waitop, 1) == -1) {
              perror("Error: wait semop");
             exit(EXIT_FAILURE);
	      }

               // swap the contents of shmPtr[1] and shmPtr[0]
	       temp = shmPtr[0];
	       shmPtr[0] = shmPtr[1];
	       shmPtr[1] = temp;
          
          //sempahore signalling
          if (semop(semId, &sigop, 1) == -1) {
          perror("Error: signal semop");
          exit(EXIT_FAILURE);      
	  }
      }

   wait (&status);
   printf ("values: %li\t%li\n", shmPtr[0], shmPtr[1]);

   if (shmdt (shmPtr) < 0) {
      perror ("just can't let go\n");
      exit (1);
   }
   if (shmctl (shmId, IPC_RMID, 0) < 0) {
      perror ("can't deallocate\n");
      exit(1);
   }

   // remove the semaphore referenced by semId
   semctl (semId, 0, IPC_RMID);

   return 0;
}
