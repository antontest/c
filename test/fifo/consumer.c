#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <fcntl.h>

#define PIPE_NAME "/tmp/dpfifo"
#define BUFFER_SIZE PIPE_BUF

//PIPE_BUF为limits.h定义的管道最大容量
int main(int argc,char *argv[]){
	  int pipe_fd;
	  int res;
	  int len;
	  int alllen=30;
	  char buffer[BUFFER_SIZE+1];//每次写数据用的缓冲区
	  int bytes_read=0;
	  memset(buffer,'\0',BUFFER_SIZE+1);		  

	  //打开管道,管道都是FIFO
	  printf("read process %d opening pipe !",getpid());
	  pipe_fd=open(PIPE_NAME,O_RDONLY);
	  printf("read process %d result %d\n",getpid(),pipe_fd);
	  if (pipe_fd!=-1){//发送数据
	      do{
	        res=read(pipe_fd,buffer,BUFFER_SIZE);	
	        printf("%s",buffer); 			        
	        bytes_read+=res;//res为本次写的字节数，bytes_sent为总字节数
	      } while (res<=alllen);	 
	      
	      close(pipe_fd);	
	  }	
	  else{
	  	  exit(1);
	  }
	  printf("%d bytes readed!\n",bytes_read); 	
	  unlink(PIPE_NAME);
	  return 0;
  
}
	

