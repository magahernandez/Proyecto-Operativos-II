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

const char * months[12] = {"January", "February", "March", "April", "May", "June", "July", 
							"August", "September", "October", "November", "December"};

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
  *num_args = position;

  // Imprimir todos los tokens
  for (int i = 0; i < position; i++) {
  	printf("%s\n", tokens[i]);
  }
  printf("Num args: %d\n", *num_args);
  // exit(0);
  return tokens;
}


int _ls(char * flag, FILE * output)
{
  // Caso "ls"
    	if (flag == NULL){

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

				    fprintf( output, (S_ISDIR(info.st_mode)) ? "d" : "-");
				    fprintf( output, (info.st_mode & S_IRUSR) ? "r" : "-");
				    fprintf( output, (info.st_mode & S_IWUSR) ? "w" : "-");
				    fprintf( output, (info.st_mode & S_IXUSR) ? "x" : "-");
				    fprintf( output, (info.st_mode & S_IRGRP) ? "r" : "-");
				    fprintf( output, (info.st_mode & S_IWGRP) ? "w" : "-");
				    fprintf( output, (info.st_mode & S_IXGRP) ? "x" : "-");
				    fprintf( output, (info.st_mode & S_IROTH) ? "r" : "-");
				    fprintf( output, (info.st_mode & S_IWOTH) ? "w" : "-");
				    fprintf( output, (info.st_mode & S_IXOTH) ? "x" : "-");

				    fprintf(output, "\t%lo", info.st_nlink);
				    fprintf(output, "\t%s", group->gr_name);
				    fprintf(output, "\t%s", user->pw_name);
				    fprintf(output, "\t%ld", info.st_size);
				   	for (int i = 0; i < 12; ++i) {
				   		if (mtime->tm_mon == i){
	    					fprintf(output, "\t%s ", months[i]);
	    				}
					}
				    fprintf(output, "\t%s", timeString);
				    fprintf(output, "\t%s", actual->d_name);
	   				fprintf(output, "\n");
	   			}
    		}
    		fprintf(output, "Total: %i\n",s/2);
    	}
    	
    	else {
    		// Caso ls -i
    		if (strcmp(flag, builtin_str_ls[0]) == 0){
	  
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
	 					
	 					fprintf(output,"%i\t", info.st_ino);
		 				s += info.st_blocks;

					    fprintf( output,(S_ISDIR(info.st_mode)) ? "d" : "-");
					    fprintf( output,(info.st_mode & S_IRUSR) ? "r" : "-");
					    fprintf( output,(info.st_mode & S_IWUSR) ? "w" : "-");
					    fprintf( output,(info.st_mode & S_IXUSR) ? "x" : "-");
					    fprintf( output,(info.st_mode & S_IRGRP) ? "r" : "-");
					    fprintf( output,(info.st_mode & S_IWGRP) ? "w" : "-");
					    fprintf( output,(info.st_mode & S_IXGRP) ? "x" : "-");
					    fprintf( output,(info.st_mode & S_IROTH) ? "r" : "-");
					    fprintf( output,(info.st_mode & S_IWOTH) ? "w" : "-");
					    fprintf( output,(info.st_mode & S_IXOTH) ? "x" : "-");

					    fprintf(output,"\t%lo", info.st_nlink);
					    fprintf(output,"\t%s", group->gr_name);
					    fprintf(output,"\t%s", user->pw_name);
					    fprintf(output,"\t%ld", info.st_size);
					   	for (int i = 0; i < 12; ++i) {
					   		if (mtime->tm_mon == i){
		    					fprintf(output,"\t%s ", months[i]);
		    				}
						}
					    fprintf(output,"\t%s", timeString);
					    fprintf(output,"\t%s", actual->d_name);
		   				fprintf(output,"\n");
		   			}
	    		}
	    		fprintf(output,"Total: %i\n",s/2);
    		}

    		// Caso ls -G
    		else if (strcmp(flag, builtin_str_ls[1]) == 0){

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

					    fprintf(output, (S_ISDIR(info.st_mode)) ? "d" : "-");
					    fprintf( output,(info.st_mode & S_IRUSR) ? "r" : "-");
					    fprintf( output,(info.st_mode & S_IWUSR) ? "w" : "-");
					    fprintf( output,(info.st_mode & S_IXUSR) ? "x" : "-");
					    fprintf( output,(info.st_mode & S_IRGRP) ? "r" : "-");
					    fprintf( output, (info.st_mode & S_IWGRP) ? "w" : "-");
					    fprintf( output,(info.st_mode & S_IXGRP) ? "x" : "-");
					    fprintf( output,(info.st_mode & S_IROTH) ? "r" : "-");
					    fprintf( output,(info.st_mode & S_IWOTH) ? "w" : "-");
					    fprintf( output,(info.st_mode & S_IXOTH) ? "x" : "-");

					    fprintf(output,"\t%lo", info.st_nlink);
					    fprintf(output,"\t%s", user->pw_name);
					    fprintf(output,"\t%ld", info.st_size);
					   	for (int i = 0; i < 12; ++i) {
					   		if (mtime->tm_mon == i){
		    					fprintf(output,"\t%s ", months[i]);
		    				}
						}
					    fprintf(output,"\t%s", timeString);
					    fprintf(output,"\t%s", actual->d_name);
		   				fprintf(output,"\n");
		   			}
	    		}
	    		fprintf(output,"Total: %i\n",s/2);

    		}

    		// Caso ls -g
    		else if (strcmp(flag, builtin_str_ls[2]) == 0){

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

					    fprintf(output, (S_ISDIR(info.st_mode)) ? "d" : "-");
					    fprintf( output,(info.st_mode & S_IRUSR) ? "r" : "-");
					    fprintf( output,(info.st_mode & S_IWUSR) ? "w" : "-");
					    fprintf( output,(info.st_mode & S_IXUSR) ? "x" : "-");
					    fprintf( output,(info.st_mode & S_IRGRP) ? "r" : "-");
					    fprintf( output,(info.st_mode & S_IWGRP) ? "w" : "-");
					    fprintf( output,(info.st_mode & S_IXGRP) ? "x" : "-");
					    fprintf( output,(info.st_mode & S_IROTH) ? "r" : "-");
					    fprintf( output,(info.st_mode & S_IWOTH) ? "w" : "-");
					    fprintf( output,(info.st_mode & S_IXOTH) ? "x" : "-");

					    fprintf(output,"\t%lo", info.st_nlink);
					    fprintf(output,"\t%s", group->gr_name);
					    fprintf(output,"\t%ld", info.st_size);
					   	for (int i = 0; i < 12; ++i) {
					   		if (mtime->tm_mon == i){
		    					fprintf(output,"\t%s ", months[i]);
		    				}
						}
					    fprintf(output,"\t%s", timeString);
					    fprintf(output,"\t%s", actual->d_name);
		   				fprintf(output,"\n");
		   			}
	    		}
	    		fprintf(output,"Total: %i\n",s/2);

    		}


    		// Caso ls -h
    		else if (strcmp(flag, builtin_str_ls[3]) == 0){

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

					    fprintf(output, (S_ISDIR(info.st_mode)) ? "d" : "-");
					    fprintf( output,(info.st_mode & S_IRUSR) ? "r" : "-");
					    fprintf( output,(info.st_mode & S_IWUSR) ? "w" : "-");
					    fprintf( output,(info.st_mode & S_IXUSR) ? "x" : "-");
					    fprintf( output,(info.st_mode & S_IRGRP) ? "r" : "-");
					    fprintf( output,(info.st_mode & S_IWGRP) ? "w" : "-");
					    fprintf( output,(info.st_mode & S_IXGRP) ? "x" : "-");
					    fprintf( output,(info.st_mode & S_IROTH) ? "r" : "-");
					    fprintf( output,(info.st_mode & S_IWOTH) ? "w" : "-");
					    fprintf( output,(info.st_mode & S_IXOTH) ? "x" : "-");

					    fprintf(output,"\t%lo", info.st_nlink);
					    fprintf(output,"\t%s", group->gr_name);
					    fprintf(output,"\t%s", user->pw_name);


					    float size = (float) info.st_size;
					    size /= 1024;
					    char units = 'K';
					    if (size >= 800) {
					    	size /= 1024;
					    	units = 'M';
					    }
					    fprintf(output,"\t%.1f%c", size, units);



					   	for (int i = 0; i < 12; ++i) {
					   		if (mtime->tm_mon == i){
		    					fprintf(output,"\t%s ", months[i]);
		    				}
						}
					    fprintf(output,"\t%s", timeString);
					    fprintf(output,"\t%s", actual->d_name);
		   				fprintf(output,"\n");
		   			}
	    		}
	    		fprintf(output,"Total: %i\n",s/2);
    		}

    		// Caso ls -R
    		else if (strcmp(flag, builtin_str_ls[4]) == 0){

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

					    fprintf(output, (S_ISDIR(info.st_mode)) ? "d" : "-");
					    fprintf( output,(info.st_mode & S_IRUSR) ? "r" : "-");
					    fprintf( output,(info.st_mode & S_IWUSR) ? "w" : "-");
					    fprintf( output,(info.st_mode & S_IXUSR) ? "x" : "-");
					    fprintf( output,(info.st_mode & S_IRGRP) ? "r" : "-");
					    fprintf( output,(info.st_mode & S_IWGRP) ? "w" : "-");
					    fprintf( output,(info.st_mode & S_IXGRP) ? "x" : "-");
					    fprintf( output,(info.st_mode & S_IROTH) ? "r" : "-");
					    fprintf( output,(info.st_mode & S_IWOTH) ? "w" : "-");
					    fprintf( output,(info.st_mode & S_IXOTH) ? "x" : "-");

					    fprintf(output,"\t%lo", info.st_nlink);
					    fprintf(output,"\t%s", group->gr_name);
					    fprintf(output,"\t%s", user->pw_name);
					    fprintf(output,"\t%ld", info.st_size);
					   	for (int i = 0; i < 12; ++i) {
					   		if (mtime->tm_mon == i){
		    					fprintf(output,"\t%s ", months[i]);
		    				}
						}
					    fprintf(output,"\t%s", timeString);
					    fprintf(output,"\t%s", actual->d_name);
		   				fprintf(output,"\n");

		   				if (info.st_nlink>1){
		   					directories[num_directories++] = actual;
		   				}
		   			}
	    		}
	    		fprintf(output,"Total: %i\n",s/2);
	    		for (int i = 0; i < num_directories; i++) {
	    			s = 0;
	    			actual = directories[i];
	    			dir_name = "./\0";
	    			char *c = strconcat(dir_name, actual->d_name);
	    			fprintf(output,"\n%s:\n", c);
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

						    fprintf(output, (S_ISDIR(info.st_mode)) ? "d" : "-");
						    fprintf(output, (info.st_mode & S_IRUSR) ? "r" : "-");
						    fprintf(output, (info.st_mode & S_IWUSR) ? "w" : "-");
						    fprintf(output, (info.st_mode & S_IXUSR) ? "x" : "-");
						    fprintf(output, (info.st_mode & S_IRGRP) ? "r" : "-");
						    fprintf(output, (info.st_mode & S_IWGRP) ? "w" : "-");
						    fprintf(output, (info.st_mode & S_IXGRP) ? "x" : "-");
						    fprintf(output, (info.st_mode & S_IROTH) ? "r" : "-");
						    fprintf(output, (info.st_mode & S_IWOTH) ? "w" : "-");
						    fprintf(output, (info.st_mode & S_IXOTH) ? "x" : "-");

						    fprintf(output,"\t%lo", info.st_nlink);
						    fprintf(output,"\t%s", group->gr_name);
						    fprintf(output,"\t%s", user->pw_name);
						    fprintf(output,"\t%ld", info.st_size);
						   	for (int i = 0; i < 12; ++i) {
						   		if (mtime->tm_mon == i){
			    					fprintf(output,"\t%s ", months[i]);
			    				}
							}
						    fprintf(output,"\t%s", timeString);
						    fprintf(output,"\t%s", actual->d_name);
			   				fprintf(output,"\n");
			   			}

		   			}
		   			fprintf(output,"Total: %i\n",s/8);
		   			free(c);
	    		}
    		}

    		else{
    			fprintf(output,"Comando no valido\n");
    		}
    	}
    	return 0;
}

