///////////////////Libraries///////////////////////
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <sys/mman.h> //Library for mmap
#include <pthread.h>
#include <sys/stat.h> //Library for struct stat
#include <sys/sysinfo.h>
#include <unistd.h>
#include <assert.h>
///////////////////////////////////////////////////



/////////////////GLOBAL VARIABLES////////////////////////
int total_threads; //Total number of threads that will be created for consumer.
int page_size; //Page size = 4096 Bytes
int num_files; //Total number of the files passed as the arguments.
int isComplete=0; //Flag needed to wakeup any sleeping threads at the end of program.
int total_pages; //required for the compressed output
int q_head=0; //Circular queue head.
int q_tail=0; //Circular queue tail.
#define q_capacity 10 //Circular queue current size. We can not have static array 
//for buf which size is given as a variable, so use define. 
int q_size=0; //Circular queue capacity.

pthread_mutex_t lock;
pthread_cond_t empty , fill;
int* pages_per_file;

/////////////////////WRAPPER FUNCTIONS////////////////////////////
// wrapper function are a way of making sure that our function calls work fine while also maintaining clean code 
void Pthread_mutex_lock(pthread_mutex_t *mutex){
int rc = pthread_mutex_lock(mutex);
assert(rc ==0);
}

void Pthread_mutex_unlock(pthread_mutex_t *mutex){
int rc = pthread_mutex_unlock(mutex);
assert(rc ==0);
}

void Pthread_create(pthread_t *thread, const pthread_attr_t * attr, void * (*start_routine)(void*), void * arg){
int rc = pthread_create(thread,attr,start_routine,arg);
assert(rc ==0);
}

void Pthread_join(pthread_t thread, void **value_ptr){
int rc=pthread_join(thread,value_ptr);
assert(rc==0);
}

void Pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t * attr){
int rc=pthread_mutex_init(mutex,attr);
	assert(rc==0);
	}
	
void Pthread_cond_init(pthread_cond_t *cond,pthread_condattr_t * attr){
int rc=pthread_cond_init(cond,attr);
	assert(rc==0);
	}
/////////////////////////////////////////////////////////

/////////////////STRUCTURES///////////////////////////////

//Contains the compressed output
struct output {
	char* data;   //containes the characters
	int* count;   //how many times each character has been repeated 
	int size;     // how many characters (important when writing the output)
}*out;

//Contains page specific data of a specific file.
struct buffer {
    char* address; //Mapping address of the file_number file + page_number page
    int file_number; //File Number
    int page_number; //Page number
    int last_page_size; //Page sized or (size_of_file)%page_size
}buf[q_capacity];

//Contains file specific data for munmap which remove memory mapping

////////////////////////////////////////////////////////

/////////////////QUEUE Functions////////////////////////

//buf is the Buffer queue. Queue capacity by default = 10
//Add at q_head index of the circular queue. 
void put(struct buffer b){
  	buf[q_head] = b; //Enqueue the buffer
  	q_head = (q_head + 1) % q_capacity;
  	q_size++;
}

//Remove from q_tail index of the circular queue.
struct buffer get(){
  	struct buffer b = buf[q_tail]; //Dequeue the buffer.
	q_tail = (q_tail + 1) % q_capacity;
  	q_size--;
  	return b;
}

////////////////////////////////////////////////////////

