
using namespace std;
#include "LEDCheck.h"


#define SERIAL_PORT "/dev/ttyUSB1"
#define SW_VERSION "1.0"
#define CMMD_LENGTH 100
#define TIME_OUT 15000  //10S

LEDCheck::LEDCheck()
{
}
LEDCheck::~LEDCheck()
{
}

void LEDCheck::setTermios(struct termios* pNewtio,int uBaudRate)
{
    bzero(pNewtio,sizeof(struct termios));
    pNewtio->c_cflag=uBaudRate|CS8|CREAD|CLOCAL;
    pNewtio->c_iflag=IGNPAR;
    pNewtio->c_oflag=0;
    pNewtio->c_lflag=0;
    pNewtio->c_cc[VINTR] = 0;
    pNewtio->c_cc[VQUIT] = 0;
    pNewtio->c_cc[VERASE] = 0;
    pNewtio->c_cc[VKILL] = 0;
    pNewtio->c_cc[VEOF] = 4;
    pNewtio->c_cc[VTIME] = 5;
    pNewtio->c_cc[VMIN] = 0;
    pNewtio->c_cc[VSWTC] = 0;
    pNewtio->c_cc[VSTART] = 0;
    pNewtio->c_cc[VSTOP] = 0;
    pNewtio->c_cc[VSUSP] = 0;
    pNewtio->c_cc[VEOL] = 0;
    pNewtio->c_cc[VREPRINT] = 0;
    pNewtio->c_cc[VDISCARD] = 0;
    pNewtio->c_cc[VWERASE] = 0;
    pNewtio->c_cc[VLNEXT] = 0;
    pNewtio->c_cc[VEOL2] = 0;
}
int LEDCheck::initCheck()
{
    char cmd[BUFFER_SIZE]="";
    int returnVal=0;
    if ((fd_com1 = open(SERIAL_PORT, O_RDWR | O_NOCTTY))<0) 
    {
        perror("err: can't open serial port!\n");
        return 0;
    }
    sprintf(cmd,"init\r\n");
    tcgetattr(fd_com1, &oldtio);
    setTermios(&newtio, B9600);
    tcflush(fd_com1, TCIFLUSH);
    tcsetattr(fd_com1, TCSANOW, &newtio);
    printf("Start init\n");
    if(strstr(sendCommand(cmd),"OK"))
        returnVal =1;
    tcsetattr(fd_com1, TCSANOW, &oldtio);
    close(fd_com1);
    return returnVal;
}
char *LEDCheck::sendCommand(char *cmd)
{
   
    clock_t start, finish;
    double duration;
    int nRead=0;
    int nTemp=0;
    struct timeval tv;
    fd_set rfds;
    char msg[BUFFER_SIZE]="";
    memset(buff,0,BUFFER_SIZE);

    strcpy(msg,cmd);
    printf("msg=%s\n",msg);
    tv.tv_sec=15;
    tv.tv_usec=0;
    FD_ZERO(&rfds);
    FD_SET(fd_com1, &rfds);
    int sCount=write(fd_com1, msg, strlen(msg));
    if(sCount<=0)
    {
        printf("Serial port data send fail!\n");
        strcpy(buff,"fail");
        return buff;
    }
    start = clock();
    
    while(1)
    {
        if (select(1+fd_com1, &rfds, NULL, NULL, &tv)>0) 
        {
            if (FD_ISSET(fd_com1, &rfds)) 
            {
                nRead=read(fd_com1, buff+nTemp, BUFFER_SIZE-nTemp);
                nTemp=nRead+nTemp;
                char *endBuff=NULL;
                if((endBuff=strstr(buff,";"))||(endBuff=strstr(buff,"!")))
                {
                    *(endBuff+1)='\0';
                    return buff;
                }
                else
                {
                    printf("data not finish\n");
                }
    
            }
            else
            {
                printf("No data read\n");
            }
        }
        else
        {
            printf("select err\n");
        }
        finish = clock();
        duration = (double)(finish - start) ;
        if(duration>TIME_OUT)
        {
            printf("Serial port timed out!\n");
            strcpy(buff,"fail");
            return buff;
    
        }
    }
     
}
int LEDCheck::getFrequency(char *stringRGB,int channel)
{
    char cmd[BUFFER_SIZE]="";
    if ((fd_com1 = open(SERIAL_PORT, O_RDWR | O_NOCTTY))<0) 
    {
        perror("err: can't open serial port!\n");
        return 0;
    }
    tcgetattr(fd_com1, &oldtio);
    setTermios(&newtio, B9600);
    tcflush(fd_com1, TCIFLUSH);
    tcsetattr(fd_com1, TCSANOW, &newtio);
    printf("Start LED test\n");
    sprintf(cmd,"getcolor=%d\r\n",(int)channel);
    strcpy(stringRGB,sendCommand(cmd));
    if(strstr(stringRGB,"error"))
        sprintf(stringRGB,"%s,err cmd:%s",stringRGB,cmd);
    tcsetattr(fd_com1, TCSANOW, &oldtio);
    close(fd_com1);
    return 1;
}



//Function getting digit value from one string.
int LEDCheck::GetStrBetween(char *strSource,char *rGb,const char *strB,const char *strN)
{
        //TODO: Add your source code here
    char *iBPos = strstr(strSource,strB);
    if(!iBPos)
        return 0;
    iBPos += strlen(strB);
    char *iNPos = strstr(strSource,strN);
    if(!iNPos)
        return 0;
    strncpy(rGb,iBPos,iNPos-iBPos);
    return 1;
}
