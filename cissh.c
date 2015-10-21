/* cissh.c
 *
 * A shell for the cissh assignment in CIS 704.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

/* External routines --
 */
extern void cisshSingleCommand(char* command[]);
extern void cisshRedirectedInput(char* command[], char* standardInput);
extern void cisshRedirectedOutput(char* command[], char* standardOutput);
extern void cisshPipe(char* command1[], char* command2[]);

/* Constants --
 */
#define MAXLINE		(1024 + 1)
#define MAXTOKENS	(256 + 1)

/* Global data --
 */
int			verbose;

/* Global routines --
 */
int			main(int argc, char* argv[]);
void			error(char* message);

/* Static data --
 */
static int		ttyp;
static int		done;

/* Static routines --
 */
static int		isWhitespace(char c);
static int		isSeparator(char c);
static void		processLine(char* line);
static int		tokenize(char* line, char* tokens[], int size, int* tokenCount);
static void		displayTokens(char* description, char* tokens[]);

/* Here we go.  Hold onto your hats.
 */
int
main(int argc, char* argv[])
{
  char			inputLine[MAXLINE];
  char*			cp;

  /* We want stdout to be unbuffered so that the output mixes properly with
   * stderr.
   */
  setvbuf(stdout, NULL, _IONBF, 0);

  /* 'ttyp' indicates whether the standard input is a terminal.
   * We use this to control the printing of a prompt.
   */
  ttyp = isatty(0);

  /* Repeat until we are done, for some reason or another, or we reach the
   * end of the input.
   */
  done = 0;
  while(!done && !feof(stdin))
    {
      /* If we are talking to a human (i.e., this is a terminal) then
       * display a helpful prompt.
       */
      if(ttyp)
	fprintf(stdout, "cissh-| ");

      /* Get the next line of input.
       * We set errno to 0 to differentiate between end-of-file and an input
       * error.
       */
      errno = 0;
      if(fgets(inputLine, MAXLINE, stdin) == NULL)
	{
	  if(errno != 0)
	    perror("fgets()");
	  done++;
	  continue;
	}

      /* Replace the '\n' with '\0'.
       * We do this just to improve the display of the lines.
       */
      cp = strchr(inputLine, '\n');
      if(cp)
	*cp = '\0';

      /* Show the line we are working on.
       */
      printf("==== %s\n", inputLine);

      /* Process the line.
       */
      processLine(inputLine);
    }  

  /* If we're talking to a terminal, add an extra '\n' to the end.
   * I'm not sure why.
   */
  if(ttyp)
    printf("\n");

  exit(0);
}

/* isWhitespace(c)
 * returns true if 'c' is a whitespace character.
 */
static int
isWhitespace(char c)
{
  return ((c == ' ')
	  || (c == '\t')
	  || (c == '\r')
	  || (c == '\n'));
}

/* isSeparator(c)
 * returns true if 'c' is a separator.
 */
static int
isSeparator(char c)
{
  return ((c == '#')
	  || (c == ';')
	  || (c == '|')
	  || (c == '<')
	  || (c == '>'));
}

/* tokenize(line, tokens, size, &tokenCount)
 * parses the input line, 'line', breaking it into tokens.
 * Tokens are strings separated by whitespace.
 *
 * The tokens are returned in the array 'tokens'.
 * 'size' is the maximum number of entries in 'tokens' (counting the tokens
 * and the terminating NULL).
 * Upon return 'tokenCount' contains the number of tokens found.
 *
 * Note that the simple way that we parse demands that separators and other
 * punctuation be surrounded by whitespace.
 * That is, there must be whitespace around '>', '<', '|', and ';'.
 */
static int
tokenize(char* line, char* tokens[], int size, int* tokenCount)
{
  char*		cp;
  char*		tokenStart;
  int		done;

  /* Initialize.
   */
  *tokenCount = 0;
  cp = line;
  done = 0;

  /* Repeat until were done or we reach the end of the input line.
   */
  while(!done && *cp)
    {
      /* Skip any whitespace.
       */
      while(*cp && isWhitespace(*cp))
	cp++;

      /* We're at the beginning of a token, so mark it.
       * Skip to the end of the token (i.e., the next whitespace).
       */
      tokenStart = cp;
      while(*cp && !isWhitespace(*cp))
	cp++;

      /* If 'cp' == 'tokenStart' then the token is empty, so go on.
       */
      if(cp > tokenStart)
	{
	  /* We must terminate the token string.
	   * If we're looking at the end of the input string, then we're
	   * done.
	   * Otherwise, we replace the whitespace with the end of string.
	   */
	  if(*cp == '\0')
	    done++;
	  else
	    {
	      *cp = '\0';
	      cp++;
	    }

	  /* Add the token to the list of tokens.
	   * Make sure we are not overflowing the list.
	   * If we have, return an error.
	   */
	  tokens[*tokenCount] = tokenStart;
	  *tokenCount += 1;
	  if(*tokenCount >= (size - 1))
	    {
	      fprintf(stderr, "cissh: Too many tokens in the input line.\n");
	      return 0;
	    }
	}
    }

  /* Terminate the list of tokens with NULL.
   * Return success.
   */
  tokens[*tokenCount] = NULL;
  return 1;
}

