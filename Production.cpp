#include "Production.h"
#include "Singleup.h"

Production::Production()
{
    do_cmd = -1;
}
void Production::run()
{
    while(ProductionRun)
    {
        Is_Display = false;
        switch(do_cmd){
            case WriteBaseInfo: Write_BaseInfo();do_cmd=-1;break;
            case OpenLight:     Open_Light();do_cmd=-1;break;
            case CloseLight:    Close_Light();do_cmd=-1;break;
            case ReadBaud:      Read_Baud();do_cmd=-1;break;
            case ReadAddr:      Read_Addr();do_cmd=-1;break;
            case ReadVol:       Read_vol();do_cmd=-1;break;
            case ReadElec:      Read_Elec();do_cmd=-1;break;
            case ReadPower:     Read_Power();do_cmd=-1;break;
            case OneTest:       One_Test();do_cmd=-1;break;
            case AdjustLight:   Adjust_Light();do_cmd=-1;break;
            default:break;
        }
        Is_Display = true;
        sleep(1);
    }
}

void Production::EmitSingles(int cmd,const void *buf,int nLen)
{
    Signal_OK = false;
    emit DisplayInfo(cmd,buf,nLen);
    while(!Signal_OK);
}

int Production::write_Addr(void)
{
    Protocol package;
    memset(&package,0,sizeof(package));

    package.Crtl            = 0x10;
    package.Cmd[0]          = 0x00;
    package.Cmd[1]          = 0x04;
    package.Data[0]         =(SingleAddr>>8)&0xff;
    package.Data[1]         = SingleAddr&0xff;
    Crc16(package.Crc16,(u8*)&package,PACKAGE_SIZE-2);

    ClearBuf();
    EmitSingles(Dsplay_Hexonlab,&package,PACKAGE_SIZE);
    SendData(&package,sizeof(package));


    if(RecvOnePackage(&package,sizeof(Protocol))){
        return -1;
    }
    EmitSingles(Dsplay_Hexonwindow,&package,PACKAGE_SIZE);
    if(package.Data[0] == 0){
        return 0;
    }else{
        return -1;
    }
}

int  Production::Write_Vol(void)
{
    Protocol package;
    memset(&package,0,sizeof(package));
    package.Crtl    = 0x10;
    package.Cmd[0]  = 0x02;
    package.Cmd[1]  = 0x00;
    package.Data[0] = K_Vol>>0  &0xff;
    package.Data[1] = K_Vol>>8  &0xff;
    package.Data[2] = K_Vol>>16 &0xff;
    package.Data[3] = K_Vol>>24 &0xff;
    Crc16(package.Crc16,(u8*)&package,sizeof(package)-2);

    ClearBuf();
    EmitSingles(Dsplay_Hexonlab,&package,PACKAGE_SIZE);

    SendData(&package,sizeof(package));

    if(RecvOnePackage(&package,sizeof(Protocol))){
        return -1;
    }
    EmitSingles(Dsplay_Hexonwindow,&package,PACKAGE_SIZE);
    if(package.Data[0] == 0){
        return 0;
    }else{
        return -1;
    }
}

int  Production::Write_Elec(void)
{
    Protocol package;
    memset(&package,0,sizeof(package));
    package.Crtl    = 0x10;
    package.Cmd[0]  = 0x08;
    package.Cmd[1]  = 0x00;
    package.Data[0] = K_Elec>>0  &0xff;
    package.Data[1] = K_Elec>>8  &0xff;
    package.Data[2] = K_Elec>>16 &0xff;
    package.Data[3] = K_Elec>>24 &0xff;
    Crc16(package.Crc16,(u8*)&package,sizeof(package)-2);

    ClearBuf();
    EmitSingles(Dsplay_Hexonlab,&package,PACKAGE_SIZE);

    SendData(&package,sizeof(package));

    if(RecvOnePackage(&package,sizeof(Protocol))){
        return -1;
    }
    EmitSingles(Dsplay_Hexonwindow,&package,PACKAGE_SIZE);
    if(package.Data[0] == 0){
        return 0;
    }else{
        return -1;
    }
}

int  Production::Write_Baud(void)
{
    Protocol package;
    memset(&package,0,sizeof(package));
    package.Crtl            = 0x10;
    package.Cmd[0]          = 0x00;
    package.Cmd[1]          = 0x08;
    package.Data[3]         = (Baud&0xff)/10*16 + (Baud&0xff)%10;
    Crc16(package.Crc16,(u8*)&package,sizeof(package)-2);
    ClearBuf();
    EmitSingles(Dsplay_Hexonlab,&package,PACKAGE_SIZE);
    SendData(&package,sizeof(package));


    if(RecvOnePackage(&package,sizeof(Protocol))){
        return -1;
    }
    EmitSingles(Dsplay_Hexonwindow,&package,PACKAGE_SIZE);
    if(package.Data[0] == 0){
        return 0;
    }else{
        return -1;
    }
}

