
/*
 * wzip.c --- This program compresses files using the
 *            run-length encoding (RLE) algorithm.
 */

#include <stdio.h>
#include <stdlib.h>

void write_compressed(int *char_count, char *character); 

int main(int argc, char *argv[]) {
	/*
	argv[0] is the filename of the program.
	argv[argc] is always NULL.
	*/
  if (argc < 2) {
    printf("wzip: file1 [file2 ...]\n");
    exit(1);
  }

  int char_count = 0;
  char prev_char;
  char cur_char;

  for (int i = 1; i < argc; i++) {           		    //if more than one file
    FILE *fp = fopen(argv[i], "r");                         // r is for reading and writing in the opened file

    if (fp == NULL) {
      printf("wzip: cannot open file\n");
      exit(1);
    }

    while ((cur_char = (char) fgetc(fp)) > 0) {     // 'fgetc' returns single character at a time from the file and returns EOF (End Of File) at the end which is numerically =-1 and that's when we're out of the loop
      if (prev_char == cur_char) {                  // if previous character is similar to current character
        char_count++;
      } else if (char_count == 0) {             		 // if it's the first character
        char_count++;
        prev_char = cur_char;
      } else {                                			 // if previous character is not similar to current character
        write_compressed(&char_count, &prev_char);               // compress the previous character 
        char_count = 1;                            		 // increment the current new character
        prev_char = cur_char;
      }
    }
    fclose(fp);
  }

  write_compressed(&char_count, &prev_char);                    // compress the last character 

  return 0;
}

void write_compressed(int *char_count, char *character) {
	
	
  fwrite(char_count, sizeof (int), 1, stdout);             
  fwrite(character, sizeof (char), 1, stdout);          /*
	                                                      character − pointer to the character to be written.
							      size (int size),(char size) − size in bytes of each element to be written.
							      stdout − pointer to a FILE object that specifies an output stream.
							      fwrite returns the total number of elements successfully returned as a size_t object
														*/
}


