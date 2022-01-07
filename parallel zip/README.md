### Parallel Zip Project

This repository holds the source code, a helper header file and a set of test cases for a program named `pzip` that simulates the Unix `zip` command but using multitasking and threads

> Description

In this project we aim to improve the performance the previous simpler version that is `zip.c` ,taking advantage of the multitasking property we will aim to increase the compression performance.

**Threading advantages:**

1. Makes a single process faster
2. Threads don't require new address spaces.
3. Improve CPU performance and reduce the turnaround time as shown in the test run of both ```wzip``` program and ```pzip``` program:


**We use multiple concepts that we learned about in the following chapters:**

-  Intro to Threads
-  Threads API
-  Locks
-  Using Locks
-  Condition Variables

**The Main Concept:**

In the previous simple zip project we use the **RLE (run-length encoding)** compression which is a lossless data compression in which when the same data appear consecutively it is stored as a count and value not the original run of data. It is best suitable for data types that have many reoccurring sequence of the same data in it.

- *For example if we have the following data (17 bytes) to be compressed* >>>>  using RLE compression, the compressed file takes up 10 bytes and will look like this :

  ```
  Before : XAAAAAAAAFGQQQQK
  After  : X *8A F G *4Q K
  ```

**So What is the difference? **

The only difference between this implementation and the previous one is that it uses threading. Taking advantage of the multitasking property, we will aim to increase the compression performance. Unlike the simple zip implementation, we will divide the file to multiple threads and work on them separately. Cool isn't it? do you want to know how to do that? We will answer that in the next section.  



> How to Implement 

Well that is going to be a long section, ready? As a general, concept we use the POSIX thread to multitask our way through this project. Now, let’s dive into details. First of all, we must know how many threads to run so as not to use more than the CPU can handle and end up having slower performance.

 For that we use the **get_nprocs() function**. It returns the number of available processors. We set the number of consumer threads to be equal to the number of available processors. We also have one additional thread for the producer. We create the consumer/producer threads using the function : 

```
void Pthread_create(pthread_t *thread, const pthread_attr_t * attr, void * (*start_routine)(void*), void * arg)
```

which is a wrapper function for the ```pthread_create()``` function, it is used to insure a successful call of the original function while also maintaining a clean code. For that we also included all the wrapper functions in a separate **c** file so that it can be more configurable.

We also initialized the locks and conditional variables as needed.

We will explain who the consumer is and who the producer is later in the description.

To be able to implement these codes we called libraries such as :

```c
#include <sys/mman.h> //Library for mmap
#include <pthread.h>   //Library for threads
#include <sys/stat.h> //Library for struct stat
#include <assert.h> // to use assert()
```

Secondly, we divide the file itself because what will each thread work on if we did not? The file is divided into pages of a known size and each page of the file is stored in a different buffer struct, and how we link those struct together, we use a **circular Queue** but with a twist. We use our new friends the lock and conditional variable.

That is where the consumer and producer are put to action. The producer is the one who divides the file like we explained above and use the **mmap() function** to map the file to a process address space. 

```c
map = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, file, 0);
//If addr is NULL, then the kernel chooses the (page-aligned) address
		//at which to create the mapping. Source: man pages								`
```

The file now can be accessed like an array in the program. Then, it writes each page in a buffer struct BUT before it writes it sends to check if the queue is full or not. If it is full it wakes up the consumers so they can empty it so that the producer can continue writing its data. But what happens when the producer wakes up the consumers? 

Well, the consumers begin reading each buffer struct, compressing it using RLE, calculate where it should be written, then write it to the output file. If they finished their tasks and the producer hasn’t written any new information yet, **they send a signal**; “hey wake up lazy boy we have work to do”. Then, they wait until the producer sends them a signal saying “okay I am up and running it’s your turn now”. We go on like that until the whole file is compressed and written to the output file and even if we have multiple input files we finish them as well as they are passed in the ```char* argv[]``` array.

> Usage

Firstly we compile the code file as follows to link the two ***c*** files we have

``` bash
prompt> gcc -pthread -o pzip pzip.c Pthread.c
```

#### Test Cases :

To make sure our code is working perfectly we designed some test cases to test the performance of our parallel zip implementation. 

1. The first test case is a simple file containing some “a” characters 
2. The second test is to test how the program will respond to multiple files on the command line
3. The third test is that we do not pass any files to see how it will handle this error
4. The fourth test is we take the first one up a notch and we pass a file with longer lines and multi lines
5. The fifth test just shows the flaw of the RLE algorithm, we pass a file that has no repeated letter and see will it compress it? Spoiler alert, no it won’t.
6. The final test we go big and pass a very large file with some compressibility and observe the difference in size.

To run the tests we firstly have to give a running permission for the bash script `test-pzip.sh`

```bash
prompt> chmod u+x ./test-pzip.sh
prompt> ./test-pzip.sh
```

The ```pzip``` utility expression take the following form:

```sh
./pzip filename.txt > filename.z
``` 


the output will be in the following form

```bash
test 1: passed
test 2: passed
test 3: passed
test 4: passed
test 5: passed
test 6: passed
```

photos of the output :
![photo_2022-01-07_23-44-51](https://user-images.githubusercontent.com/66523119/148611860-5e961c14-0b74-4fb1-9a40-e6cafe471d14.jpg)
![photo_2022-01-07_23-44-42](https://user-images.githubusercontent.com/66523119/148612360-a6f34321-a4f5-4c96-9e7f-c269bd62e1e1.jpg)



