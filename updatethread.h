#ifndef UPDATETHREAD_H
#define UPDATETHREAD_H
#include "Crc16.h"
#include <cstdio>
#include <ctype.h>
#include <QThread>
#include <QFile>
#include <iostream>
#include <QTextStream>
#include <QFileDialog>
using namespace std;

#define SOH 	0x01
#define STX 	0x02
#define EOT 	0x04
#define ACK 	0x06
#define NAK 	0x15
#define CAN 	0x18
#define CTRLZ	0x1A

#define UPACKSIZE 	128

#define AssertProgram(Expr)	do{if(Expr)return 0; }while(0)

#define Single_A        "./Single_A.bin"
#define Single_B        "./Single_B.bin"
#define Coordinate_A    "./Coordinate_A.bin"
#define Coordinate_B    "./Coordinate_B.bin"

#define Repeat_Num 		5
#define TimeoutNum 		5

#define SUCCESS         0x00
#define FAIL            0X01
#define ERRORS          0XFF
#define Open_FAIL       0XFE
#define TIME_OUT        0XFD
#define Update_Pend     0XFC

#define ReadbufSize     1024

typedef unsigned char   u8;
typedef unsigned int    u32;
typedef int             s32;

enum{
    Crc_16,
    Crc_8,
    addChk,
};

enum {
    Update_Message,
    Update_GetNewtalk,
    Update_GetCoordiAddr,
    Update_GetSingleAddr,
    Update_IsCoordi,
    Update_GetFileName,
    Update_SetprogressMax,
    Update_SetprogressValues,
    Update_finish,
};

typedef struct{
    u32 filesize;       //升级文件大小
    u32 DataNum;        //需要发送多少帧数据
    u32 CntDataNum;     //当前发送的帧
    u32 Repeat;         //每帧错误重发次数
    u32 SinOrCoordi;    //升级单灯/协调器标记
    u32 CheckType;      //校验方式
    FILE *fd;          //升级文件描述符
    s32 pend;           //升级挂起
    s32 Timeout;        //超时
    s32 Update_OK;      //升级完成
    s32 Is_UpdateImageA;//升级A区？
}Package_Info;

typedef struct{
    u8 	Header;
    u8 	Coor_Addr;
    u8 	Single_Addr_H;
    u8 	Single_Addr_L;
    u8 	F_Num;              //第几帧数据
    u8 	__Num;
    u8 	Data[UPACKSIZE];
    u8 	CRC16[2];           //低在前，高位在后
}Package_Data;

typedef struct{
    u8 	group_addr;
    u8 	slave_addr;
    u8 	single_addrH;
    u8 	single_addrL;
    u8 	ctrl;
    u8 	cmd_h;
    u8 	cmd_l;
    u8 	light_level;
    u8 	crc16_l;
    u8 	crc16_h;
}Frame_485;

typedef struct {
    u8	Header;
    u8 	Ctrl;
    u8 	Group_Addr;
    u8 	Coordi_Addr;
    u8 	Single_Addr[2]; //高地址在前，低地址在后
    u8 	Cmd[2];         //高字节在前，低字节在后
    u8 	Data[2];
    u8 	Crc16[2];
}Pag_Single;


class UpdateThread:public QThread
{
    Q_OBJECT
public:
    UpdateThread();
    void run();
    int  Update_Single_Coordi(void);
    void Close_Update(void);
    s32  Update_Init(char Image);
    int  SendSignals(int,const void*);
    int  SendData(void*,int);
    void Send_Package(void);
    void Update_Thread();

    Package_Info   UpdateInfo;
    Package_Data   UpdatePackage;
    char           UpdateFilename[128];
    QFileDialog    fffd;


signals:
    void UpdateSignals(int,void*);
    void SendSignal(void*,int*);
};

#endif // UPDATETHREAD_H
