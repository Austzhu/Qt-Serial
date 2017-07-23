#include "updatethread.h"
#include "Singleup.h"
#include <cstring>
#include <QMessageBox>
#include <QIODevice>

void DisplayPackage(const char*Message,void *buf,int nLen)
{
    int i = 0;
    unsigned char *Pbuf =  (unsigned char*)buf;
    printf("%s",Message);
    while(i++ < nLen){
        printf("%02X ",*Pbuf++);
    }printf("\n");
}

UpdateThread::UpdateThread()
{
    memset(&UpdateInfo,0,sizeof(UpdateInfo));
    memset(&UpdatePackage,0,sizeof(UpdatePackage));
    memset(UpdateFilename,0,sizeof(UpdateFilename));
}

int  UpdateThread::SendSignals(int cmd,const void *buf)
{
    static int flag;
    char Str[128];
    memset(Str,0,sizeof(Str));
    Signal_OK = false;
    flag = 0;
    switch(cmd){
        case Update_Message:
            if(buf){strcpy(Str,(const char*)buf);}emit UpdateSignals(Update_Message,Str);break;
        case Update_GetNewtalk:
            emit UpdateSignals(Update_GetNewtalk,&flag);break;
        case Update_IsCoordi:
            emit UpdateSignals(Update_IsCoordi,&flag);break;
        case Update_GetFileName:
            emit UpdateSignals(Update_GetFileName,UpdateFilename); break;
        case Update_SetprogressMax:
            flag = *(const int*)buf; emit UpdateSignals(Update_SetprogressMax,&flag); break;
        case Update_SetprogressValues:
            flag = *(const int*)buf; emit UpdateSignals(Update_SetprogressValues,&flag);break;
        case Update_GetCoordiAddr:
            emit UpdateSignals(Update_GetCoordiAddr,&flag);
            while(!Signal_OK);
            //printf("$$$$0x%x\n",flag);
            break;
        case Update_GetSingleAddr:
            emit UpdateSignals(Update_GetSingleAddr,&flag);
            while(!Signal_OK);
            //printf("$$$$0x%x\n",flag);
            break;
        case Update_finish:
            emit UpdateSignals(Update_finish,NULL);
            break;
        default:
            Signal_OK = true; break;
    }
    while(!Signal_OK);

    return flag;
}

int UpdateThread::SendData(void*buf,int nLen)
{
    Signal_OK = false;
    //int ii = 0;
    static u8 SendBuf[1024];
    memset(SendBuf,0,sizeof(SendBuf));
    memcpy(SendBuf,buf,nLen);
    emit SendSignal(SendBuf,&nLen);

//    printf("send data:");
//    while(ii < nLen){
//        printf("%02x ",SendBuf[ii++]);
//    }printf("\n");
    while(!Signal_OK);
    return nLen;
}

void UpdateThread::run()
{
    Update_Thread();
    cout<<"Qthread will be exit!"<<endl;
    SendSignals(Update_finish,NULL);
}

