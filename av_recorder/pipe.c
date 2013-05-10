linux 下PIPE管道通信的实现（linux C语言） 
2010-11-13 10:26
学习linux 已经 有一段时间了。把之前学习中一些东西记下来，和大家分享。


/**********************************************************************
*  file name :upipe.h
*  author :hiland          pengsor(＠)gmail(dot)com
*  license : GPL
*  description : define some function and variable  
***************************************************************************/


#ifndef UPIPE_H
#define UPIPE_H

typedef void upipe_t;

upipe_t *upipe_new(void);
int upipe_delete(upipe_t *);

int upipe_read(upipe_t *, char *buf, size_t size);
int upipe_write(upipe_t *, const char *buf, size_t size);

#endif


/**********************************************************************
*  file name :upipe.c
*  author :hiland   pengsor@gmail.com
*  license : GPL
*  description : user pthread_mutex_t  & pthread_cond_t  , and support multithread .
***************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../include/upipe.h"

// #define KPS 192   /* MP3 file 's KPS value */
//#define PIPECOUNT 10    
#define PIPESIZE (192*1280)   /*  pipe  's buffer size  */

struct upipe_st {
char data[PIPESIZE];    //   buffer   array    
int head, tail;                  //  tail   :write  pos   ;   head   :read pos  
int count;                         // length of data     
pthread_mutex_t mut;      // mutex for protect cond
pthread_cond_t cond;      //   pthread_cond 
};

/*
fun_name : upipe_new
Description : create a  new pipe

*/
upipe_t *upipe_new(void)
{
struct upipe_st *newnode;
newnode = malloc(sizeof(struct upipe_st));
if (newnode==NULL) {
return NULL;
}

newnode->head=0;
newnode->tail=0;
newnode->count=0;
pthread_mutex_init(&newnode->mut, NULL);
pthread_cond_init(&newnode->cond, NULL);

return newnode;
}
/*
fcun_name :upipe_delete 
Description  : delete  a upipe 
*/

int upipe_delete(upipe_t *p)
{
struct upipe_st *ptr=p;
pthread_cond_destroy(&ptr->cond);
pthread_mutex_destroy(&ptr->mut);
free(p);
return 0;
}


/*
fun_name : read_byte    
Description : read data from upipe  by byte.
*/
static int
upipe_read_byte(struct upipe_st *ptr, char *datap)
{
if (ptr->count==0) {
return -1;
}
//    printf(". ",ptr->count);
fflush(stdout);
*datap = ptr->data[ptr->head];
ptr->count--;
ptr->head++;
if (ptr->head == PIPESIZE) {
ptr->head=0;
} 
return 0;
}
/*
fun_name :upipe_read
Decription : read data from upipe .
*/

int upipe_read(upipe_t *p, char *buf, size_t size)
{
int i, retcode;
struct upipe_st *ptr=p;

pthread_mutex_lock(&ptr->mut);
while(ptr->count < size  ) 
pthread_cond_wait(&ptr->cond, &ptr->mut);
for (i=0;i<size;++i) {
//            printf("r....");
if (upipe_read_byte(ptr, buf+i)<0) {
break;
}
}
retcode=i;
pthread_cond_broadcast(&ptr->cond);
pthread_mutex_unlock(&ptr->mut);
if (retcode == 0) {
retcode = -1;
}
return retcode;
}


/*
fun_name :write_byte
Decription : write  data into upipe  by byte..
*/

static int
upipe_write_byte(struct upipe_st *ptr, char datap)
{
if (ptr->count == PIPESIZE) {
return -1;
}
//    printf("o ",ptr->count);
fflush(stdout);
ptr->data[ptr->tail] = datap;
ptr->count++;
ptr->tail++;
if (ptr->tail == PIPESIZE) {
ptr->tail=0;
} 
return 0;
}

/*
fun_name :upipe_write   
Decription : write data into upipe .
*/

int upipe_write(upipe_t *p,  const char *buf, size_t size)
{
int i, retcode;
struct upipe_st *ptr = p;
pthread_mutex_lock(&ptr->mut);
while (PIPESIZE - ptr->count < size) 
pthread_cond_wait(&ptr->cond, &ptr->mut);
for (i=0;i<size;++i) {
//        printf("w....");
if (upipe_write_byte(ptr, buf[i])<0) {
break;
}
}
retcode = i;
pthread_cond_broadcast(&ptr->cond);
pthread_mutex_unlock(&ptr->mut);
if (retcode == 0) {
retcode = -1;
}
return retcode;

}
