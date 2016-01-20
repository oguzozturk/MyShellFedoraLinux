#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>

#define MAX_LINE 80

#define CREATE_FLAGS (O_WRONLY | O_CREAT | O_APPEND)
#define CREATE_FLAGS2 (O_WRONLY | O_CREAT | O_TRUNC)
#define CREATE_FLAGS3 (O_RDONLY | O_APPEND )

#define CREATE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

//Sequence number of processes
int jobnum=1;

//Linked LÝst elements
struct node{
    int jobnumber;    
    long mypid;
    int isFinished;
    int isPrinted;
    char processname[100];
    struct node *next;
}*head=NULL,*var,*trav;

void setup(char inputBuffer[], char *args[],int *background){

    int length, 
        i,      
        start,
        ct;
    
    ct = 0;
    length = read(STDIN_FILENO,inputBuffer,MAX_LINE);  

    start = -1;
    if (length == 0)
        exit(0);
    if ( (length < 0) && (errno != EINTR) ) {
        perror("error reading the command");
	      exit(-1); 
    }

    for (i=0;i<length;i++){

        switch (inputBuffer[i]){
	    case ' ':
	    case '\t' :          
		if(start != -1){
                    args[ct] = &inputBuffer[start];
		    ct++;
		}
                inputBuffer[i] = '\0';
		start = -1;
		break;

            case '\n': 
		if (start != -1){
                    args[ct] = &inputBuffer[start];     
		    ct++;
		}
                inputBuffer[i] = '\0';
                args[ct] = NULL; 
		break;

	    default :   
		if (start == -1)
		    start = i;
		if (inputBuffer[i] == '>' ){
		    *background  = 2;
		}
        if (inputBuffer[i] == '&' && inputBuffer[i-1] != '>'){
		    *background  = 1;
                    inputBuffer[i-1] = '\0';
		}
	}
     }
     args[ct] = NULL;
}

//Create Linked List
void createList(int value,char name[100],long mypd,int finished,int printed){

	    struct node *temp; 
      //Initialize temp as head of list
      temp=head;
      //Allocate memory for var node
      var=(struct node *)malloc(sizeof (struct node));
      //Assign values to linked list elements
      var->jobnumber=value;
      strcpy(var->processname,name);
      var->mypid=mypd;
      var->isFinished=finished;
      var->isPrinted=printed;

      if(head==NULL){
          head=var;
          head->next=NULL;
      }
      else{
          while(temp->next!=NULL){     
               temp=temp->next;
          }
          var->next=NULL;
          temp->next=var;
      }
}

//Update state if user closed the process
void updateProcessState() {
  int status=0;
  trav=head;
  while(trav!=NULL){
    if (waitpid(trav->mypid,&status,WNOHANG)) {
      trav->isFinished=1;
    }
    trav=trav->next;
  }
}

//Delete Processes
int killProcess(int value){

     struct node *temp,*var;
     //Initialize temp as head of linked list
     temp=head;
     int count=0;

     while(temp!=NULL){
          //If argument equals linked list pid value
          if(temp->mypid == value){
            count++;
            //Assign isFinished to 1 for print in ps_all one time
            temp->isFinished=1;
            //If it is printed then delete element in this condition
            if(temp->isPrinted==1){
              //If element is first element
                if(temp==head){
                     head=temp->next;
                     free(temp);                   
                     return 0;
                }
                else{
                     var->next=temp->next;
                     free(temp);                    
                     return 0;
                }
           }
           return 0;     
          }
          //If argument equals linked list jobnumber value
          if(temp->jobnumber == value){
            count++;
            temp->isFinished=1;
              //Kill process 
                kill(temp->mypid,SIGKILL);
              if(temp->isPrinted==1){
                if(temp==head){
                     head=temp->next;
                      free(temp);
                     return 0;
                }
                else{
                     var->next=temp->next;
                      free(temp);
                     return 0;
                }
             }
             return 0;   
          }
          else{
               var=temp;
               temp=temp->next;
          }
          
     }
     //If does not enter any condition value is not  exist in linked list 
     if(count==0){
        printf("Error! Process is not exist.\n");
     }
}

