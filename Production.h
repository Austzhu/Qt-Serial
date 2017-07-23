#ifndef PRODUCTION_H
#define PRODUCTION_H
#include <QThread>
#include "Crc16.h"
#include <cstdio>
#include <ctype.h>
#include <QFile>
#include <iostream>
using namespace std;

typedef unsigned char u8;

#define PACKAGE_SIZE  9

enum {
    WriteBaseInfo,
    OpenLight,
    CloseLight,
    ReadBaud,
    ReadAddr,
    ReadVol,
    ReadElec,
    ReadPower,
    OneTest,
    CloseProduction,
    AdjustLight,
};

enum {
    Dsplay_onlab,
    Dsplay_onwindow,
    Dsplay_Hexonlab,
    Dsplay_Hexonwindow,
};

struct Protocol{
    u8 Crtl;
    u8 Cmd[2];
    u8 Data[4];
    u8 Crc16[2];
};



class Production : public QThread
{
     Q_OBJECT
public:
    Production();
    void run();
    int SendData(void*,int);
    int write_Addr(void);
    int Write_Vol(void);
    int Write_Elec(void);
    int Write_Baud(void);

    int RecvOnePackage(void*,int);
    int AnalysisBaud(u8);

    int Adjust_Light(void);
    int Write_BaseInfo(void);
    int Open_Light(void);
    int Close_Light(void);
    int Read_Addr(void);
    int Read_vol(void);
    int Read_Elec(void);
    int Read_Power(void);
    int Read_Baud(void);
    void One_Test(void);
    void EmitSingles(int,const void*,int);

signals:
    void SendSignal(void*,int*);
    void ResponseSignal(int,int);
    void DisplayInfo(int,const void *,int);

private slots:
    void ProductionSlots(int);
private:
    int do_cmd;
};

#endif // PRODUCTION_H
