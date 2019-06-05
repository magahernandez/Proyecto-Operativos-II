#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <ctype.h>

char *builtin_str[] = {
  "ls",
  "grep",
  "chmod"
};

char *builtin_str_ls[] = {
	"-i",
	"-G",
	"-g",
	"-h",
	"-R"
};

char *builtin_str_grep[] = {
	"-i",
	"-v"
};

int _ls(char **args)
{
  return 1;
}

int _grep(char **args)
{
  return 1;
}

int _chmod(char **args)
{
  return 1;
}


int (*builtin_func[]) (char **) = {
  &_ls};

void *strconcat(char * a, char * b) {
	int lena = strlen(a);
    int lenb = strlen(b);
    char *con = malloc(lena+lenb+1);

    memcpy(con,a,lena);
    memcpy(con+lena,b,lenb+1);        
    return con;
}


// Lee la linea que se escribio
char *read_line(void)
{
    int bufsize = 1024;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if ( NULL == buffer) {
    	fprintf(stderr, "lsh: allocation error\n");
    	exit(EXIT_FAILURE);
  	}

    while (1) {
    	c = getchar();
    	if (c == EOF) {;
        	exit(EXIT_SUCCESS);
    	} 
    	else if (c == '\n') {
        	buffer[position] = '\0';
      		return buffer;
    	} 
    	else {
      		buffer[position] = c;
    	}
    	position++;
    }
}

// Tokeniza la linea de input
char **split_line(char *line, int * num_args)
{
	// Cantidad de tokens maxima
  int bufsize = 64;
  // Token actual escribiendo
  int position = 0;
  // Arreglo de todos los comandos
  char **tokens = malloc(bufsize * sizeof(char*));
  memset(tokens, 0, sizeof(char) * bufsize);

  if (NULL == tokens) {
    fprintf(stderr, "allocation error\n");
    exit(EXIT_FAILURE);
  }

  int begin, end;
  begin = 0;
  // Tokenizamos programas, operadores, flags, archivos
  for(end = 1; end < strlen(line); end++) {
  	if (line[end] == ' ') {
  		if (end == begin) {
  			begin++;
  			continue;
  		}
  		tokens[position] = malloc(sizeof(char) * 64);
  		memset(tokens[position], 0, sizeof(char) * 64);
  		memcpy(tokens[position], line+begin, end - begin);
  		begin = end + 1;
  		position++;
  	}
  	else if (line[end] == '|' || line[end] == '>' || line[end] == '<') {
  		// chequeamos el caracter anterior
  		if (line[end - 1] == ' ') {
  			tokens[position] = malloc(sizeof(char) * 64);
  			memset(tokens[position], 0, sizeof(char) * 64);
  			memcpy(tokens[position], line+begin, 1);
  			begin++;
  			position++;
  		}
  		else {
  			// Guardamos dos tokens, el anterior y el operador
  			tokens[position] = malloc(sizeof(char) * 64);
  			memset(tokens[position], 0, sizeof(char) * 64);
  			memcpy(tokens[position], line+begin, end - begin);
  			position++;
  			tokens[position] = malloc(sizeof(char) * 64);
  			memset(tokens[position], 0, sizeof(char) * 64);
  			memcpy(tokens[position], line+end, 1);
  			position++;
  			begin = end + 1;
  		}
  	}
  }
  tokens[position] = malloc(sizeof(char) * 64);
  memset(tokens[position], 0, sizeof(char) * 64);
  memcpy(tokens[position], line+begin, end - begin + 1);
  position++;

  // Imprimir todos los tokens
  //for (int i = 0; i < position; i++) {
  //	printf("%s\n", tokens[i]);
  //}
  // exit(0);
  *num_args = position;
  return tokens;
}

const char * months[12] = {"January", "February", "March", "April", "May", "June", "July", 
							"August", "September", "October", "November", "December"};