////////////////////////PRODUCER/////////////////////////
//Producer function to memory map the files passed as arguments.
void* producer(void *arg){
	//Step 1: Get the file.
	char** filenames = (char **)arg;
	struct stat sb;    // stat returns information about the file
	char* map; //mmap address
	int file;

	//Step 2: Open the file
	for(int i=0;i<num_files;i++){
		
		file = open(filenames[i], O_RDONLY); // It creates an open file description that refers to a file and a file descriptor that refers to that open file description
		int pages_in_file=0; //Calculates the number of pages in the file. Number of pages = Size of file / Page size.
		int last_page_size=0; //Variable required if the file is not page-alligned ie Size of File % Page size !=0
		
		if(file == -1){ // file not found
			printf("Error: File didn't open\n");
			exit(1);
		}

		//Step 3: Get the file info.
		if(fstat(file,&sb) == -1){ 
			close(file);
			printf("Error: Couldn't retrieve file stats");
			exit(1);
		}
		//check for empty files
        	if(sb.st_size==0){
               		continue;
        	}
		//Step 4: Calculate the number of pages and last page size.
		//st_size contains the size offset. 
		pages_in_file=(sb.st_size/page_size);
		//In case file is not page alligned, we'll need to assign an extra page.
		if(((double)sb.st_size/page_size)>pages_in_file){ 
			pages_in_file+=1;
			last_page_size=sb.st_size%page_size;
		}
		else{ //Page alligned
			last_page_size=page_size;
		}
		total_pages+=pages_in_file;
		pages_per_file[i]=pages_in_file;
		
		//Step 5: Map the entire file.
		map = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, file, 0); //If addr is NULL, then the kernel chooses the (page-aligned) address					
		
	
										  
		if (map == MAP_FAILED) { 
			close(file);
			printf("Error mmapping the file\n");
			exit(1);
    	}	
    	
    	//Step 6: For all the pages in file, create a Buffer type data with the relevant information for the consumer.
		for(int j=0;j<pages_in_file;j++){
			Pthread_mutex_lock(&lock);
			while(q_size==q_capacity){
			    pthread_cond_broadcast(&fill); //if it's full Wake-up all the sleeping consumer threads.
				pthread_cond_wait(&empty,&lock); //Call the consumer to start working on the queue.
			}
			Pthread_mutex_unlock(&lock);
			struct buffer temp;
			if(j==pages_in_file-1){ //Last page, might not be page-alligned
				temp.last_page_size=last_page_size;
			}
			else{
				temp.last_page_size=page_size;
			}
			temp.address=map;
			temp.page_number=j;
			temp.file_number=i;
			
			map+=page_size; //Go to next page in the memory.
			//possible race condition while changing size.
			Pthread_mutex_lock(&lock);
			put(temp);
			Pthread_mutex_unlock(&lock);
			pthread_cond_signal(&fill);
		}
		
		//Step 7: Close the file.
		close(file);
	}
	//Possible race condition at isComplete?
	isComplete=1; //When producer is done mapping.
	pthread_cond_broadcast(&fill); //Wake-up all the sleeping consumer threads.
	return 0;
}
/////////////////////////////////////////////////////////////////////////

///////////////////////////CONSUMER/////////////////////////////////////

//Compresses the buffer object.
struct output RLECompress(struct buffer temp){
	struct output compressed;    
	compressed.count=malloc(temp.last_page_size*sizeof(int)); //the size is determined by the worst case scenario (if there is no repeated consecutive characters)
	char* tempString=malloc(temp.last_page_size);  //also worst case scenario size 
	int countIndex=0;
	for(int i=0;i<temp.last_page_size;i++){
		tempString[countIndex]=temp.address[i];  // points to the first character in page
		compressed.count[countIndex]=1;
		while(i+1<temp.last_page_size && temp.address[i]==temp.address[i+1]){
			compressed.count[countIndex]++;  //increment the count until another character comes next 
			i++;
		}
		countIndex++;
	}
	compressed.size=countIndex; 
	compressed.count=realloc(compressed.count, countIndex*sizeof(int)); //resize a block of memory that was previously allocated to it's proper size
	compressed.data=realloc(tempString,countIndex); // resize a block of memory that was previously allocated to it's proper size
	return compressed;
}

//Calculates the relative output position for the buffer object.
int calculateOutputPosition(struct buffer temp){
	int position=0;
	//step 1: Find the relative file position of the buffer object.
	for(int i=0;i<temp.file_number;i++){
		position+=pages_per_file[i];
	}
	//Step 2: Now we're at the index where the particular file begins, we need
	//to find the page relative position
	position+=temp.page_number;
	return position;
}