void UpdateThread::Update_Thread()
{
    int RetuenValue = -1;
    int TimeOut = 0;
    Frame_485  pkg;
    Pag_Single Newpkg;
    char buf[32];
    memset(buf,0,sizeof(buf));
    memset(&Newpkg,0,sizeof(Newpkg));
    memset(&pkg,0,sizeof(pkg));
    cout<<"start update..."<<endl;
    Close_Update();
    SendSignals(Update_Message,"start update...");

    if(SendSignals(Update_GetNewtalk,NULL)){//使用的是新的协议
        Newpkg.Header   = 0xff;
        Newpkg.Ctrl     = 0x10;
        Newpkg.Cmd[0]   = (0x80>>8)&0xff;
        Newpkg.Cmd[1]   = 0x80&0xff;
        Newpkg.Data[0]  = !0;
        Newpkg.Coordi_Addr = SendSignals(Update_GetCoordiAddr,NULL);
        printf("Coordi_Addr:0x%x\n",Newpkg.Coordi_Addr);
        if(!SendSignals(Update_IsCoordi,NULL)){
            cout<<"use new talk!"<<endl;
            UpdateInfo.SinOrCoordi = 0;
            Newpkg.Data[0]         = 0;
            Newpkg.Data[1]         = 0;
            RetuenValue            = SendSignals(Update_GetSingleAddr,NULL);
            Newpkg.Single_Addr[0]  = (RetuenValue>>8) &0xff;
            Newpkg.Single_Addr[1]  = RetuenValue&0xff;
            printf("Single_Addr:0x%x\n",(Newpkg.Single_Addr[0]<<8)|Newpkg.Single_Addr[1]);
        }else{
            UpdateInfo.SinOrCoordi = !0;
            UpdateInfo.SinOrCoordi = !0;
        }
        Crc16(Newpkg.Crc16,(u8*)&Newpkg,sizeof(Newpkg)-2);
        SendData(&Newpkg,sizeof(Newpkg));
        TimeOut = 5;
        memset(&Newpkg,0,sizeof(Newpkg));
        while(Newpkg.Header != 0xff && TimeOut--){
            Recv((char*)&Newpkg.Header,1,1000);
        }
        if(TimeOut <= 0){
            SendSignals(Update_Message,"Can't Recv Header!");
            cout<<"Can't Recv Header!"<<endl;
            return;
        }
        Recv((char*)&Newpkg.Ctrl,sizeof(Newpkg)-1,2000);
        if(CHK_Crc16(Newpkg.Crc16,(u8*)&Newpkg,sizeof(Newpkg)-2)){
            SendSignals(Update_Message,"Crc check err!");
            cout<<"Crc check err!"<<endl;
            return;
        }
        UpdatePackage.Coor_Addr     = Newpkg.Coordi_Addr;
        UpdatePackage.Single_Addr_H	= Newpkg.Single_Addr[0];
        UpdatePackage.Single_Addr_L = Newpkg.Single_Addr[1];
        printf("Coordi Addr:0x%02x,Single Addr:0x%04x\n",UpdatePackage.Coor_Addr,
                        (UpdatePackage.Single_Addr_H<<8) | UpdatePackage.Single_Addr_L);
        RetuenValue = Update_Init(Newpkg.Data[0]) == SUCCESS ? (int)Update_Single_Coordi():-1;
    }else{//旧协议
            pkg.ctrl    = 0x01;
            pkg.cmd_h   = (0x80>>8)&0xff;
            pkg.cmd_l   = 0x80&0xff;
            pkg.slave_addr = SendSignals(Update_GetCoordiAddr,NULL);
            printf("Coordi Addr:0x%02x\n",pkg.slave_addr);
            if(!SendSignals(Update_IsCoordi,NULL)){
                UpdateInfo.SinOrCoordi = 0;
                pkg.light_level  = 0;
                RetuenValue      = SendSignals(Update_GetSingleAddr,NULL);
                pkg.single_addrH = (RetuenValue>>8) &0xff;
                pkg.single_addrL = RetuenValue&0xff;
                printf("Single Addr:0x%04x\n",(pkg.single_addrH<<8) | pkg.single_addrL);
            }else{
                pkg.light_level  = !0;
                UpdateInfo.SinOrCoordi = !0;
            }
            Crc16(&pkg.crc16_l,(u8*)&pkg,sizeof(pkg)-2);
            SendData(&pkg,sizeof(pkg));
            memset(&pkg,0,sizeof(pkg));
            if(-1 == Recv((char*)&pkg,sizeof(pkg),1000)){
                SendSignals(Update_Message,"Wait response time out!");
                return;
            }
            DisplayPackage("Recv date:",&pkg,sizeof(pkg));
            if(CHK_Crc16(&pkg.crc16_l,(u8*)&pkg,sizeof(pkg)-2)){
                SendSignals(Update_Message,"Crc check err!");
                cout<<"Crc check err!"<<endl;
                return;
            }
            UpdatePackage.Coor_Addr     = pkg.slave_addr;
            UpdatePackage.Single_Addr_H	= pkg.single_addrH;
            UpdatePackage.Single_Addr_L = pkg.single_addrL;
            printf("Coordi Addr:0x%02x,Single Addr:0x%04x\n",UpdatePackage.Coor_Addr,
                    UpdatePackage.Single_Addr_H<<8 | UpdatePackage.Single_Addr_L);
            RetuenValue = Update_Init(pkg.light_level) == SUCCESS ? Update_Single_Coordi():FAIL;
    }

    switch(  RetuenValue&0xff  ){
        case SUCCESS:
            cout<<"success!"<<endl;
            Close_Update();
            break;
        case Update_Pend:
            Close_Update();
            cout<<"Update will be Pend!"<<endl;
            break;
        case TIME_OUT:
            Close_Update();
            cout<<"Update time out!"<<endl;
            break;
        case FAIL:cout<<"Update Init error!"<<endl;
            Close_Update();
            break;
        default:cout<<"return values"<<RetuenValue<<endl;
        break;

    }
   // this->terminate();
}