void loop(void){
  char *line;
  char **args;
  int num_args;
  do
  {	
    printf("> ");
    line = read_line();
    // Tokenizamos la linea de input
    args = split_line(line, &num_args);
    // Realizamos el plan de ejecucion


    // Caso ls
    if (strcmp(args[0], builtin_str[0]) == 0){

    	// Caso "ls"
    	if (args[1] == NULL){

    		struct passwd *user;
    		struct group *group;
    		struct tm * mtime;

    		char timeString[9];
  
   			DIR *repertorio = opendir (".");

    		struct stat info; // Stat

    		struct dirent *actual; //Archivo actual

    		int s = 0;

    		while ((actual = readdir(repertorio))){
        		stat(actual->d_name, &info); // Se consulta el archivo actual y se almacena en info
        		user = getpwuid(info.st_uid); //Database for the user
       	 		group = getgrgid(info.st_gid); //Database for the group
        		time_t current_time= info.st_mtime;
        		mtime = localtime(&current_time);
        		strftime(timeString,9,"%d %H:%M", mtime);
 				
 				// Ignoramos . y ..
 				if ((strcmp(actual->d_name, "..") != 0) && (strcmp(actual->d_name, ".") != 0)){
 				
	 				s += info.st_blocks;

				    printf( (S_ISDIR(info.st_mode)) ? "d" : "-");
				    printf( (info.st_mode & S_IRUSR) ? "r" : "-");
				    printf( (info.st_mode & S_IWUSR) ? "w" : "-");
				    printf( (info.st_mode & S_IXUSR) ? "x" : "-");
				    printf( (info.st_mode & S_IRGRP) ? "r" : "-");
				    printf( (info.st_mode & S_IWGRP) ? "w" : "-");
				    printf( (info.st_mode & S_IXGRP) ? "x" : "-");
				    printf( (info.st_mode & S_IROTH) ? "r" : "-");
				    printf( (info.st_mode & S_IWOTH) ? "w" : "-");
				    printf( (info.st_mode & S_IXOTH) ? "x" : "-");

				    printf("\t%lo", info.st_nlink);
				    printf("\t%s", group->gr_name);
				    printf("\t%s", user->pw_name);
				    printf("\t%ld", info.st_size);
				   	for (int i = 0; i < 12; ++i) {
				   		if (mtime->tm_mon == i){
	    					printf("\t%s ", months[i]);
	    				}
					}
				    printf("\t%s", timeString);
				    printf("\t%s", actual->d_name);
	   				printf("\n");
	   			}
    		}
    		printf("Total: %i\n",s/2);
    	}
    	
    	else {
    		// Caso ls -i
    		if (strcmp(args[1], builtin_str_ls[0]) == 0){
	  
	   			struct passwd *user;
	    		struct group *group;
	    		struct tm * mtime;

	    		char timeString[9];
	  
	   			DIR *repertorio = opendir (".");

	    		struct stat info; // Stat

	    		struct dirent *actual; //Archivo actual

	    		int s = 0;

	    		while ((actual = readdir(repertorio))){
	        		stat(actual->d_name, &info); // Se consulta el archivo actual y se almacena en info
	        		user = getpwuid(info.st_uid); //Database for the user
	       	 		group = getgrgid(info.st_gid); //Database for the group
	        		time_t current_time= info.st_mtime;
	        		mtime = localtime(&current_time);
	        		strftime(timeString,9,"%d %H:%M", mtime);
	 				
	 				// Ignoramos . y ..
	 				if ((strcmp(actual->d_name, "..") != 0) && (strcmp(actual->d_name, ".") != 0)){
	 					
	 					printf("%i\t", info.st_ino);
		 				s += info.st_blocks;

					    printf( (S_ISDIR(info.st_mode)) ? "d" : "-");
					    printf( (info.st_mode & S_IRUSR) ? "r" : "-");
					    printf( (info.st_mode & S_IWUSR) ? "w" : "-");
					    printf( (info.st_mode & S_IXUSR) ? "x" : "-");
					    printf( (info.st_mode & S_IRGRP) ? "r" : "-");
					    printf( (info.st_mode & S_IWGRP) ? "w" : "-");
					    printf( (info.st_mode & S_IXGRP) ? "x" : "-");
					    printf( (info.st_mode & S_IROTH) ? "r" : "-");
					    printf( (info.st_mode & S_IWOTH) ? "w" : "-");
					    printf( (info.st_mode & S_IXOTH) ? "x" : "-");

					    printf("\t%lo", info.st_nlink);
					    printf("\t%s", group->gr_name);
					    printf("\t%s", user->pw_name);
					    printf("\t%ld", info.st_size);
					   	for (int i = 0; i < 12; ++i) {
					   		if (mtime->tm_mon == i){
		    					printf("\t%s ", months[i]);
		    				}
						}
					    printf("\t%s", timeString);
					    printf("\t%s", actual->d_name);
		   				printf("\n");
		   			}
	    		}
	    		printf("Total: %i\n",s/2);
    		}

    		// Caso ls -G
    		else if (strcmp(args[1], builtin_str_ls[1]) == 0){

    			struct passwd *user;
	    		struct group *group;
	    		struct tm * mtime;

	    		char timeString[9];
	  
	   			DIR *repertorio = opendir (".");

	    		struct stat info; // Stat

	    		struct dirent *actual; //Archivo actual

	    		int s = 0;

	    		while ((actual = readdir(repertorio))){
	        		stat(actual->d_name, &info); // Se consulta el archivo actual y se almacena en info
	        		user = getpwuid(info.st_uid); //Database for the user
	       	 		group = getgrgid(info.st_gid); //Database for the group
	        		time_t current_time= info.st_mtime;
	        		mtime = localtime(&current_time);
	        		strftime(timeString,9,"%d %H:%M", mtime);
	 				
	 				// Ignoramos . y ..
	 				if ((strcmp(actual->d_name, "..") != 0) && (strcmp(actual->d_name, ".") != 0)){
	 					
		 				s += info.st_blocks;

					    printf( (S_ISDIR(info.st_mode)) ? "d" : "-");
					    printf( (info.st_mode & S_IRUSR) ? "r" : "-");
					    printf( (info.st_mode & S_IWUSR) ? "w" : "-");
					    printf( (info.st_mode & S_IXUSR) ? "x" : "-");
					    printf( (info.st_mode & S_IRGRP) ? "r" : "-");
					    printf( (info.st_mode & S_IWGRP) ? "w" : "-");
					    printf( (info.st_mode & S_IXGRP) ? "x" : "-");
					    printf( (info.st_mode & S_IROTH) ? "r" : "-");
					    printf( (info.st_mode & S_IWOTH) ? "w" : "-");
					    printf( (info.st_mode & S_IXOTH) ? "x" : "-");

					    printf("\t%lo", info.st_nlink);
					    printf("\t%s", user->pw_name);
					    printf("\t%ld", info.st_size);
					   	for (int i = 0; i < 12; ++i) {
					   		if (mtime->tm_mon == i){
		    					printf("\t%s ", months[i]);
		    				}
						}
					    printf("\t%s", timeString);
					    printf("\t%s", actual->d_name);
		   				printf("\n");
		   			}
	    		}
	    		printf("Total: %i\n",s/2);

    		}

    		// Caso ls -g
    		else if (strcmp(args[1], builtin_str_ls[2]) == 0){

    			struct passwd *user;
	    		struct group *group;
	    		struct tm * mtime;

	    		char timeString[9];
	  
	   			DIR *repertorio = opendir (".");

	    		struct stat info; // Stat

	    		struct dirent *actual; //Archivo actual

	    		int s = 0;

	    		while ((actual = readdir(repertorio))){
	        		stat(actual->d_name, &info); // Se consulta el archivo actual y se almacena en info
	        		user = getpwuid(info.st_uid); //Database for the user
	       	 		group = getgrgid(info.st_gid); //Database for the group
	        		time_t current_time= info.st_mtime;
	        		mtime = localtime(&current_time);
	        		strftime(timeString,9,"%d %H:%M", mtime);
	 				
	 				// Ignoramos . y ..
	 				if ((strcmp(actual->d_name, "..") != 0) && (strcmp(actual->d_name, ".") != 0)){
	 					
		 				s += info.st_blocks;

					    printf( (S_ISDIR(info.st_mode)) ? "d" : "-");
					    printf( (info.st_mode & S_IRUSR) ? "r" : "-");
					    printf( (info.st_mode & S_IWUSR) ? "w" : "-");
					    printf( (info.st_mode & S_IXUSR) ? "x" : "-");
					    printf( (info.st_mode & S_IRGRP) ? "r" : "-");
					    printf( (info.st_mode & S_IWGRP) ? "w" : "-");
					    printf( (info.st_mode & S_IXGRP) ? "x" : "-");
					    printf( (info.st_mode & S_IROTH) ? "r" : "-");
					    printf( (info.st_mode & S_IWOTH) ? "w" : "-");
					    printf( (info.st_mode & S_IXOTH) ? "x" : "-");

					    printf("\t%lo", info.st_nlink);
					    printf("\t%s", group->gr_name);
					    printf("\t%ld", info.st_size);
					   	for (int i = 0; i < 12; ++i) {
					   		if (mtime->tm_mon == i){
		    					printf("\t%s ", months[i]);
		    				}
						}
					    printf("\t%s", timeString);
					    printf("\t%s", actual->d_name);
		   				printf("\n");
		   			}
	    		}
	    		printf("Total: %i\n",s/2);

    		}


    		// Caso ls -h
    		else if (strcmp(args[1], builtin_str_ls[3]) == 0){

    			struct passwd *user;
	    		struct group *group;
	    		struct tm * mtime;

	    		char timeString[9];
	  
	   			DIR *repertorio = opendir (".");

	    		struct stat info; // Stat

	    		struct dirent *actual; //Archivo actual

	    		int s = 0;

	    		while ((actual = readdir(repertorio))){
	        		stat(actual->d_name, &info); // Se consulta el archivo actual y se almacena en info
	        		user = getpwuid(info.st_uid); //Database for the user
	       	 		group = getgrgid(info.st_gid); //Database for the group
	        		time_t current_time= info.st_mtime;
	        		mtime = localtime(&current_time);
	        		strftime(timeString,9,"%d %H:%M", mtime);
	 				
	 				// Ignoramos . y ..
	 				if ((strcmp(actual->d_name, "..") != 0) && (strcmp(actual->d_name, ".") != 0)){
	 				
		 				s += info.st_blocks;

					    printf( (S_ISDIR(info.st_mode)) ? "d" : "-");
					    printf( (info.st_mode & S_IRUSR) ? "r" : "-");
					    printf( (info.st_mode & S_IWUSR) ? "w" : "-");
					    printf( (info.st_mode & S_IXUSR) ? "x" : "-");
					    printf( (info.st_mode & S_IRGRP) ? "r" : "-");
					    printf( (info.st_mode & S_IWGRP) ? "w" : "-");
					    printf( (info.st_mode & S_IXGRP) ? "x" : "-");
					    printf( (info.st_mode & S_IROTH) ? "r" : "-");
					    printf( (info.st_mode & S_IWOTH) ? "w" : "-");
					    printf( (info.st_mode & S_IXOTH) ? "x" : "-");

					    printf("\t%lo", info.st_nlink);
					    printf("\t%s", group->gr_name);
					    printf("\t%s", user->pw_name);


					    float size = (float) info.st_size;
					    size /= 1024;
					    char units = 'K';
					    if (size >= 800) {
					    	size /= 1024;
					    	units = 'M';
					    }
					    printf("\t%.1f%c", size, units);



					   	for (int i = 0; i < 12; ++i) {
					   		if (mtime->tm_mon == i){
		    					printf("\t%s ", months[i]);
		    				}
						}
					    printf("\t%s", timeString);
					    printf("\t%s", actual->d_name);
		   				printf("\n");
		   			}
	    		}
	    		printf("Total: %i\n",s/2);
    		}

    		// Caso ls -R
    		else if (strcmp(args[1], builtin_str_ls[4]) == 0){

    			struct passwd *user;
	    		struct group *group;
	    		struct tm * mtime;

	    		char timeString[9];
	  
	  			char * dir_name = malloc(sizeof(char) * 100);
	   			DIR *repertorio = opendir (".");

	    		struct stat info; // Stat

	    		struct dirent *actual; //Archivo actual

	    		int s = 0;

	    		// List to save the directories found
	    		struct dirent ** directories = malloc(sizeof(struct dirent*) * 100);
	    		int num_directories = 0;

	    		while ((actual = readdir(repertorio))){
	        		stat(actual->d_name, &info); // Se consulta el archivo actual y se almacena en info
	        		user = getpwuid(info.st_uid); //Database for the user
	       	 		group = getgrgid(info.st_gid); //Database for the group
	        		time_t current_time= info.st_mtime;
	        		mtime = localtime(&current_time);
	        		strftime(timeString,9,"%d %H:%M", mtime);
	 				
	 				// Ignoramos . y ..
	 				if ((strcmp(actual->d_name, "..") != 0) && (strcmp(actual->d_name, ".") != 0)){
	 				
		 				s += info.st_blocks;

					    printf( (S_ISDIR(info.st_mode)) ? "d" : "-");
					    printf( (info.st_mode & S_IRUSR) ? "r" : "-");
					    printf( (info.st_mode & S_IWUSR) ? "w" : "-");
					    printf( (info.st_mode & S_IXUSR) ? "x" : "-");
					    printf( (info.st_mode & S_IRGRP) ? "r" : "-");
					    printf( (info.st_mode & S_IWGRP) ? "w" : "-");
					    printf( (info.st_mode & S_IXGRP) ? "x" : "-");
					    printf( (info.st_mode & S_IROTH) ? "r" : "-");
					    printf( (info.st_mode & S_IWOTH) ? "w" : "-");
					    printf( (info.st_mode & S_IXOTH) ? "x" : "-");

					    printf("\t%lo", info.st_nlink);
					    printf("\t%s", group->gr_name);
					    printf("\t%s", user->pw_name);
					    printf("\t%ld", info.st_size);
					   	for (int i = 0; i < 12; ++i) {
					   		if (mtime->tm_mon == i){
		    					printf("\t%s ", months[i]);
		    				}
						}
					    printf("\t%s", timeString);
					    printf("\t%s", actual->d_name);
		   				printf("\n");

		   				if (info.st_nlink>1){
		   					directories[num_directories++] = actual;
		   				}
		   			}
	    		}
	    		printf("Total: %i\n",s/2);
	    		for (int i = 0; i < num_directories; i++) {
	    			s = 0;
	    			actual = directories[i];
	    			dir_name = "./\0";
	    			char *c = strconcat(dir_name, actual->d_name);
	    			printf("\n%s:\n", c);
	    			fflush(stdout);
		   			repertorio = opendir(c);
		   			while (actual = readdir(repertorio)) {
		   				stat(actual->d_name, &info); // Se consulta el archivo actual y se almacena en info
		        		user = getpwuid(info.st_uid); //Database for the user
		       	 		group = getgrgid(info.st_gid); //Database for the group
		        		time_t current_time= info.st_mtime;
		        		mtime = localtime(&current_time);
		        		strftime(timeString,9,"%d %H:%M", mtime);
		 				
		 				// Ignoramos . y ..
		 				if ((strcmp(actual->d_name, "..") != 0) && (strcmp(actual->d_name, ".") != 0)){
		 				
			 				s += info.st_blocks;

						    printf( (S_ISDIR(info.st_mode)) ? "d" : "-");
						    printf( (info.st_mode & S_IRUSR) ? "r" : "-");
						    printf( (info.st_mode & S_IWUSR) ? "w" : "-");
						    printf( (info.st_mode & S_IXUSR) ? "x" : "-");
						    printf( (info.st_mode & S_IRGRP) ? "r" : "-");
						    printf( (info.st_mode & S_IWGRP) ? "w" : "-");
						    printf( (info.st_mode & S_IXGRP) ? "x" : "-");
						    printf( (info.st_mode & S_IROTH) ? "r" : "-");
						    printf( (info.st_mode & S_IWOTH) ? "w" : "-");
						    printf( (info.st_mode & S_IXOTH) ? "x" : "-");

						    printf("\t%lo", info.st_nlink);
						    printf("\t%s", group->gr_name);
						    printf("\t%s", user->pw_name);
						    printf("\t%ld", info.st_size);
						   	for (int i = 0; i < 12; ++i) {
						   		if (mtime->tm_mon == i){
			    					printf("\t%s ", months[i]);
			    				}
							}
						    printf("\t%s", timeString);
						    printf("\t%s", actual->d_name);
			   				printf("\n");
			   			}

		   			}
		   			printf("Total: %i\n",s/8);
		   			free(c);
	    		}
    		}

    		else{
    			printf("Comando no valido\n");
    		}
    	}
    	
    }

    // Caso grep
    else if (strcmp(args[0], builtin_str[1]) == 0){


    	if (args[1] == NULL){
    		printf("Incompleto\n");

    	}

    	// -i
    	else if (strcmp(args[1], builtin_str_grep[0]) == 0){
    		if (args[2] == NULL || args[3] == NULL) {
    			printf("Comando incompleto\n");
    		}
    		else {
    			char * to_match = malloc(sizeof(char) * (strlen(args[2]) + 1));
    			to_match = args[2];
    			int match_len = strlen(to_match);
    			fflush(stdout);
    			for (int i  = 0; i < strlen(to_match); i++) {
    				to_match[i] = tolower(to_match[i]);
    			}

	    		FILE* f = fopen(args[3], "r");

	    		if (f == NULL) {
	    			printf("Error abriendo archivo: %s\n", args[3]);
	    		}

	    		else {
    			
	    			char * line = malloc(sizeof(char) * 250);
	    			for(line = fgets(line, 250, f); feof(f)==0; line = fgets(line, 250, f)) {
	    				for (int i = 0; i < strlen(line); i++) {
	    					line[i] = tolower(line[i]);
	    				}
	    				
	    				int match = 1;
	    				for(int i = 0; i < strlen(line); i++) {

	    					// Verificamos que todavia nos quedan caracteres suficientes para hacer un match
	    					if (i + strlen(to_match) > strlen(line)) break;
	    					
	    					if (line[i] == to_match[0]) {
	    						// Inicializamos el match en cierto
	    						match = 1;
	    						for(int j = 1; j < strlen(to_match) && match != 0; j++) {
	    							// Si un caracter es distinto descartamos el match
	    							if (line[i+j] != to_match[j]) match = 0;
	    						}
	    						// Vemos si tenemos match
	    						if (match == 1) {
	    							printf("%s", line);
	    						}
	    					}
	    				}
	    			}
	    		}
    		}
    		

    	}

    	// -v
    	else if (strcmp(args[1], builtin_str_grep[1]) == 0){
    		if (args[2] == NULL || args[3] == NULL) {
    			printf("Comando incompleto\n");
    		}
    		else {
    			char * to_match = malloc(sizeof(char) * (strlen(args[2]) + 1));
    			to_match = args[2];
    			int match_len = strlen(to_match);
    			fflush(stdout);

	    		FILE* f = fopen(args[3], "r");

	    		if (f == NULL) {
	    			printf("Error abriendo archivo: %s\n", args[3]);
	    		}

	    		else {
    			
	    			char * line = malloc(sizeof(char) * 250);
	    			for(line = fgets(line, 250, f); feof(f)==0; line = fgets(line, 250, f)) {
	    				
	    				int match = 0;
	    				for(int i = 0; i < strlen(line); i++) {

	    					// Verificamos que todavia nos quedan caracteres suficientes para hacer un match
	    					if (i + strlen(to_match) > strlen(line)) break;
	    					
	    					if (line[i] == to_match[0]) {
	    						// Inicializamos el match en cierto
	    						match = 1;
	    						for(int j = 1; j < strlen(to_match) && match != 0; j++) {
	    							// Si un caracter es distinto descartamos el match
	    							if (line[i+j] != to_match[j]) match = 0;
	    						}
	    					}
	    					if (match == 1) break;
	    				}
	    				if (match == 0) printf("%s", line);
	    			}
	    			free(line);
	    		}
    		}
    	}

    	else{
    		if (args[1] == NULL || args[2] == NULL) {
    			printf("Comando incompleto\n");
    		}
    		else {
    			char * to_match = malloc(sizeof(char) * (strlen(args[1]) + 1));
    			to_match = args[1];
    			int match_len = strlen(to_match);
    			fflush(stdout);

	    		FILE* f = fopen(args[2], "r");

	    		if (f == NULL) {
	    			printf("Error abriendo archivo: %s\n", args[3]);
	    		}

	    		else {
    			
	    			char * line = malloc(sizeof(char) * 250);
	    			for(line = fgets(line, 250, f); feof(f)==0; line = fgets(line, 250, f)) {
	    				
	    				int match = 0;
	    				for(int i = 0; i < strlen(line); i++) {

	    					// Verificamos que todavia nos quedan caracteres suficientes para hacer un match
	    					if (i + strlen(to_match) > strlen(line)) break;
	    					
	    					if (line[i] == to_match[0]) {
	    						// Inicializamos el match en cierto
	    						match = 1;
	    						for(int j = 1; j < strlen(to_match) && match != 0; j++) {
	    							// Si un caracter es distinto descartamos el match
	    							if (line[i+j] != to_match[j]) match = 0;
	    						}
	    						if (match == 1) {
	    							printf("%s", line);
	    						}
	    					}
	    				}
	    			}
	    			free(line);
	    		}
    		}
    	}


    }

    // Caso chmod
    else if (strcmp(args[0], builtin_str[2]) == 0){
    	if (args[1] == NULL || args[2] == NULL) {
    		printf("Not enought params\n");
    	}
    	else {
    		struct stat info;
    		mode_t file_mode;
    		stat(args[2], &info);
    		file_mode = info.st_mode & 0777;
    		// Chequea si lo que se esta es agregando permisos o quitando
    		int allow_permission;
    		if (args[1][0] == '+') {
    			allow_permission = 1;
    		}
    		else if (args[1][0] == '-') {
    			allow_permission = 0;
    		}
    		else {
    			printf("Invalid parameters\n");
    			continue;
    		}
    		for (int i = 1; i < strlen(args[1]); i++) {
    			char c = args[1][i];
    			if (c == 'r') {
    				if (allow_permission == 1) {
    					file_mode |= S_IRUSR | S_IRGRP | S_IROTH;
    				}
    				else {
    					file_mode &= ~(S_IRUSR | S_IRGRP | S_IROTH);
    				}
    			}
    			else if (c == 'w') {
    				if (allow_permission == 1) {
    					file_mode |= S_IWUSR | S_IWGRP | S_IWOTH;	
    				}
    				else {
    					file_mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);	
    				}
    			}
    			else if (c == 'x') {
    				if (allow_permission == 1) {
    					file_mode |= S_IXUSR | S_IXGRP | S_IXOTH;
    				}
    				else {
    					file_mode &= ~(S_IXUSR | S_IXGRP | S_IROTH);
    				}
    			}
    			else if (c == '+') {
    				allow_permission = 1;
    			}
    			else if (c == '-') {
    				allow_permission = 0;
    			}
    		}
    		chmod(args[2], file_mode);
    	}
    }

    else {
    	printf("Comando no valido\n");
    }
    free(line);
    for(int i = 0; i < num_args; i++) {
    	free(args[i]);
    }
    free(args);
   
  } while (1);
}


int main(int argc, char **argv){

	// Lee	r un archivo
	if(argv[1]!=NULL){
	}

	loop();
	return 0;
}