
/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <chrono>
#include <glob.h>
#include <ctime>
#include <fstream>
using namespace std;

#include "command.h"
int cdFlag = 0;
int err;
SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}
	
	_arguments[ _numberOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}

void
Command:: clear()
{
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _outFile || _errFile) {
		free( _outFile );
	}

	if ( _inputFile ) {
		free( _inputFile );
	}

	/*if ( _errFile ) {
		free( _errFile );
	}*/

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	aF=0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	printf("APPEND FLAG = %d \n",aF);
	
}

void
Command::execute()
{
	cdFlag=0;
	err=0;
	// Don't do anything if there are no simple commands
	if ( _numberOfSimpleCommands == 0 ) {
		prompt();
		return;
	}
	if(!strcmp(_simpleCommands[0]->_arguments[0],"cd")){
		char s[100];
		cdFlag=1;
		if(_simpleCommands[0]->_numberOfArguments==1){
			chdir(getenv("HOME"));	
		}
		else{
			chdir(_simpleCommands[0]->_arguments[1]);
			}	
	}
	
	// Print contents of Command data structure
	print();
	int defaultIn=dup(0);
	int defaultOut=dup(1);
	int defaultErr=dup(2);
	int inpFile;
	if(_inputFile){
		inpFile=open(_inputFile,O_RDONLY);
	}else{
		inpFile=dup(defaultIn);
	}
	int outFile;
	int errFile;
	int pid;
	for (int i=0; i< _numberOfSimpleCommands ; i++){
		dup2(inpFile,0);
		if(i==_numberOfSimpleCommands-1){
			if(_outFile){
				if(aF){
					outFile=open(_outFile,O_APPEND | O_CREAT | O_WRONLY ,0666);
				}else{
					outFile=creat(_outFile,0666);
				//dup2(outFile,1);
				//close(outFile);
			}
			}
			else{
				outFile=dup(defaultOut);
			}
			if(_errFile){
				if(aF){
					errFile=open(_errFile,O_APPEND | O_CREAT | O_WRONLY ,0666);
				}else{
					errFile=creat(_errFile,0666);
				//dup2(outFile,1);
				//close(outFile);
			}
			}
			else{
				errFile=dup(defaultErr);
			}
		}
		else{ //Any other simple command (for pipes)
			int fdpipe[2];
			pipe(fdpipe); //destructures the fdpipe array
			inpFile=fdpipe[0];
			outFile=fdpipe[1];
		}
		dup2(outFile,1);
		dup2(errFile,2);
		close(outFile);
		pid=fork();
		//child process
		if(pid==0){
			//close(errFile);
			//close(outFile);
			//close(inpFile);
			execvp(_simpleCommands[i]->_arguments[0],_simpleCommands[i]->_arguments);
			if(cdFlag==0){
				perror("Execution error");
			}
			exit(1);
		}else if(pid<0){
			perror("Forking error");
			return;
		}
		
	}

	
	if(!_background){
		waitpid(pid,NULL,0);
	}
	dup2(defaultIn,0);
	dup2(defaultOut,1);
	dup2(defaultErr,2);
	close(errFile);
	close(outFile);
	close(inpFile);
	if(!err && !_background){
		prompt();
	}
	clear();
	
	
}


void logFile(int x){
	if(cdFlag==0){
		std::ofstream outfile;
		outfile.open("/home/zeftawy/lab3-src/log.txt",std::ios_base::app);
		auto end =std::chrono::system_clock::now();
		std::time_t end_time=std::chrono::system_clock::to_time_t(end);
		outfile << "Process ended at:" << std::ctime(&end_time);
		outfile.close();
}
}

void ctrlReject(int x){
	printf("\nZefto w Anas>");
	signal(SIGINT,ctrlReject);
	signal(SIGCHLD,logFile);
	fflush(stdout);
	err=1;
	
}

void
Command::prompt()
{
	printf("Zefto w Anas>");
	signal(SIGINT,ctrlReject);
	signal(SIGCHLD,logFile);
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

int 
main()
{
	Command::_currentCommand.prompt();
	yyparse();
	
	return 0;
}

