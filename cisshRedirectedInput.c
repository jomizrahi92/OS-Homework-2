/* cisshRedirectedInput.c
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

/* External functions --
 */
extern void error(char* message);

/* cisshRedirectedInput(char* command[], char* inputFile)
 * handles command lines with redirected input.
 */
void
cisshRedirectedInput(char* command[], char* inputFile)
 {
  pid_t	pid;
  int status;
  int fd;

  /* Fork the child process.
   */
  if((pid = fork()) == 0)
    {
      /* This is the child process.
       * Open the output file, failing on an error.
       */
      fd = open(inputFile, O_RDONLY, 0775);
      if(fd < 0)
	{
	  error("cissh: error opening standard intput file");
	  exit(1);
	}

      /* Close the existing standard output and use dup() to copy the
       * redirected file to file descriptor number 1.
       */
      close(0);
      if(dup(fd) < 0)
	{
	  error("cissh: error duplicating standard input");
	  perror("dup()");
	  exit(1);
	}

      /* We no longer need the, now redundant, file descriptor.
       */
      close(fd);

      /* exec() the command.
       */
      execvp(command[0], command);

      /* If the exec failed for any reason (e.g. the command may not exist,
       * or the permissions may be wrong).
       */
      error("cissh: failure to execute command");
      exit(1);
    }
  else
    {
      /* This is the parent process.
       * Wait for the child to terminate.
       */
      if(wait(&status) < 0)
	{
	  error("cissh: error waiting for child.");
	  perror("wait");
	}

      if(status != 0)
	error("cissh: command exited with nonzero error status.");
    }
}
