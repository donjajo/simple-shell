#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include <limits.h>
#include <string.h>
#include <sys/wait.h>
#include <libgen.h>
#include <time.h>

int show_time;
struct commands {
	char program[NAME_MAX];
	char args[ARG_MAX];
};

void get_prompt() {
	char hostname[HOST_NAME_MAX];
	struct tm *local_t;
	char time_format[20];
	
	if( show_time ) {
		time_t t = time( NULL );
		local_t = localtime( &t );
		strftime( time_format, 20, ":\033[0;36m%H:%M\033[0m", local_t );
	}
	gethostname( hostname, HOST_NAME_MAX );

	printf( 
		"[%s@%s%s %s]$ ", 
		getenv( "USER" ), 
		hostname, 
		( show_time ? time_format : "" ),
		basename( getcwd( NULL, PATH_MAX ) ) 
		);
}

int enable_config( struct commands commands ) {
	int config;
	if( strcmp( commands.program, "show-time" ) == 0 ) {
		show_time = 1;
		return 1;
	}

	return 0;
}

struct commands split_commands( char *command ) {
	struct commands commands;
	char *str;
	int i = 0;

	str = strtok( command, " " );
	while( str != NULL ) {
		if( i == 0 ) {
			strcpy( commands.args, "" ); // Empty previous args. Don't know why it retains
			strcpy( commands.program, str );
		}
		else {
			strcat( commands.args, str );
			strcat( commands.args, " " );
		}
		str = strtok( NULL, " " );
		i++;
	}
	commands.args[ strlen( commands.args)-1 ] = '\0'; //Remove ending spaces

	return commands;
}

void process_command( char *command ) {
	
	struct commands com_para = split_commands( command );
	char *n[3] = { com_para.program, ( !strlen( com_para.args) ? NULL : com_para.args ), NULL };

	if( strcmp( com_para.program, "cd" ) == 0 ) {
		if( chdir( com_para.args ) < 0 ) {
			perror( "cd" );
		}
	} 
	else if( enable_config( com_para ) ) {

	}
	else {
		if( !fork() ) {
			execvp( com_para.program, n );

			printf( "%s: Command not found\n", com_para.program );
			exit(1);
		}
		else {
			wait(0);
		}
	}
}

void main( int argc, char *argv[] ) {
	char command[ARG_MAX];

	get_prompt();
	while( fgets( command, ARG_MAX, stdin ) != NULL ) {
		command[ strlen( command )-1] = '\0';
		if( strlen( command ) )
			process_command( command );
		get_prompt();
	}
}