/* displayTokens(char* description, char* tokens[])
 * is a utilitiy function which displays a list of tokens on a line along
 * with 'description'.
 */
static void
displayTokens(char* description, char* tokens[])
{
  char*		comma;
  char**	token;

  printf("%s", description);
  comma = " ";
  for(token = tokens; *token; token++)
    {
      printf("%s\"%s\"", comma, *token);
      comma = ", ";      
    }
  printf("\n");
}

/* error(char* message)
 * displays error messages on stderr.
 */
void
error(char* message)
{
  fprintf(stderr, "ERROR: %s\n", message);
}

/* processLine(char* line)
 * processes a line from the script file.
 * This is the main routine.
 */
static void
processLine(char* line)
{
  char**		command1;
  char**		command2;
  char			separator;
  char*			filename;
  char*			tokens[MAXTOKENS + 1];
  char**		token;
  int			tokenCount;
  int			i;
  int			ok;

  /* Break the line into tokens.
   * If the is a problem parsing, we do not continue with this line.
   */
  ok = tokenize(line, tokens, MAXTOKENS, &tokenCount);
  if(!ok)
    return;

  /* Make sure that the last token of the line is the semicolon.
   * This makes life easier later on.
   * I suspect that this means that one cannot have comments after a command.
   */
  if((tokenCount < 1) || (*tokens[tokenCount - 1] != ';'))
    {
      tokens[tokenCount] = ";";
      tokenCount++;
      tokens[tokenCount] = NULL;
    }

  /* If there is only one token (i.e., the semicolon) then we have nothing
   * to do.
   */
  if(tokenCount < 2)
    return;

  if(verbose)
    displayTokens("tokens:", tokens);

  /* Now we want to break the line into the separate command(s) and find the
   * separator between the command(s).
   */
  separator = ';';
  token = tokens;
  command1 = &token[0];
  i = 0;
  while((i < tokenCount) && !isSeparator(*token[i]))
    i++;

  if(i >= tokenCount)
    {
      error("cissh: semicolon is missing, this should not happen");
      return;
    }

  /* We've found the separator, so save it.
   * We now know where the second command begins, if there is one.
   */
  separator = *token[i];
  token[i] = NULL;
  i++;
  command2 = &token[i];

  if(verbose)
    displayTokens("command1:", command1);

  if(verbose)
    printf("separator = '%c'\n", separator);

  /* If separator is not ';', then there is a second command so we process
   * it a little.
   */
  if(separator != ';')
    {
      while((i < tokenCount) && !isSeparator(*token[i]))
	i++;
      token[i] = NULL;
      i++;

      if(verbose)  
	displayTokens("command2:", command2);
    }

  /* A sanity check.
   */
  if(i != tokenCount)
    {
      error("cissh: parsing error, probably too many commands");
      return;
    }

  /* Now, depending on the separator execute the line.
   */
  switch(separator)
    {

      /* There is a single command on this line, execute it.
       */
    case ';':
      cisshSingleCommand(command1);
      break;

      /* We have redirected input.
       * Get the input filename.
       * Make sure there is nothing following the filename.
       * Execute it.
       */
    case '<':
      filename = command2[0];

      if(command2[1] != NULL)
	error("cissh: too many tokens after a redirect");

      cisshRedirectedInput(command1, filename);
      break;

      /* We have redirected output.
       * Get the output filename.
       * Make sure there is nothing following the filename.
       * Execute it.
       */
    case '>':
      filename = command2[0];

      if(command2[1] != NULL)
	error("cissh: too many tokens after a redirect");

      cisshRedirectedOutput(command1, filename);
      break;

      /* We have a pipe.
       * Do it.
       */
    case '|':
      cisshPipe(command1, command2);
      break;

      /* For comments, we do nothing.
       * Comments should be taken care of in the parsing routine.
       */
    case '#':
	break;

	/* Hmmm... don't know what this is.
	 */
    default:
      error("cissh: unknown separator");
      break;
    }
}