//Kill terminal process
void exitTerminal(){

     trav=head;
     int a=0;
     //If linked list is empty then exit
     if(trav==NULL){
          exit(0);
     }
     else{

          while(trav!=NULL){
            //If background processes exit then it shows them
              if(trav->isFinished==0){
                printf("Running   ");
                printf(" -> [%d]  %s  (Pid = %ld)",trav->jobnumber,trav->processname,trav->mypid);
                printf("\n");
                a++;
              }                            
               trav=trav->next;               
          }
          if(a==0){
            exit(0); 
          }
          printf("Background processes are still running.\n" );
      }
}

//Search jobnumber exists in linked list
long searchForFg(int jbn){

     trav=head;
     //Linked list is empty so it is not exist
     if(trav==NULL){
          printf("\nThis is not exits.\n");
     }
     else{

          while(trav!=NULL){
            //If it is exist
            if(trav->jobnumber=jbn){
              //return pid value of this process
              return trav->mypid;
            }
               trav=trav->next;
          }
      }
}

//Displays linked list for ps_all command
void display(){

	printf("\nMYSHELL:  ps_all\n" );

     trav=head;
     if(trav==NULL){
          printf("List is Empty\n");
     }
     else{
          
          while(trav!=NULL){
               //Prints running processes
               if(trav->isPrinted==0 && trav->isFinished==0){
                  printf("Running:\n");
                  printf("     -> [%d]  %s  (Pid = %ld) isFinished:%d isPrinted:%d",trav->jobnumber,trav->processname,trav->mypid,trav->isFinished,trav->isPrinted);
               } 
               //Prints finished processes
               if(trav->isPrinted==0 && trav->isFinished==1){
                    printf("Finished:\n");
                    printf("     -> [%d]  %s  (Pid = %ld) isFinished:%d isPrinted:%d",trav->jobnumber,trav->processname,trav->mypid,trav->isFinished,trav->isPrinted);
                    trav->isPrinted=1;
                    //killProcess(trav->jobnumber);
               }      
               
               trav=trav->next;
               printf("\n");
          }               
      printf("\n");
     }
}
 
