
/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD
%token	<great> GREAT

%token 	NOTOKEN NEWLINE LESS BACK PIPE APPEND_ERR GREAT_ERR APPEND EXIT

%union	{
		char   *string_val;
		char   *great;
	}

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include "command.h"
#include <unistd.h>
#include <glob.h>
#include <string.h>
%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command 
	;

command: simple_command
        ;

simple_command:	
	pipes iomodifier_opt NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	|
	pipes iomodifier_opt AMP NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| NEWLINE 
	| error NEWLINE { yyerrok; }
	| exitShell
	;
pipes:
	pipes PIPE command_and_args
	|	command_and_args
	;
command_and_args:
	command_word arg_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

arg_list:
	arg_list argument
	| 
	/* can be empty */
	;

argument:
	WORD {
               
			    char *word=$1; //Take whole word
			    if(strstr(word,"?")!=NULL|| strstr(word,"*")!=NULL ){   //special characters check
					glob_t globTst;
					if(glob(word,GLOB_ERR, NULL, &globTst)!=0){ //Checks the pattern and returns output in globalTst.gl.pathv and prints if error occurs
						perror("ERROR in glob");
					}
					for(int i=0;i< globTst.gl_pathc;i++){
						printf("   Yacc: insert argument \"%s\"\n", globTst.gl_pathv[i]);
						Command::_currentSimpleCommand->insertArgument( globTst.gl_pathv[i] );
					}
				}
				else{
					printf("   Yacc: insert argument \"%s\"\n", $1);
					Command::_currentSimpleCommand->insertArgument( $1 );
				}
	       
	}
	;

command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;
iomodifier_opt:
	GREAT WORD {
		Command::_currentCommand.aF=0;
		printf("   Yacc: insert output \"%s\"\n", $1);
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
	}
	| 
	LESS WORD{
		Command::_currentCommand.aF=0;
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}
	| 
	GREAT WORD LESS WORD{
		Command::_currentCommand.aF=0;
		printf("   Yacc: insert output \"%s\"\n", $2);
		printf("   Yacc: insert input \"%s\"\n", $4);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._inputFile = $4;
	}
	| 
	LESS WORD GREAT WORD{
		Command::_currentCommand.aF=0;
		printf("   Yacc: insert output \"%s\"\n", $4);
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._outFile = $4;
		Command::_currentCommand._inputFile = $2;
	}
	|
	APPEND WORD{
		Command::_currentCommand.aF=1;
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;		
	}
	| 
	APPEND_ERR WORD{
		Command::_currentCommand.aF=1;
		printf("   Yacc: ERROR APPEND \n");
		Command::_currentCommand._errFile = $2;
		Command::_currentCommand._outFile = $2;	
	}
	|
	GREAT_ERR WORD{
		Command::_currentCommand.aF=0;
		printf("   Yacc: GREAT APPEND \n");
		Command::_currentCommand._errFile = $2;
		Command::_currentCommand._outFile = $2;
	}
	|
	LESS WORD APPEND WORD{
		Command::_currentCommand.aF=1;
		printf("   Yacc: insert output \"%s\"\n", $4);
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._outFile = $4;
		Command::_currentCommand._inputFile = $2;
	}
	|
	LESS WORD GREAT_ERR WORD{
		Command::_currentCommand.aF=0;
		printf("   Yacc: insert output \"%s\"\n", $4);
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._errFile = $4;
		Command::_currentCommand._outFile = $4;
		Command::_currentCommand._inputFile = $2;
	}
	|
	LESS WORD APPEND_ERR WORD{
		Command::_currentCommand.aF=1;
		printf("   Yacc: insert output \"%s\"\n", $4);
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._errFile = $4;
		Command::_currentCommand._outFile = $4;
		Command::_currentCommand._inputFile = $2;
	}
	|
	;
AMP:
	BACK {
		printf("   Background mode = true \n");
		Command::_currentCommand._background = 1;
	}
	;
exitShell:
	EXIT {
		printf("GOODBYE\n");
		printf("Leaving shell\n");
		exit(1);
	}
	;

%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
