/* cisshPipe.c
 */

#include <signal.h>
/* External functions --
 */
extern void error(char* message);

/* cisshPipe(char* command1[], char* command2[])
 * handles command lines with pipes.
 */
void
cisshPipe(char* command1[], char* command2[])
{
  pid_t	pid;
  int status;
  int fd[2];

  /* Fork the child process.
   */
  pipe(fd);

  if((pid = fork()) == 0)
    {
	dup2(fd[1],1);
	close(fd[0]);	

      	execvp(command1[0], command1);

      /* If the exec failed for any reason (e.g. the command may not exist,
       * or the permissions may be wrong).
       */
      
    }
  else
    {

	dup2(fd[0],0);
	close(fd[1]);	

      	execvp(command2[0], command2);

	}
	close(fd[0]); close(fd[1]);
}