int main(void){

      char inputBuffer[MAX_LINE];
      int background;
      char *args[MAX_LINE/2 + 1]; 
      char *args2[MAX_LINE/2 + 1];  /*command line arguments */
	    pid_t childpid,childpid2;
	    char buf[100];
	    struct node* head = NULL;	
	    

      while (1){
      int a = 0 , b = 0 , i = 0 ;
      background = 0;
      printf(" 333sh: ");
			fflush(stdout);

      setup(inputBuffer, args, &background);
      //Checks processes states
      updateProcessState();
      
      //If user enters exit command
      if((strcmp(args[0],"exit"))==0 ){
          exitTerminal();
      }
      //If user enters fg commands
      else if((strcmp(args[0],"fg"))==0){
        //Compare % character
        if(strstr(args[1], "%")!=NULL){
          //Take jobnumber value
          char *token2 = strtok(args[1],"%");
          int x=atoi(token2);
          //Return us pid value of this jobs
          long a=searchForFg(x);
          //Then waits parent dead
          if (wait(NULL) > a){
          } 
        }

      }
      //If user enter ps_all command
      else if((strcmp(args[0],"ps_all"))==0 ){
				display();
			}
      //If user enter kill command
			else if((strcmp(args[0],"kill"))==0){				
        //Compares exist % character
				if(strstr(args[1], "%")!=NULL){
          //Take jobnumber value
					char *token = strtok(args[1],"%");
					int x=atoi(token);
          //Send this value for kill this process
					killProcess(atoi(token));
				}
				else{
          //If user enters pid value kill directly and send it killProcess
					kill(atoi(args[1]),SIGKILL);
					killProcess(atoi(args[1]));
				}

			}
      //This condition includes I/O REDIRECTION,BACKGROUND AND FOREGROUND PROCESSES
			else{
			char wcommand[] = "which ";
      //Combine which and with argument 0
			strcat(wcommand,args[0]);
			
      //Writes path to file.
			FILE *ls = popen(wcommand, "r");
			//Reads path from file with fgets function 
			while (fgets(buf, sizeof(buf), ls) != 0){
			}
			pclose(ls);
      //Delete last character of buf array because fgets puts new line character end of buf array
			buf[strlen(buf)-1] = '\0';

      while(args[i]!= NULL){                          ///// BEGIN WHILE FOR REDIRECTION
                  
      int fd;   

      if((strcmp(args[i],">>")) == 0){                  //// >> CHECKING
            a=1;
            childpid=fork();

            if(childpid==0){

            fd = open(args[i+1], CREATE_FLAGS, CREATE_MODE);

            if (fd == -1) {
              perror("Failed to open file");
                return 1;
            }
            else if (dup2(fd, STDOUT_FILENO) == -1) {
                perror("Failed to redirect standard output");
                return 1;
            }
            else if (close(fd) == -1) {
                perror("Failed to close the file");
                return 1;
            } 

            args[i]= NULL;
            execv(buf,&args[0]); 
            }else{
            waitpid(childpid,NULL,0);
            kill(childpid,SIGKILL);
            }   
      }

      else if(((strcmp(args[i],">")) == 0)  ){

          a=1;            /// > CHECKING
          childpid=fork();

          if(childpid==0){

              fd = open(args[i+1], CREATE_FLAGS2, CREATE_MODE);
          if (fd == -1) {
             perror("Failed to open file");
              return 1;
            }
            else if (dup2(fd, STDOUT_FILENO) == -1) {
                perror("Failed to redirect standard output");
               return 1;
            }
            else if (close(fd) == -1) {
             perror("Failed to close the file");
               return 1;
            }
         
          args[i]= NULL;
            execv(buf,&args[0]); 
          
          }else{
            waitpid(childpid,NULL,0);
            kill(childpid,SIGKILL);
          } 
            
      }
    
    
    else if(strcmp(args[i],"<") == 0 ){

        a = 1;
        childpid2= fork();

        if(childpid2==0){
                                       
          fd = open(args[i+1],CREATE_FLAGS3);
                                      
        if (fd == -1) {
          perror("Failed to open file");
          return 1;
            }                             
            else if (dup2(fd, STDIN_FILENO) == -1) {
                 perror("Failed to redirect standard output");
               return 1;
            }
            else if (close(fd) == -1) {
             perror("Failed to close the file");
               return 1;
            }                           
           args[i]=NULL;                              
           execv(buf,&args[0]); 
          
          }else{                               
            waitpid(childpid2,NULL,0);                              
            kill(childpid2,SIGKILL);
          }

    }

    else if((strcmp(args[i],">&")) == 0 ){

       a=1;
        childpid = fork();
        if(childpid==0){
              fd = open(args[i+1],CREATE_FLAGS2,CREATE_MODE);
            if (fd == -1) {
               perror("Failed to open file");
               return 1;
            }

            else if (dup2(fd, STDERR_FILENO) == -1) {
                
               perror("Failed to redirect standard output");
               return 1;
            }
            else if (close(fd) == -1) {
               perror("Failed to close the file");
               return 1;
            }
          args[i]= NULL;

          char wcommand3[] = "which ";
          char buf3[100];
          strcat(wcommand3,args[i-1]);
              
            FILE *ls3 = popen(wcommand3, "r");
        
            while (fgets(buf3, sizeof(buf3), ls3) != 0) {
            }
            pclose(ls3);

            buf3[strlen(buf3)-1] = '\0';

          if((execv(buf,&args[0]))<0){
            perror("Failed Not exist command !");
            exit(0);
          }
          
                    }else{
            
            waitpid(childpid,NULL,0); 
            
            kill(childpid,SIGKILL);
           
            
          }     
    }


       else if((strcmp(args[i],"<") == 0) && (strcmp(args[i+2],">") == 0 )){  

            a=1;                                    ///// < > CHECKING
            childpid=fork();

            if(childpid==0){
              int fd2;
              fd = open(args[i+1],CREATE_FLAGS3,CREATE_MODE);
            if (fd == -1) {
             perror("Failed to open file");
               return 1;
            }
            else if (dup2(fd, STDIN_FILENO) == -1) {
             perror("Failed to redirect standard output");
               return 1;
            }
            else if (close(fd) == -1) {
             perror("Failed to close the file");
               return 1;
            }
              
            fd2 = open(args[i+3],CREATE_FLAGS2, CREATE_MODE);
            if (fd2 == -1) {
             perror("Failed to open file");
               return 1;
            }

            else if (dup2(fd2, STDOUT_FILENO) == -1) {
             perror("Failed to redirect standard output");
               return 1;
            }
            else if (close(fd2) == -1) {
             perror("Failed to close the file");
               return 1;
            }

          args[i]= NULL;
          //args[i+2]=NULL;  //????????????
          execv(buf,&args[0]); 
          
          }else{
            waitpid(childpid,NULL,0);
            kill(childpid,SIGKILL);
          }
      }

      else if((strcmp(args[i],"|")) == 0){
        a=1;
        childpid2=fork();

       if(childpid2==0) {  //////   PÝPE LÝNE   
            int fd[2];
       if ((pipe(fd) == -1) || ((childpid = fork()) == -1)) {
           perror("Failed to setup pipeline");
           return 1;
        }
        if (childpid == 0) { /* ls is the child */
        if (dup2(fd[1], STDOUT_FILENO) == -1)
             perror("Failed to redirect stdout of ls");
        else if ((close(fd[0]) == -1) || (close(fd[1]) == -1))
             perror("Failed to close extra pipe descriptors on ls");
        else {
                args[i]=NULL;
                execv(buf,&args[0]);
                perror("Failed to exec ls");
        }
          return 1;
        }

      if (dup2(fd[0], STDIN_FILENO) == -1) /* sort is the parent */
          perror("Failed to redirect stdin of sort");
      else if ((close(fd[0]) == -1) || (close(fd[1]) == -1))
          perror("Failed to close extra pipe file descriptors on sort");
      else {

          char wcommand2[] = "which ";
          char buf2[100];
          strcat(wcommand2,args[i+1]);
              
            FILE *ls2 = popen(wcommand2, "r");
        
            while (fgets(buf2, sizeof(buf2), ls2) != 0) {
            }
            pclose(ls2);

            buf2[strlen(buf2)-1] = '\0';
            args[i]=NULL;
            execv(buf2,&args[i+1]);
        
            perror("Failed to exec sort");
      }

     }
     else{
           waitpid(childpid2,NULL,0);
            kill(childpid2,SIGKILL);
     }
 }

     i++;   
}
			
			//Run foreground
			if(background == 0 && a == 0){
      //Create process
			childpid=fork();

			if(childpid<0){
			   perror("Error!");
			   return 1;
			}
			else if(childpid==0){
         //Execute child process
			   execv(buf,&args[0]);         
			}
			else{	
        //Waits until child process die
				while (wait(NULL) > childpid){
				}   
			}
				
      }
			//Run background
			if(background==1){
			   int i=0;
			   while(args[i]!=NULL){
			   		//Extract & character from arguments
			   		if((strcmp(args[i],"&"))==0){
			   			args[i]='\0';
			   		}
			   		i++;			   
			   }
			  char name[100];
        //Get name of process
        strcpy(name,args[0]);
			  pid_t mypid;
        //Create process
			  childpid=fork();
			  mypid=getpid();			

			  if(childpid<0){
			     perror("Error!");
			     return 1;
			  }
			  else if(childpid==0){	
           //Execute child process				      		   		   
			     execv(buf,&args[0]);					      			   			   		           
			  }
			  else{
          //Ýnsert elements to linked list
				  createList(jobnum,name,childpid,0,0);
          //Increase jobnumber
				  jobnum++;
			  }			
			
			}
		}
  }
	
}


