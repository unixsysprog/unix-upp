/* builtin.c
 * contains the switch and the functions for builtin commands
 */
#include	"builtin.h"
#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>
#include    <errno.h>
#include	<stdlib.h>
#include    <string.h>
#include    <unistd.h>
#include	"varlib.h"
#include	"smsh.h"

static int is_exit_request(const char*, int*);
static int is_change_dir(char**, int*);
static int is_export(char**, int*);
static int is_read_var(char**, int*);
static int is_comment(char* cmd, int* resultp);

/*
 * Runs command if built in.
 * @args: args - an array of pointers.
 * @args: resultp - returns 0 if built in command executed without error.
 * @rets: returns 1 if command was built in, 0 otherwise.
 */
int is_builtin(char **args, int *resultp)
{
	if (is_assign_var(args[0], resultp))
		return 1;
	if (is_list_vars(args[0], resultp))
		return 1;
	if (is_export(args, resultp))
		return 1;
    if (is_change_dir(args, resultp))
        return 1;
    if (is_exit_request(args[0], resultp))
        return 1;
    if (is_read_var(args, resultp))
        return 1;
    if (is_comment(args[0], resultp))
        return 1;

	return 0;
}

/* 
 * Checks if cmd is a legal assignment.
 * @args: cmd - command of type char*.
 * @args: resultp - returns 0 if built in command executed without error.
 * @rets: returns 1 if cmd was built in, 0 otherwise.
 */
int is_assign_var(char *cmd, int *resultp)
{
	if (strchr(cmd, '=') != NULL){
		*resultp = assign(cmd);
		if (*resultp != -1)
			return 1;
	}
	return 0;
}

/* 
 * Checks if command is "set" : if so list vars.
 * @args: cmd - command to check.
 * @args: resultp - 0 if built in and executes successfully.
 * @rets: returns 1 if built in, 0 otherwise.
 */
int is_list_vars(char *cmd, int *resultp)
{
	if (strcmp(cmd,"set") == 0){	     /* 'set' command? */
		VLlist();
		*resultp = 0;
		return 1;
	}
	return 0;
}

/*
 * if an export command, then export it and ret 1
 * else ret 0 
*/
static int is_export(char **args, int *resultp)
{
	if (strcmp(args[0], "export") == 0){
		if (args[1] != NULL && okname(args[1]))
			*resultp = VLexport(args[1]);
		else
			*resultp = 1;
		return 1;
	}
	return 0;
}

static void path_err(const char *msg)
/* Handles syntax errors in path names in the call to is_change_dir. */
{
    fprintf(stderr, "path name error: %s\n", msg);
}


static int is_change_dir(char **args, int *resultp)
/* 
 * Checks if argument is cd command, if so than executes.
 */
{
    int rv = 0;
    
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] != NULL) {
            if (chdir(args[1]) == -1) {
                path_err("Invalid path");
                *resultp = -1;
            } else {
                *resultp = 0;
                rv = 1;
            }
        } else {
            if (chdir("/") == -1) {
                path_err("Cant get to root.");
                *resultp = -1;
            } else {
                *resultp = 0;
                rv = 1;
            }
        }
    }
    return rv;
}
            

static int is_exit_request(const char* cmd, int* resultp)
/*
 * Exits with success if cmd is to exit.
 */
{
    if (strcmp(cmd, "exit") == 0) {
        exit(EXIT_SUCCESS);
    }
    return 0;
}

static int is_read_var(char** args, int* resultp)
{
    if (strcmp(args[0], "read") == 0 && okname(args[1])) {
        char buf[1024];
        fgets(buf, sizeof(buf), stdin);
        if (VLstore(args[1], buf) != 0);
            *resultp = -1;
        
        return 1;
    }
    return 0;
}

static int is_comment(char* cmd, int* resultp)
{
    if (cmd[0] == '#') {
        *resultp = 0;
        return 1;
    }
    return 0;
}

int assign(char *str)
/*
 * purpose: execute name=val AND ensure that name is legal
 * returns: -1 for illegal lval, or result of VLstore 
 * warning: modifies the string, but retores it to normal
 */
{
	char	*cp;
	int	rv ;

	cp = strchr(str,'=');
	*cp = '\0';
	rv = (okname(str) ? VLstore(str,cp+1) : -1);
	*cp = '=';
	return rv;
}

int okname(char *str)
/*
 * purpose: determines if a string is a legal variable name
 * returns: 0 for no, 1 for yes
 */
{
	char	*cp;

	for(cp = str; *cp; cp++){
		if ((isdigit(*cp) && cp==str) || !(isalnum(*cp) || *cp=='_'))
			return 0;
	}
	return (cp != str);	/* no empty strings, either */
}

/*
 * step through args.  REPLACE any arg with leading $ with the
 * value for that variable or "" if no match.
 * note: this is NOT how regular sh works
 */
void varsub(char **args)
{
	int	i;
	char	*newstr;

	for(i = 0 ; args[i] != NULL ; i++)
		if (args[i][0] == '$'){
			newstr = VLlookup(args[i]+1);
			if (newstr == NULL)
				newstr = "";
			free(args[i]);
			args[i] = strdup(newstr);
		}
}