int Production::Write_BaseInfo(void)
{
    if(!write_Addr() && !Write_Vol() && !Write_Baud() && !Write_Elec()){
        emit ResponseSignal(WriteBaseInfo,!0);
    }else{
        emit ResponseSignal(WriteBaseInfo,0);
        return -1;
    }
    return 0;
}

int Production::Open_Light(void)
{
    Protocol package;
    memset(&package,0,sizeof(package));

    package.Crtl            = 0x10;
    package.Cmd[0]          = 0x00;
    package.Cmd[1]          = 0x01;
    package.Data[0]         = 0x1c;//调光值
    Crc16(package.Crc16,(u8*)&package,sizeof(package)-2);

    ClearBuf();
    EmitSingles(Dsplay_Hexonlab,&package,PACKAGE_SIZE);

    SendData(&package,sizeof(package));

    if(RecvOnePackage(&package,sizeof(Protocol))){
        emit ResponseSignal(OpenLight,0);
        return -1;
    }
    EmitSingles(Dsplay_Hexonwindow,&package,PACKAGE_SIZE);
    if(package.Data[0] == 0){
        emit ResponseSignal(OpenLight,!0);
        return 0;
    }else{
        emit ResponseSignal(OpenLight,0);
        return -1;
    }
}

int Production::Close_Light(void)
{
    Protocol package;
    memset(&package,0,sizeof(package));

    package.Crtl            = 0x10;
    package.Cmd[0]          = 0x00;
    package.Cmd[1]          = 0x02;
    package.Data[0]         = 0x00;//调光值
    Crc16(package.Crc16,(u8*)&package,sizeof(package)-2);
    ClearBuf();
    EmitSingles(Dsplay_Hexonlab,&package,PACKAGE_SIZE);
    SendData(&package,sizeof(package));

    if(RecvOnePackage(&package,sizeof(Protocol))){
        emit ResponseSignal(CloseLight,0);
        return -1;
    }
    EmitSingles(Dsplay_Hexonwindow,&package,PACKAGE_SIZE);
    if(package.Data[0] == 0){
        emit ResponseSignal(CloseLight,!0);
        return 0;
    }else{
        emit ResponseSignal(CloseLight,0);
        return -1;
    }
}

int Production::Read_Addr(void)
{
    Protocol package;
    u32 recvaddr = 0;
    memset(&package,0,sizeof(package));
    package.Crtl            = 0x10;
    package.Cmd[0]          = 0x00;
    package.Cmd[1]          = 0x10;
    package.Data[0]         = 0x00;//调光值
    Crc16(package.Crc16,(u8*)&package,sizeof(package)-2);
    ClearBuf();
    EmitSingles(Dsplay_Hexonlab,&package,PACKAGE_SIZE);
    SendData(&package,sizeof(package));

    if(RecvOnePackage(&package,sizeof(Protocol))){
        emit ResponseSignal(ReadAddr,0);
        return -1;
    }
    EmitSingles(Dsplay_Hexonwindow,&package,PACKAGE_SIZE);
    recvaddr = package.Data[0]<<8 | package.Data[1];
     emit ResponseSignal(ReadAddr,recvaddr);
    return 0;
}

int Production::Read_vol(void)
{
    Protocol package;
    int temp =0;
    memset(&package,0,sizeof(package));

    package.Crtl            = 0x10;
    package.Cmd[0]          = 0x00;
    package.Cmd[1]          = 0x40;
    package.Data[0]         = 0x00;//调光值
    Crc16(package.Crc16,(u8*)&package,sizeof(package)-2);
    ClearBuf();
    EmitSingles(Dsplay_Hexonlab,&package,PACKAGE_SIZE);
    SendData(&package,sizeof(package));

    if(RecvOnePackage(&package,sizeof(Protocol))){
        emit ResponseSignal(ReadVol,0);
        return -1;
    }
    EmitSingles(Dsplay_Hexonwindow,&package,PACKAGE_SIZE);
    temp = package.Data[0] | package.Data[1]<<8 | package.Data[2]<<16 | package.Data[3]<<24;
    emit ResponseSignal(ReadVol,temp);
    return 0;
}

int Production::Read_Elec(void)
{
    Protocol package;
    int temp = 0;
    memset(&package,0,sizeof(package));
    package.Crtl            = 0x10;
    package.Cmd[0]          = 0x00;
    package.Cmd[1]          = 0x80;
    package.Data[0]         = 0x00;//调光值
    Crc16(package.Crc16,(u8*)&package,sizeof(package)-2);
    ClearBuf();
    EmitSingles(Dsplay_Hexonlab,&package,PACKAGE_SIZE);
    SendData(&package,sizeof(package));

    if(RecvOnePackage(&package,sizeof(Protocol))){
        emit ResponseSignal(ReadElec,0);
        return -1;
    }
    EmitSingles(Dsplay_Hexonwindow,&package,PACKAGE_SIZE);
    temp = package.Data[0] | package.Data[1]<<8 | package.Data[2]<<16 | package.Data[3]<<24;
    emit ResponseSignal(ReadElec,temp);
    return 0;
}

