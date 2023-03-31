
/*
 *
 * CS-413 Spring 98
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <string.h>
#include "y.tab.h"

%}

%%

\n 	{
		return NEWLINE;
	}

[ \t] 	{
		/* Discard spaces and tabs */
     	}	
"exit"	{
		return EXIT;
}
">>"	{
		return APPEND;
}
">>&"	{
		return APPEND_ERR;
}
">&"	{
		return GREAT_ERR;
}
">" 	{
		yylval.great=">";
		return GREAT;
	}
"<"		{
		return LESS;
}
"&"		{
		return BACK;
}
"|"		{
		return PIPE;
}

[^ ><&|\t\n][^ ><&|\t\n]*  {
		/* Assume that file names have only alpha chars */
		yylval.string_val = strdup(yytext);
		return WORD;
	}

.	{
		/* Invalid character in input */
		return NOTOKEN;
	}

%%
	
