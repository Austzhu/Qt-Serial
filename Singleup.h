#ifndef SINGLEUP_H
#define SINGLEUP_H

#include <QMainWindow>
#include <tchar.h>
#include <cstring>
#include <cctype>
#include "Crc16.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QIODevice>
#include <QThread>
#include <QFileDialog>
#include <iostream>
#include "updatethread.h"
#include <QMessageBox>
#include "Production.h"
using namespace std;

extern u8   Readbuf[ReadbufSize];
extern int  ReadPtr;
extern int  WritePtr;
extern void ClearBuf(void);
extern void WriteData(QByteArray str);
extern int  ReadData(void *data);
extern int  Recv(char*RecvBuf,int nLen,int block);
extern unsigned char Recv_OneByte(void);
extern volatile bool Signal_OK;
extern volatile bool ProductionRun;
extern volatile bool Is_Display;
extern volatile bool DisplayOnlab; //显示到最下面的lab上
extern int CoordiAddr;
extern int SingleAddr;
extern int K_Vol;
extern int K_Elec;
extern int Baud;

namespace Ui {
class SingleUp;
}
//class UpdateThread;
class SingleUp : public QMainWindow
{
    Q_OBJECT

public:
    explicit SingleUp(QWidget *parent = 0);
    ~SingleUp();
    /* 升级相关 */
    void HexToStr(char *pbDest,unsigned char *pbSrc, int nLen);
    void StrToHex(unsigned char *pbDest, char *pbSrc, int nLen);
    /* 串口相关函数 */
    bool Open_Uart(void);
    void Close_Uart(void);
    int  Send(const void *Sendbuf,int nLen);
    void Testlight(int light);

    QSerialPort    MyCom;
    QFileDialog    Filefd;
    UpdateThread   *thred_update;
    Production     *thread_Production;
    QString        UpdateFilename;
signals:
    void ProductionSignal(int);
private slots:
    /* 自定义槽函数 */
    void ProductionResponse(int,int);   //单灯测试回复
    void UpdateThreadSlots(int,void*);  //升级相关
    void SendData(void*,int*);          //子线程发送数据
    void DisplaySlots(int cmd,const void *buff, int nLen);
    void ShowMouseRightButton(const QPoint);

    void cursorclean();
    void DLight_01();
    void DLight_02();
    void DLight_03();
    void DLight_04();
    void DLight_05();
    void DLight_06();
    void DLight_07();
    void DLight_08();
    void DLight_09();
    void DLight_0A();
    void DLight_0B();
    void DLight_0C();
    void DLight_0D();
    void DLight_0E();
    void DLight_0F();
    void DLight_10();
    void DLight_11();
    void DLight_12();
    void DLight_13();
    void DLight_14();
    void DLight_15();
    void DLight_16();
    void DLight_17();
    void DLight_18();
    void DLight_19();
    void DLight_1A();
    void DLight_1B();
    void DLight_1C();
    void DLight_1D();



    /* 系统自带槽函数 */
    void readyReadSlot();
    void on_toolButton_clicked();
    void on_btn_opencom_clicked();
    void on_btn_Update_clicked();
    void on_btn_test_clicked();
    void on_btn_clean_clicked();
    void on_btn_Send_clicked();
    void on_btn_WriteInfo_clicked();
    void on_btn_Open_clicked();
    void on_btn_Close_clicked();
    void on_btn_ReadBound_clicked();
    void on_btn_ReadAddr_clicked();
    void on_btn_Readelec_clicked();
    void on_btn_ReadVol_clicked();
    void on_btn_ReadPower_clicked();

    void on_ck_newtalk_clicked();

private:
    Ui::SingleUp *ui;
    bool Is_connect;

    bool isRunning   = false;
    bool startProduc = false;

};

#endif // SINGLEUP_H
