// file:	copyit_extracredit.c
// author:	Patrick Drumm
// class:	cse 30341
//
// compilation:
// 	$ make
//
// usage:
// 	$ copyit_extracredit SourceFile TargetFile
//
// overview:
// 	This program will...
// 	 - recursivley copy the contents of a directory/file from one place to another
// 	 - report the total number of bytes copied
//
// basic outline:
// --------------
// copy()
// 	open the source file
// 	if (directory)
// 		recursively call copy() on all contents of directory
// 	create the target file
// 	loop {
// 		read a bit of data from the source file
// 		write a bit of data to the target file
// 	}
// 	close both files
// print success message

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>	// open(), stat(), chmod()
#include <fcntl.h>	// creat(), open()
#include <string.h>	// strerror()
#include <errno.h>	// errno
#include <signal.h>	// signal()
#include <unistd.h>	// write(), close(), alarm()
#include <dirent.h>	// struct dirent
#include <string.h>	// strcmp()

// define function prototypes
void print_copyit1(char[]); 
void print_copyit3(char[],char[],char[]); 
int usage_error(int);
int open_error(char[],int*);
int creat_error(char[],int*);
void display_message(int);
long copy(char[],char[]);
// global variable to define maximum buffer size
#define MAX_BUF 4096

// begin procedure
int main( int argc, char** argv ){
	/* DEFINE VARIABLES */
	char 	*sourcefile = argv[1],
		*targetfile = argv[2];
	long file_size;

	/* ERROR CHECK */
	// exit program if incorrect usage
	if( usage_error(argc) ){
		print_copyit1("Incorrect number of arguments.");
		printf("usage: copyit_extracredit <sourcefile> <targetfile>\n");
		exit(1);
	}

	/* BEGIN RECURSIVE COPY */
	file_size = copy(sourcefile, targetfile);

	/* SUCCESS */
	printf("Copied %ld bytes from file %s to %s.\n",file_size,sourcefile,targetfile);
	return 0;
}// END MAIN


long copy(char* sourcefile, char* targetfile) {
	/* DEFINE VARIABLES */
	int 	fd_s,	// source file descriptor
		fd_t;	// target file descriptor
	struct stat source_perm;	// holds permissions info of sourcefile to be transfered to targetfile
	struct dirent **namelist;
	int n;
	int num_bytes;
	long file_size = 0;
	char buf[MAX_BUF];
	char source_path[1000], target_path[1000];

	/* OPEN SOURCEFILE */
	// open the sourcefile (and exit program on error)
	if( open_error(sourcefile,&fd_s) ){
		print_copyit3("Unable to open",sourcefile,strerror(errno));
		exit(1);
	}
	// get stats & permissisions of sourcefile (and exit program on error)
	if( stat(sourcefile,&source_perm) < 0 ){
		print_copyit3("Unable to get stats/permissions on",sourcefile,strerror(errno));
		exit(1);
	}

	/* DIRECTORY CASE */
	if( S_ISDIR(source_perm.st_mode) ){
		// create the directory
		if( mkdir(targetfile, source_perm.st_mode&0777) < 0 ){
			print_copyit3("Unable to create the directory",targetfile,strerror(errno));
			exit(1);
		}
		// gather info on each file in dir
		if( (n=scandir(sourcefile,&namelist,NULL,alphasort)) < 0 ){
			print_copyit3("Unable to scan the directory",sourcefile,strerror(errno));
			exit(1);
		}
		// copy each file in dir
		while( --n > 1 ){
			bzero(source_path, sizeof(source_path)); // ensure allocated memory to path is cleared
			bzero(target_path, sizeof(target_path)); // ensure allocated memory to path is cleared
			// add on new filename to the full source/target path
			strcpy(source_path,sourcefile);
			strcat(source_path,"/");
			strcat(source_path,namelist[n]->d_name);
			strcpy(target_path,targetfile);
			strcat(target_path,"/");
			strcat(target_path,namelist[n]->d_name);
			// limit copying to filepath the size of the buffer allocated
			if( strlen(source_path)>999 ){
				printf("Error. File %s was not copied. Path is too deep.\n",sourcefile);
			} else {
				file_size += copy(source_path,target_path);
			}
		}
		free(namelist);
		return file_size;
	}

	/* CREATE TARGETFILE */
	// create the targetfile (and exit program on error)
	if( creat_error(targetfile,&fd_t) ){
		print_copyit3("Unable to create",targetfile,strerror(errno));
		exit(1);
	}
	// change permissions of targetfile to match those of sourcefile (but do not exit on error)
	if( chmod(targetfile,source_perm.st_mode & 07777) < 0 ){
		print_copyit3("Unable to set permissions of",targetfile,strerror(errno));
	}

	/* COPY SOURCEFILE INTO TARGETFILE */
	signal(SIGALRM,display_message);
	alarm(1); // initial alarm set - all others will be called by the display_message function
	do{
		bzero(buf, sizeof(buf)); // ensure allocated memory to buf is cleared
		// the following two while loops will continue trying to read/write if an EINTR occurs. It will exit on other error signals
		while ( (num_bytes=read(fd_s,&buf,sizeof(buf))) < 0 ){
			if( errno!=EINTR ){
				print_copyit3("Unable to read",sourcefile,strerror(errno));
				exit(1);
			}
		}
		while ( (num_bytes=write(fd_t,buf,num_bytes)) < 0 ){
			if( errno!=EINTR ){
				print_copyit3("Unable to write to",targetfile,strerror(errno));
				exit(1);
			}
		}
		// file_size keeps continuous count of # of bytes copied over
		file_size += num_bytes;
	} while( num_bytes >= MAX_BUF );
	// turn off alarm now that copying is finished
	alarm(0);

	/* CLEANUP */
	close(fd_s);
	close(fd_t);
	
	/* SUCCESS */
	return file_size;
}

// print_copyit[13] are pre-formatted print functions to standardize output messages
void print_copyit1(char* msg) {
	printf("copyit_ec: ");
	printf("%s\n", msg);
}
void print_copyit3(char* msg, char* file, char* error) {
	printf("copyit_ec: ");
	printf("%s %s: %s\n", msg, file, error);
}

// helper function returns 1 if incorrect number of arguments used
int usage_error(int argc) {
	int expected_argc = 3;
	return( argc != expected_argc );
}

// returns 1 if file was successfully opened. Also replaces fd_s in main with the file descriptor of the file opened
int open_error(char* sourcefile, int* fd) {
	(*fd) = open(sourcefile, O_RDONLY, 0);
	return( (*fd) < 0 );
}

// returns 1 if file was successfully created. Also replaces fd_t in main with the file descriptor of the file created
int creat_error(char* targetfile, int* fd) {
	(*fd) = open(targetfile, O_CREAT | O_WRONLY);
	return( (*fd) < 0 );
}

// prints to stdout while the file is being copied over; this function is the signal handler for SIGALRM
void display_message(int s) {
	print_copyit1("still copying...");
	alarm(1);
}
