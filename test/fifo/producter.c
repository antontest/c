#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <fcntl.h>

#define PIPE_NAME "/tmp/dpfifo"
#define BUFFER_SIZE PIPE_BUF

//PIPE_BUF为limits.h定义的管道最大容量
int main(void){
	  int pipe_fd;
	  int res;
	  int len;
	  int alllen;
	  char buffer[BUFFER_SIZE+1];//每次写数据用的缓冲区
	  int bytes_sent=0;
	  
	  if (access(PIPE_NAME,F_OK)==-1){//如果不存在PIPE_NAME，则建立
	  	  res=mkfifo(PIPE_NAME,0777);//命名管道 
	  	  if (res!=0){
	  	     perror("create pipe error!");	
	  	     exit(1);	  	     
	  	  }	
	  }
	  strcpy(buffer,"deepfuture.iteye.com\n");
	  len=strlen(buffer); 	  	
	  alllen= len*2;
	  //打开管道,管道都是FIFO
	  printf("process %d opening pipe !",getpid());
	  pipe_fd=open(PIPE_NAME,O_WRONLY);
	  printf("process %d result %d\n",getpid(),pipe_fd);
	  if (pipe_fd!=-1){//发送数据
	      while (bytes_sent<alllen){
	        res=write(pipe_fd,buffer,len);	
            printf("%d bytes sending........\n",res); 		        
	        if (res==-1){
	        	  perror("write error on pipe\n");
	  	          exit(1);	        	  
	        }
	        bytes_sent+=res;//res为本次写的字节数，bytes_sent为总字节数
	      }
	      close(pipe_fd);	
	  }	
	  else{
	  	  exit(1);
	  }
	  printf("%d bytes sended!\n",bytes_sent); 	
	  return 0;
}
	