//Called by consumer threads to start memory compression
//of the files in the queue 'buf'
void *consumer(){
	do{
		Pthread_mutex_lock(&lock);
		while(q_size==0 && isComplete==0){
		    pthread_cond_signal(&empty);
			pthread_cond_wait(&fill,&lock); //call the producer to start filling the queue.
		}
		if(isComplete==1 && q_size==0){ //If producer is done mapping and there's nothing left in the queue.
			Pthread_mutex_unlock(&lock);
			return NULL;
		}
		struct buffer temp=get();
		if(isComplete==0){
		    pthread_cond_signal(&empty);
		}	
		Pthread_mutex_unlock(&lock);
		//Output position calculation
		int position=calculateOutputPosition(temp);
		out[position]=RLECompress(temp);
	}while(!(isComplete==1 && q_size==0));
	return NULL;
}

////////////////////////////////////////////////////////////////////////

///////////////////////////Main/////////////////////////////////////////

void printOutput(){
	char* finalOutput=malloc(total_pages*page_size*(sizeof(int)+sizeof(char))); 
    char* init=finalOutput; //contains the starting point of finalOutput pointer
	for(int i=0;i<total_pages;i++){
		if(i<total_pages-1){
			if(out[i].data[out[i].size-1]==out[i+1].data[0]){ //Compare i'th output's last character with the i+1th output's first character
				out[i+1].count[0]+=out[i].count[out[i].size-1]; // merge them if they are equal
				out[i].size--;
			}
		}
		
		for(int j=0;j<out[i].size;j++){
			int num=out[i].count[j];
			char character=out[i].data[j];
			*((int*)finalOutput)=num; // cast int pointer to point to the number in order
			finalOutput+=sizeof(int); // increment the pointer by size of int to point to the next character in line
			*((char*)finalOutput)=character; //point to the character so that they are stored consecutivly in memory
            finalOutput+=sizeof(char);
		
		}
	}
	fwrite(init,finalOutput-init,1,stdout);
}

void freeMemory(){
	
	free(pages_per_file);
	for(int i=0;i<total_pages;i++){
		free(out[i].data);
		free(out[i].count);
	}
	free(out);

	
}

int main(int argc, char* argv[]){
	//Check if less than two arguments
	if(argc<2){
		printf("pzip: file1 [file2 ...]\n");
		exit(1);
	}

	//Initialize all the global Variables.
	//we took 4096 as page size but the program was running very slow,
	//started trying out with huge random values, program execution
	//decreased by atleast 1/4
	page_size = 10000000;//sysconf(_SC_PAGE_SIZE); //4096 bytes
	num_files=argc-1; //Number of files, needed for producer.
	total_threads=get_nprocs(); //Number of processes consumer threads 
	pages_per_file=malloc(sizeof(int)*num_files); //Pages per file.
	
    out=malloc(sizeof(struct output)* 512000*2); 
	//dynamically initialize the mutex lock
	Pthread_mutex_init(&lock,NULL);
	
	//dynamically initialize the conditional variables
	Pthread_cond_init(&empty,NULL);
	
	Pthread_cond_init(&fill,NULL);
	
	//Create producer thread to map all the files.
	pthread_t pid,cid[total_threads];
	Pthread_create(&pid, NULL, producer, argv+1); //argv + 1 to skip argv[0].

	//Create consumer thread to compress all the pages per file.
	for (int i = 0; i < total_threads; i++) {
        Pthread_create(&cid[i], NULL, consumer, NULL);
    }

    //Wait for producer-consumers to finish. so that you don't print output until the job is done
    for (int i = 0; i < total_threads; i++) {
        Pthread_join(cid[i], NULL);
    }
    Pthread_join(pid,NULL);
	printOutput();
	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&empty);
	pthread_cond_destroy(&fill);
	freeMemory();
	return 0;
}
//////////////////////////////////////////////////////////////////////////