int _grep(char *flag, char * pattern, FILE * input, FILE * output)
{
  	if (flag == NULL) {
    			char * to_match = malloc(sizeof(char) * (strlen(pattern) + 1));
    			to_match = pattern;
    			int match_len = strlen(to_match);

    				printf("Preparamos para leer el archivo\n");
	    			char * line = malloc(sizeof(char) * 250);
	    			for(line = fgets(line, 250, input); feof(input)==0; line = fgets(line, 250, input)) {
	    				//printf("nueva linea: %s\n", line);
	    				int match = 0;
	    				for(int i = 0; i < strlen(line); i++) {
	    					//printf("bye\n");
	    					fflush(stdout);
	    					// Verificamos que todavia nos quedan caracteres suficientes para hacer un match
	    					if (i + strlen(to_match) > strlen(line)) break;
	    					
	    					if (line[i] == to_match[0]) {
	    						// Inicializamos el match en cierto
	    						match = 1;
	    						for(int j = 1; j < strlen(to_match) && match != 0; j++) {
	    							//printf("Viendo caracter\n");
	    							// Si un caracter es distinto descartamos el match
	    							if (line[i+j] != to_match[j]) match = 0;
	    						}
	    						if (match == 1) {
	    							fprintf(output,"%s", line);
	    							break;
	    						}
	    					}
	    			}
	    			
	    		}free(line);
	    	//printf("algo");
	    	//fflush(stdout);
	    	}

    	// -i
    	else if (strcmp(flag, builtin_str_grep[0]) == 0){

    			//printf("Entro en el -i\n");
    		
    			char * to_match = malloc(sizeof(char) * (strlen(pattern) + 1));
    			to_match = pattern;
    			int match_len = strlen(to_match);
    			fflush(stdout);
    			for (int i  = 0; i < strlen(to_match); i++) {
    				to_match[i] = tolower(to_match[i]);
    			}   
    			//printf("Paso el primer for\n"); 			
	    			char * line = malloc(sizeof(char) * 250);

	    			for(line = fgets(line, 250, input); feof(input)==0; line = fgets(line, 250, input)) {
	    				//printf("input");
	    				for (int i = 0; i < strlen(line); i++) {
	    					line[i] = tolower(line[i]);
	    				}
	    				//printf("paso el tolower\n");
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
	    							fprintf(output, "%s", line);
	    						}
	    					}
	    				}
	    			}
	    			free(line);
    		

    	}

    	// -v
    	else if (strcmp(flag, builtin_str_grep[1]) == 0){
    		
    		
    			char * to_match = malloc(sizeof(char) * (strlen(pattern) + 1));
    			to_match = pattern;
    			int match_len = strlen(to_match);
    			fflush(stdout);

	    			char * line = malloc(sizeof(char) * 250);
	    			for(line = fgets(line, 250, input); feof(input)==0; line = fgets(line, 250, input)) {
	    				
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
	    				if (match == 0) fprintf(output,"%s", line);
	    			}
	    			free(line);
    		}

    		printf("algo");
	    	fflush(stdout);
    		return 0;
}