void UpdateThread::Close_Update(void)
{
    if(UpdateInfo.fd != NULL){
        fclose(UpdateInfo.fd);
        memset(&UpdateInfo,0,sizeof(Package_Info));
    }
}

s32 UpdateThread::Update_Init(char Image)
{
    //QString str;
    char MessageBuf[128];
    char Is_ImageA;
    switch(Image){
        case 'a':
        case 'A':Is_ImageA = 'A';break;
        case 'b':
        case 'B':Is_ImageA = 'B';break;
        case FAIL:
        default:
            SendSignals(Update_Message,"Unknown Update for Image");
            cout<<"Unknown Update for Image "<<Image<<endl;
            return FAIL;
    }
    if(UpdateInfo.SinOrCoordi){//协调器
        memset(MessageBuf,0,sizeof(MessageBuf));
        sprintf(MessageBuf,"Update Coordinate for Image %c",Is_ImageA);
        SendSignals(Update_Message,MessageBuf);

        cout<<"Update Coordinate for Image "<<Is_ImageA<<endl;
        memset(UpdateFilename,0,sizeof(UpdateFilename));
        //SendSignals(Update_GetFileName,NULL);
        //Is_ImageA == 'A' ?  open(Coordinate_A,O_RDONLY) :  open(Coordinate_B,O_RDONLY);
        strcpy(UpdateFilename,Is_ImageA == 'A' ?Coordinate_A:Coordinate_B);
        if(!strlen(UpdateFilename)){
           // QMessageBox MessageBox("请选择协调器的升级文件","标题", QMessageBox::MB_OK);
        }
    }else{//单灯
        cout<<"Update Single for Image "<<Is_ImageA<<endl;
        memset(MessageBuf,0,sizeof(MessageBuf));
        sprintf(MessageBuf,"Update Single for Image %c",Is_ImageA);
        SendSignals(Update_Message,MessageBuf);

        memset(UpdateFilename,0,sizeof(UpdateFilename));
        strcpy(UpdateFilename,Is_ImageA == 'A' ?Single_A:Single_B);
        //SendSignals(Update_GetFileName,NULL);
        if(!strlen(UpdateFilename)){
            //QMessageBox MessageBox("请选择协调器的升级文件","标题", MB_OK);
        }
    }
    printf("Upadte File name:%s\n",UpdateFilename);
    memset(MessageBuf,0,sizeof(MessageBuf));
    sprintf(MessageBuf,"Upadte File name:%s",UpdateFilename);
    SendSignals(Update_Message,MessageBuf);

    UpdateInfo.fd = fopen(UpdateFilename,"rb");
    if(UpdateInfo.fd == NULL){
        SendSignals(Update_Message,"Open update file failed!");
        cout<<"Open update file failed!"<<endl;

        memset(UpdateFilename,0,sizeof(UpdateFilename));
        SendSignals(Update_GetFileName,NULL);
        if((UpdateInfo.fd = fopen(UpdateFilename,"rb")) == NULL){
            return FAIL;
        }


    }

    /* 初始化升级信息 */
    fseek(UpdateInfo.fd,0,SEEK_END);
    UpdateInfo.filesize 	= ftell(UpdateInfo.fd);
    cout<<"FileSize:"<<UpdateInfo.filesize<<endl;

    memset(MessageBuf,0,sizeof(MessageBuf));
    sprintf(MessageBuf,"FileSize:%d",UpdateInfo.filesize);
    SendSignals(Update_Message,MessageBuf);

    fseek(UpdateInfo.fd,0,SEEK_SET);

    UpdateInfo.DataNum 	= (UpdateInfo.filesize+UPACKSIZE-1)/UPACKSIZE;	//(UpdateInfo.filesize+1023)/256,计算数据包有多少帧
    cout<<"DataNum:"<<UpdateInfo.DataNum<<endl;

    memset(MessageBuf,0,sizeof(MessageBuf));
    sprintf(MessageBuf,"DataNum:%d",UpdateInfo.DataNum);
    SendSignals(Update_Message,MessageBuf);

    UpdateInfo.CntDataNum   = 0;
    UpdateInfo.Repeat       = Repeat_Num;			//重发次数
    UpdateInfo.CheckType 	= Crc_16;
    UpdateInfo.pend         = 0;
    UpdateInfo.Timeout      = TimeoutNum;
    /* 初始化进度条 */
    SendSignals(Update_SetprogressMax,&UpdateInfo.DataNum);
    SendSignals(Update_SetprogressValues,&UpdateInfo.CntDataNum);
    SendSignals(Update_Message,"Start Update Coordinate...");
    cout<<"Start Update Coordinate..."<<endl;
    return SUCCESS;
}

