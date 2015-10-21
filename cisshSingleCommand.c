/* cisshSingleCommand.c
 */
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

/* External functions --
 */
extern void error(char* message);

/* cisshSingleCommand(char* command[])
 * executes a line containing a single command.
 */
void
cisshSingleCommand(char* command[])
{
  pid_t		pid;
  int		status;

  /* Fork the child process.
   */
  if((pid = fork()) == 0)
    {
      /* This is the child process.
       * exec() the command.
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