int _chmod(char *flag, char * input)
{	

  			// args[2] = input
			// args[1] = flag
    
    		struct stat info;
    		mode_t file_mode;
    		stat(input, &info);
    		file_mode = info.st_mode & 0777;
    		// Chequea si lo que se esta es agregando permisos o quitando
    		int allow_permission;
    		if (flag[0] == '+') {
    			allow_permission = 1;
    		}
    		else if (flag[0] == '-') {
    			allow_permission = 0;
    		}
    		else {
    			printf("Invalid parameters\n");
    			return 0;
    		}
    		for (int i = 1; i < strlen(flag); i++) {
    			char c = flag[i];
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
    		chmod(input, file_mode);
    
}

/*
int (*builtin_func[]) (char **) = {
  &_ls,
  &_grep,
  &_chmod};
*/



void loop(void){
  char *line;
  
  
  do
  {	
    printf("> ");
    line = read_line();
    // Tokenizamos la linea de input
  	int * n_tokens = malloc(sizeof(int));  
    char **args = split_line(line, n_tokens);
    // Realizamos el plan de ejecucion

    FILE * input, * output, * foutput;

    int *n_input = malloc(sizeof(int));
    char * input_name = malloc(sizeof(char) * 10);
    *n_input = 0;

    // Buscamos operador '>'
    for (int i = 0; i < *n_tokens; i++) {
    	if (args[i][0] == '>') {
    		foutput = fopen(args[i+1], "r+");
    		printf("Opened file for final output: %s\n", args[i+1]);
    	}
    }

    // Manejador de operador '|'
    input = NULL;
    
    for (int i = 0; i < *n_tokens; i++) {
    	// La salida es stdout a menos que se especifique otra salida
    	output = stdout;
    	printf("Next instruction: %s\n", args[i]);

    	////////////////// Caso ls
    	if (strcmp(args[i], builtin_str[0]) == 0){
    		printf("hola\n");
    		// Revisamos si hay flags
    		if (i + 1 < *n_tokens) {
    			if (args[i+1][0] == '-') {
    				// Revisamos si hay |
    				if (i + 2 < *n_tokens) {
    					if (args[i+2][0] == '|') {
    						*n_input = (*n_input + 1) % 2;
    						sprintf(input_name, "%d.txt", *n_input);
    						output = fopen(input_name, "w+");
    						_ls(args[i+1], output);
    						input = output;
    						fseek(input, 0, SEEK_SET);
    						i += 2;
    					}
    					else if (args[i+2][0] == '>') {

    						_ls(args[i+1], foutput);
    					}
    					else {
    						_ls(args[i+1], output);
    						i += 1;
    					}
    				}
    				else {
    					_ls(args[i+1], output);
    					i += 1;
    				}
    			}
    			// No hay flags, pero revisamos si hay operador |
    			else if (args[i+1][0] == '|') {
    				*n_input = (*n_input + 1) % 2;
    				sprintf(input_name, "%d.txt", *n_input);
    				output = fopen(input_name, "w+");
    				_ls(NULL, output);
    				input = output;
    				fseek(input, 0, SEEK_SET);
    				i += 1;
    			}
    			else if (args[i+1][0] == '>') {
    				printf("sos\n");

    				_ls(NULL, foutput);
    			}
    			// no hay flags ni operador |
    			else {
    				_ls(NULL, output);
    			}
    		}
    		else {
    			// no tiene flags ni |
    			_ls(NULL, output);
    		}
    	}

    	////////////////// CASO GREP
    	else if (strcmp(args[i], builtin_str[1]) == 0){
    		// Revisamos si tiene flags
    		if (i + 1 < *n_tokens) {
    			if (args[i+1][0] == '-') {
    				// Buscamos el archivo de entrada
    				// En este caso, vemos si otro programa nos dejo un input
    				if (input != NULL) {
    					// Vemos si hay operador '|'
    					if (i + 3 < *n_tokens && (args[i+3][0] == '|')) {
    						*n_input = (*n_input + 1) % 2;
    						sprintf(input_name, "%d.txt", *n_input);
    						output = fopen(input_name, "w+");
    						_grep(args[i+1], args[i+2], input, output);
    						input = output;
    						i += 3;
    					}
    					else {
    						// No hay operador |
    						if (i + 3 < *n_tokens && (args[i+3][0] == '>')) {
    							_grep(args[i+1], args[i+2], input, foutput);
    							input = NULL;
    							i += 3;
    						}
    						else {
    							_grep(args[i+1], args[i+2], input, output);
    							input = NULL;
    							i += 2;
    						}
    						
    					}
    				}
    				// El input debiera ser el siguiente argumento
    				else {
    					input = fopen(args[i+3], "r");
    					// Revisamos si hay operador |
    					if (i + 4 < *n_tokens && args[i+4][0] == '|') {
    						*n_input = (*n_input + 1) % 2;
    						sprintf(input_name, "%d.txt", *n_input);
    						output = fopen(input_name, "w+");
    						_grep(args[i+1], args[i+2], input, output);
    						input = output;
    						i += 4;
    					}
    					// Revisamos si hay operador '>'
    					else if (i + 4 < *n_tokens && args[i+4][0] == '>') {
    						_grep(args[i+1], args[i+2], input, foutput);
    						i += 4;
    					}
    					else {
    						_grep(args[i+1], args[i+2], input, output);
    						input = NULL;
    						i += 3;
    					}
    				}
    			}
    			else {
    				// Buscamos el archivo de entrada
    				// En este caso, vemos si otro programa nos dejo un input
    				if (input != NULL) {
    					printf("Nos dejaron un input\n");
    					// Vemos si hay operador '|'
    					if (i + 2 < *n_tokens && (args[i+2][0] == '|')) {
    						*n_input = (*n_input + 1) % 2;
    						sprintf(input_name, "%d.txt", *n_input);
    						output = fopen(input_name, "w+");
    						_grep(NULL, args[i+1], input, output);
    						input = output;
    						i += 2;
    					}
    					else {
    						// No hay operador |
    						if (i + 2 < *n_tokens && (args[i+2][0] == '>')) {
    							_grep(NULL, args[i+1], input, foutput);
    							input = NULL;
    							i += 2;
    						}
    						else {
    							_grep(NULL, args[i+1], input, output);
    							input = NULL;
    							i += 1;
    						}
    						
    					}
    				}
    				// El input debiera ser el siguiente argumento
    				else {
    					printf("Abrimos archivo: %s\n", args[i+2]);
    					input = fopen(args[i+2], "r");
    					if (input == NULL) {
    						printf("Error al abrir el archivo: %s\n", args[i+2]);
    						exit(1);
    					}
    					// Revisamos si hay operador |
    					if (i + 3 < *n_tokens && args[i+3][0] == '|') {
    						*n_input = (*n_input + 1) % 2;
    						sprintf(input_name, "%d.txt", *n_input);
    						output = fopen(input_name, "w+");
    						_grep(NULL, args[i+1], input, output);
    						input = output;
    						i += 3;
    					}
    					// Revisamos si hay operador '>'
    					else if (i + 3 < *n_tokens && args[i+3][0] == '>') {
    						_grep(NULL, args[i+1], input, foutput);
    						i += 3;
    					}
    					else {
    						_grep(NULL, args[i+1], input, output);
    						input = NULL;
    						i += 2;
    					}
    				}
    			}
    		}	

   		}

   		///////////////////// Caso chmod
	    else if (strcmp(args[i], builtin_str[2]) == 0){
	    	_chmod(args[1+i], args[i+2]);
	    }
    }
       
    free(line);
    for(int i = 0; i < *n_tokens; i++) {
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