void UpdateThread::Send_Package(void)
{
    s32 ReadSize        = 0;
    s32 IsEOT           = UpdateInfo.CntDataNum >= (UpdateInfo.DataNum-1) ? 1:0;
    UpdatePackage.F_Num = UpdateInfo.CntDataNum;
    UpdatePackage.__Num = ~UpdateInfo.CntDataNum;

    if(IsEOT){
        UpdatePackage.Header = EOT;
    }else{
        UpdatePackage.Header = SOH;
    }

    cout<<"@@@@@Read point"<<UPACKSIZE*UpdateInfo.CntDataNum<<endl;
    if( fseek(UpdateInfo.fd, UPACKSIZE*UpdateInfo.CntDataNum,SEEK_SET) ){
        cout<<"lseek error!"<<endl;
        SendSignals(Update_Message,"lseek error!");
        return ;
    }

    ReadSize = fread(UpdatePackage.Data,sizeof(char),UPACKSIZE,UpdateInfo.fd);
    cout<<"read size"<<ReadSize<<endl;
    if(ReadSize < UPACKSIZE){
        if(IsEOT){
            memset(UpdatePackage.Data+ReadSize,CTRLZ,UPACKSIZE-ReadSize);
        }
    }
    /* 计算CRC16 */
    Crc16(UpdatePackage.CRC16,UpdatePackage.Data,UPACKSIZE);
    /* 发送数据 */
    SendData(&UpdatePackage.Header,sizeof(Package_Data));
}

int UpdateThread::Update_Single_Coordi(void)
{
    UpdateInfo.Update_OK = 0;
    s32 NAK_CNT = 0;
    Send_Package();
    while(1){
        switch(Recv_OneByte()){
            case ACK:
                UpdateInfo.Repeat 	= Repeat_Num;
                UpdateInfo.Timeout 	= TimeoutNum;
                NAK_CNT =0;
                if(++UpdateInfo.CntDataNum >= UpdateInfo.DataNum){
                    UpdateInfo.Update_OK = 1;
                    SendSignals(Update_SetprogressValues,&UpdateInfo.DataNum);
                    SendSignals(Update_Message,"Update Success!");
                    cout<<"Done!"<<endl;
                    cout<<"UpdateInfo.CntDataNum:"<<UpdateInfo.CntDataNum<<endl;
                    Close_Update();
                    return  SUCCESS;
                }
                SendSignals(Update_SetprogressValues,&UpdateInfo.CntDataNum);
                cout<<"#";fflush(stdout);
                Send_Package();
                break;
            case NAK:
                Send_Package();
                if(!--UpdateInfo.Repeat){
                    UpdateInfo.pend =!0;
                }
                break;
            case FAIL://超时
                if(++NAK_CNT > 5){
                    NAK_CNT = 0;
                    cout<<"Timeout Send package agin!"<<endl;
                    Send_Package();
                    --UpdateInfo.Timeout;
                }
                break;
            default:break;
        }
        if(UpdateInfo.Update_OK){	return SUCCESS;}
        if(UpdateInfo.pend){		return Update_Pend;}
        if(!UpdateInfo.Timeout){	return TIME_OUT;}
    }
    return SUCCESS;
}