int Production::Read_Power(void)
{
    Protocol package;
    int temp = 0;
    memset(&package,0,sizeof(package));
    package.Crtl            = 0x10;
    package.Cmd[0]          = 0x01;
    package.Cmd[1]          = 0x00;
    package.Data[0]         = 0x00;//调光值
    Crc16(package.Crc16,(u8*)&package,sizeof(package)-2);
    ClearBuf();
    EmitSingles(Dsplay_Hexonlab,&package,PACKAGE_SIZE);
    SendData(&package,sizeof(package));

    if(RecvOnePackage(&package,sizeof(Protocol))){
        emit ResponseSignal(ReadPower,0);
        return -1;
    }
    EmitSingles(Dsplay_Hexonwindow,&package,PACKAGE_SIZE);
    temp = package.Data[0] | package.Data[1]<<8 | package.Data[2]<<16 | package.Data[3]<<24;
    emit ResponseSignal(ReadPower,temp);
    return 0;
}

int Production::Read_Baud(void)
{
    Protocol package;
    memset(&package,0,sizeof(package));
    package.Crtl            = 0x10;
    package.Cmd[0]          = 0x00;
    package.Cmd[1]          = 0x20;
    package.Data[0]         = 0x00;//调光值
    Crc16(package.Crc16,(u8*)&package,sizeof(package)-2);
    ClearBuf();
    EmitSingles(Dsplay_Hexonlab,&package,PACKAGE_SIZE);
    SendData(&package,sizeof(package));

    if(RecvOnePackage(&package,sizeof(Protocol))){
        emit ResponseSignal(ReadBaud,0);
        return -1;
    }
    EmitSingles(Dsplay_Hexonwindow,&package,PACKAGE_SIZE);
    emit ResponseSignal(ReadBaud,!0);
    AnalysisBaud(package.Data[0]);
    return 0;
}

void Production::One_Test(void)
{
    unsigned int i = 0;
    typedef int (Production::*Pfuc)(void);
    Pfuc func[] = {Write_BaseInfo,Open_Light,Close_Light,Read_Baud,Read_Addr,Read_vol,Adjust_Light};

    while(i < sizeof(func)/sizeof(func[0])){
        if((this->*func[i++])()){
             EmitSingles(Dsplay_onwindow,"One_Test erroe!",0);
            return ;
        }
        QThread::msleep(200);
    }
    QThread::msleep(1500);
    Read_Elec();
    QThread::msleep(200);
    Read_Power();
}

int Production::Adjust_Light(void)
{
    int i = 0x1d;
    Protocol package;
    memset(&package,0,sizeof(package));

    package.Crtl            = 0x10;
    package.Cmd[0]          = 0x00;
    package.Cmd[1]          = 0x01;

    while(i > 0){
        package.Data[0]         = i;        //调光值
        Crc16(package.Crc16,(u8*)&package,sizeof(package)-2);
        EmitSingles(Dsplay_Hexonlab,&package,PACKAGE_SIZE);
        SendData(&package,sizeof(package));
        QThread::msleep(100);
        i -= 2;
    }
    return 0;
}

int Production::AnalysisBaud(u8 data)
{
    int Baud[] = {1200,2400,4800,9600,19200,57600,115200};
    char Str[64];
    memset(Str,0,sizeof(Str));
    sprintf(Str,"Uart1 Baud:%d,Uart2 Baud:%d,Uart3 Baud:57600",Baud[(data&0x0f)-1],Baud[(data>>4&0x0f)-1]);
    EmitSingles(Dsplay_onwindow,Str,0);
    return 0;
}

int Production::RecvOnePackage(void *Rbuf,int nLen)
{
    int TimeOut = 5;
    char *Prbuf = (char*)Rbuf;
    if(NULL == Prbuf || nLen == 0){
        cout << "Incoming parameters error!" << endl;
        return -1;
    }
    memset(Rbuf,0,nLen);
    while(*(u8*)Prbuf != 0x80 && TimeOut--){
        Recv(Prbuf,1,500);
    }
    if(TimeOut <= 0){
        EmitSingles(Dsplay_onwindow,"Production Wait response time out!",0);
        return -1;
    }
    Recv(Prbuf+1,nLen-1,1000);
    if(CHK_Crc16((u8*)(Prbuf+nLen-2),(u8*)Prbuf,nLen-2)){
        EmitSingles(Dsplay_onwindow,"Production crc16 check err!",0);
        return -1;
    }
    return 0;
}

void Production::ProductionSlots(int cmd)
{
    do_cmd = cmd;
}

int  Production::SendData(void *buf,int nLen)
{
    Signal_OK = false;
    static u8 SendBuf[1024];
    memset(SendBuf,0,sizeof(SendBuf));
    memcpy(SendBuf,buf,nLen);
    emit SendSignal(SendBuf,&nLen);
    while(!Signal_OK);
    return nLen;
}
