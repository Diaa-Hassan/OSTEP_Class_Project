
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void search_file(char *search_key, FILE *fp);  

int main(int argc , char *argv[]) {
	/*
	argv[0] is the filename of the program.
	argv[argc] is always NULL.
	*/
	
  if (argc < 2) {                                  
    printf("wgrep: searchterm [file ...]\n");
    exit(1);
  }

  if (argc < 3) {                    			 	// only have the search word and no file to search within, so it will search in the standard input file.           
    search_file(argv[1], stdin);
  } 
  else {                             				// if more than one file .
    for (int i = 2; i < argc; i++) {
      FILE *fp = fopen(argv[i], "r"); 				//file pointer .

      if (fp == NULL) {              			    // if the file cannot be opened .
        printf("wgrep: cannot open file\n");
        exit(1);                    
      }

      search_file(argv[1], fp);     				// search for each file .

      fclose(fp);              				  	  	// close each file after search .
    }
  }
  
  return 0;
}

void search_file(char *search_key, FILE *fp) {
  char *buf = NULL;            				 	 	// to tell the function 'getline' about the stream from where to read the input from.
  size_t buf_size = 0;        			     	    // this parameter specifies the size in bytes of the block of memory pointed to.

  while (getline(&buf, &buf_size, fp) != -1) {		// getline is successful, it returns the number of characters read(including the newline). 
  
   
  
    if (strstr(buf, search_key) != NULL) {  		// 'strstr' returns a pointer points to the first character of the found search_key in buf otherwise a null.     
      printf("%s", buf); 							// print the line including the search term
    }
  }

  free(buf);       
}










