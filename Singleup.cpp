#include "Singleup.h"
#include "ui_Singleup.h"
#include <QTime>
#include <QMenu>
u8   Readbuf[ReadbufSize];
int  ReadPtr;
int  WritePtr;
volatile bool Signal_OK;
volatile bool ProductionRun;
volatile bool Is_Display;
volatile bool DisplayOnlab;
int CoordiAddr;
int SingleAddr;
int K_Vol;
int K_Elec;
int Baud;

void delayms(int ms)
{
    int x = 10000,y = 300000;
    while(ms >0){
        ms--;
        while(x>0){
            x--;
            while(y>0){
                y--;
            }
        }
    }
}

void Mysleep(unsigned int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void ClearBuf(void)
{
    ReadPtr  = 0;
    WritePtr = 0;
    memset(Readbuf,0,sizeof(Readbuf));
}

void WriteData(QByteArray str)
{
    int strlen = str.length();
    int i = 0;
    int nn = WritePtr;
    while(i < strlen){
        if(WritePtr == ReadPtr-1){
            cout<<"buf was full!"<<endl;
            while(WritePtr == ReadPtr-1);   //等待缓存区可以写
        }
        Readbuf[WritePtr++] = str.toStdString()[i++];
        WritePtr %= ReadbufSize;
    }
    printf("write len:%d,Write buf data:",strlen);
    while(nn < WritePtr){
        printf("%02x ",Readbuf[nn++]);
    }printf("\n");
}

int  ReadData(void *data)
{
    if(ReadPtr == WritePtr){
        return 0;
    }
    *(char*)data = Readbuf[ReadPtr++];
    ReadPtr %= ReadbufSize;
    return 1;
}

int  Recv(char*RecvBuf,int nLen,int block)
{
    int n = 0;
    while(n < nLen && block != 0){
        if(ReadData(RecvBuf)){
            ++n;
            ++RecvBuf;
            continue;
        }
        QThread::msleep(1);
        if(block > 0){--block;}
    }
    if(block <= 0){
        return -1;
    }
    return n;
}

unsigned char Recv_OneByte(void)
{
    unsigned char Response = 0;
    return Recv((char*)&Response,1,1000) == -1 ?FAIL:Response;
}

SingleUp::SingleUp(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SingleUp)
{
    ui->setupUi(this);

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()){
        ui->cb_com->addItem(info.portName());
    }

    thred_update        = new UpdateThread();
    thread_Production   = new Production();

    CoordiAddr = 0;
    SingleAddr = 0;
    K_Vol      = 0;
    K_Elec     = 0;
    Baud       = 0;
    Is_connect    = false;
    ProductionRun = false;
    DisplayOnlab  = false;

    Is_Display  = true;
    isRunning   = false;
    startProduc = false;

    ReadPtr  = 0;
    WritePtr = 0;
    memset(Readbuf,0,sizeof(Readbuf));
    Filefd.setFileMode(QFileDialog::AnyFile);
    Filefd.setViewMode(QFileDialog::Detail);
    connect(&MyCom,SIGNAL(readyRead()), this, SLOT(readyReadSlot()));
    connect(thred_update,SIGNAL(UpdateSignals(int,void*)),this,SLOT(UpdateThreadSlots(int,void*)));
    connect(thred_update,SIGNAL(SendSignal(void*,int*)),this,SLOT(SendData(void*,int*)));

    connect(thread_Production,SIGNAL(SendSignal(void*,int*)),this,SLOT(SendData(void*,int*)));
    connect(thread_Production,SIGNAL(ResponseSignal(int,int)),this,SLOT(ProductionResponse(int,int)));
    connect(this,SIGNAL(ProductionSignal(int)),thread_Production,SLOT(ProductionSlots(int)));
    connect(thread_Production,SIGNAL(DisplayInfo(int,const void*,int)),this,SLOT(DisplaySlots(int,const void*,int)));

    ui->Info_Show->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->Info_Show,SIGNAL(customContextMenuRequested(const QPoint)),this,SLOT(ShowMouseRightButton(const QPoint)));
}

SingleUp::~SingleUp()
{
    delete ui;
    delete thred_update;
    delete thread_Production;
}

void SingleUp::ShowMouseRightButton(const QPoint pos)
{
    int i = 0;
    QMenu qmenu;

    QAction clean;
    QAction Copy;
    clean.setText("清除");
    Copy.setText("复制");
    qmenu.addAction(&Copy);
    qmenu.addAction(&clean);

    QMenu light;
    light.setTitle("调光");
    qmenu.addMenu(&light);

    QAction Light[0X1D];
    char buf[32];
    while(i < 0x1D){
        memset(buf,0,sizeof(buf));
        sprintf(buf,"0x%02X",i);
        Light[i].setText(buf);
        light.addAction(&Light[i]);
        ++i;
    }
    qmenu.addAction(&Light[15]);
    connect(&Light[0],SIGNAL(triggered()),this,SLOT(DLight_01()));
    connect(&Light[1],SIGNAL(triggered()),this,SLOT(DLight_02()));
    connect(&Light[2],SIGNAL(triggered()),this,SLOT(DLight_03()));
    connect(&Light[3],SIGNAL(triggered()),this,SLOT(DLight_04()));
    connect(&Light[4],SIGNAL(triggered()),this,SLOT(DLight_05()));
    connect(&Light[5],SIGNAL(triggered()),this,SLOT(DLight_06()));
    connect(&Light[6],SIGNAL(triggered()),this,SLOT(DLight_07()));
    connect(&Light[7],SIGNAL(triggered()),this,SLOT(DLight_08()));
    connect(&Light[8],SIGNAL(triggered()),this,SLOT(DLight_09()));
    connect(&Light[9],SIGNAL(triggered()),this,SLOT(DLight_0A()));
    connect(&Light[10],SIGNAL(triggered()),this,SLOT(DLight_0B()));
    connect(&Light[11],SIGNAL(triggered()),this,SLOT(DLight_0C()));
    connect(&Light[12],SIGNAL(triggered()),this,SLOT(DLight_0D()));
    connect(&Light[13],SIGNAL(triggered()),this,SLOT(DLight_0E()));
    connect(&Light[14],SIGNAL(triggered()),this,SLOT(DLight_0F()));
    connect(&Light[15],SIGNAL(triggered()),this,SLOT(DLight_10()));
    connect(&Light[16],SIGNAL(triggered()),this,SLOT(DLight_11()));
    connect(&Light[17],SIGNAL(triggered()),this,SLOT(DLight_12()));
    connect(&Light[18],SIGNAL(triggered()),this,SLOT(DLight_13()));
    connect(&Light[19],SIGNAL(triggered()),this,SLOT(DLight_14()));
    connect(&Light[20],SIGNAL(triggered()),this,SLOT(DLight_15()));
    connect(&Light[21],SIGNAL(triggered()),this,SLOT(DLight_16()));
    connect(&Light[22],SIGNAL(triggered()),this,SLOT(DLight_17()));
    connect(&Light[23],SIGNAL(triggered()),this,SLOT(DLight_18()));
    connect(&Light[24],SIGNAL(triggered()),this,SLOT(DLight_19()));
    connect(&Light[25],SIGNAL(triggered()),this,SLOT(DLight_1A()));
    connect(&Light[26],SIGNAL(triggered()),this,SLOT(DLight_1B()));
    connect(&Light[27],SIGNAL(triggered()),this,SLOT(DLight_1C()));
    connect(&Light[28],SIGNAL(triggered()),this,SLOT(DLight_1D()));

    connect(&clean,SIGNAL(triggered()),this,SLOT(cursorclean()));
    qmenu.exec(QCursor::pos());
    cout<<pos.x();
}

void SingleUp::Testlight(int light)
{
    Protocol package;
    memset(&package,0,sizeof(package));
    package.Crtl            = 0x10;
    package.Cmd[0]          = 0x00;
    package.Cmd[1]          = 0x01;
    package.Data[0]         = light;//调光值
    Crc16(package.Crc16,(u8*)&package,sizeof(package)-2);
    MyCom.write((char*)&package,sizeof(package));
}
void SingleUp::cursorclean(){ ui->Info_Show->clear();}
void SingleUp::DLight_01(){Testlight(0x00);}
void SingleUp::DLight_02(){Testlight(0x01);}
void SingleUp::DLight_03(){Testlight(0x02);}
void SingleUp::DLight_04(){Testlight(0x03);}
void SingleUp::DLight_05(){Testlight(0x04);}
void SingleUp::DLight_06(){Testlight(0x05);}
void SingleUp::DLight_07(){Testlight(0x06);}
void SingleUp::DLight_08(){Testlight(0x07);}
void SingleUp::DLight_09(){Testlight(0x08);}
void SingleUp::DLight_0A(){Testlight(0x09);}
void SingleUp::DLight_0B(){Testlight(0x0A);}
void SingleUp::DLight_0C(){Testlight(0x0B);}
void SingleUp::DLight_0D(){Testlight(0x0C);}
void SingleUp::DLight_0E(){Testlight(0x0D);}
void SingleUp::DLight_0F(){Testlight(0x0E);}
void SingleUp::DLight_10(){Testlight(0x0F);}
void SingleUp::DLight_11(){Testlight(0x10);}
void SingleUp::DLight_12(){Testlight(0x11);}
void SingleUp::DLight_13(){Testlight(0x12);}
void SingleUp::DLight_14(){Testlight(0x13);}
void SingleUp::DLight_15(){Testlight(0x14);}
void SingleUp::DLight_16(){Testlight(0x15);}
void SingleUp::DLight_17(){Testlight(0x16);}
void SingleUp::DLight_18(){Testlight(0x17);}
void SingleUp::DLight_19(){Testlight(0x18);}
void SingleUp::DLight_1A(){Testlight(0x19);}
void SingleUp::DLight_1B(){Testlight(0x1A);}
void SingleUp::DLight_1C(){Testlight(0x1B);}
void SingleUp::DLight_1D(){Testlight(0x1C);}


bool SingleUp::Open_Uart(void)
{
    /* 串口号 */
    MyCom.setPortName(ui->cb_com->currentText());
    /* 波特率 */
    MyCom.setBaudRate(ui->cb_bound->currentText().toInt());
    /* 检验位 */
    if(ui->cb_check->currentText()=="N"){
         MyCom.setParity(QSerialPort::NoParity);
    }else if(ui->cb_check->currentText()=="E"){
         MyCom.setParity(QSerialPort::EvenParity);
    }else{
         MyCom.setParity(QSerialPort::SpaceParity);
    }
    /* 数据位 */
    switch(ui->cb_bits->currentText().toInt()){
        case 5: MyCom.setDataBits(QSerialPort::Data5); break;
        case 6: MyCom.setDataBits(QSerialPort::Data6); break;
        case 7: MyCom.setDataBits(QSerialPort::Data7); break;
        case 8: MyCom.setDataBits(QSerialPort::Data8); break;
        default:MyCom.setDataBits(QSerialPort::UnknownDataBits);break;
    }
    /* 停止位 */
    if(ui->cb_stop->currentText() == "0"){
        MyCom.setStopBits(QSerialPort::OneStop);
    }else if(ui->cb_stop->currentText() == "1.5"){
        MyCom.setStopBits(QSerialPort::OneAndHalfStop);
    }else{
        MyCom.setStopBits(QSerialPort::TwoStop);
    }
    /* 流控 */
    MyCom.setFlowControl(QSerialPort::NoFlowControl);
    /* 串口的读缓存 */
    MyCom.setReadBufferSize(2048);
    if(MyCom.open(QSerialPort::ReadWrite)){
        cout<<"Open "<<ui->cb_com->currentText().toStdString()<<" Success!"<<endl;
        Is_connect = true;
        ui->btn_opencom->setText("关闭串口");
        return true;
    }else{
        QMessageBox::information(NULL,"Error!","Open Serial fail!  ",QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
        Is_connect = false;
        ui->btn_opencom->setText("打开串口");
        cout<<"Open "<<ui->cb_com->currentText().toStdString()<<" fail!"<<endl;
        return false;
    }

}

void SingleUp::Close_Uart(void)
{
    MyCom.close();
    ui->btn_opencom->setText("打开串口");
    Is_connect = false;
}

int SingleUp::Send(const void *Sendbuf,int nLen)
{
    int len = 0;
    int i   = 0;
    const char*Pbuf = (const char*)Sendbuf;

    while(1){
        i = MyCom.write(&Pbuf[len],nLen-len);
        if(-1 == i){
            return -1;
        }else{
            len += i;
            if(nLen == len)
                break;
        }
    }
    return len;
}

void SingleUp::HexToStr(char *pbDest,unsigned char *pbSrc, int nLen)
{
    char table[]="0123456789ABCDEF";
    while(nLen--){
        *pbDest++ = table[(*pbSrc>>4)&0x0f];
        *pbDest++ = table[*pbSrc&0x0f];
        ++pbSrc;
    }*pbDest = '\0';
}

void SingleUp::StrToHex(unsigned char *pbDest, char *pbSrc, int nLen)
{
    char h1,h2;
    char s1,s2;
    int i;
    for (i=0; i<nLen; i++){
        h1 = pbSrc[2*i];
        h2 = pbSrc[2*i+1];
        s1 = toupper(h1) - '0';
        if (s1 > 9)
            s1 -= 7;

        s2 = toupper(h2) - '0';
        if (s2 > 9)
            s2 -= 7;

        pbDest[i] = 0xff&(s1*16 + s2);
    }
}

/* 自定义槽函数 */
void SingleUp::UpdateThreadSlots(int cmd,void *str)
{
    u8 Addr[2];
    QByteArray Text;
    char buf[24];
    switch(cmd){
        case Update_Message:
            ui->Info_Show->append((char*)str);break;
        case Update_GetNewtalk:
            //if(ui->ck_newtalk->checkState() == Qt::Checked){
                *(int*)str = 1;
            //}else{
            //    *(int*)str = 0;
            //}break;
        case Update_IsCoordi:
            if(ui->ck_optcoordi->checkState() == Qt::Checked){
                *(int*)str = 1;
            }else{
                *(int*)str = 0;
            }break;
        case Update_GetFileName:
            if( QMessageBox::information(NULL, "error!", "update file not exist!\ndo you want chose update file?",
                                         QMessageBox::Yes | QMessageBox::No,QMessageBox::Yes) == QMessageBox::Yes){
                if(Filefd.exec() == QDialog::Accepted ){  //如果成功的执行
                    UpdateFilename = Filefd.selectedFiles()[0];
                    ui->lab_show->setText(UpdateFilename);
                }else Filefd.close();
                strcpy((char*)str,UpdateFilename.toStdString().c_str());
                strcpy((char*)str,UpdateFilename.toStdString().c_str());
            }else{
                ui->Info_Show->append("have no update file!");
            }
            break;
        case Update_SetprogressMax:
            ui->pgb_update->setMaximum(*(int*)str);
            ui->pgb_update->setMinimum(0);
        case Update_SetprogressValues:
            ui->pgb_update->setValue(*(int*)str);
        case Update_GetCoordiAddr:
            memset(buf,0,sizeof(buf));
            Text = ui->le_CoordiAddr->text().toLatin1();
            strcpy(buf,Text.toStdString().c_str());
            StrToHex(Addr,buf,strlen(buf)/2);
            *(int*)str = Addr[0];
            //printf("^^^^0x%x\n",*(int*)str);
            break;
        case Update_GetSingleAddr:
            memset(buf,0,sizeof(buf));
            Text = ui->le_SingleAddr->text().toLatin1();
            strcpy(buf,Text.toStdString().c_str());
            StrToHex(Addr,buf,strlen(buf)/2);
            *(int*)str = (int)((Addr[0]<<8) | Addr[1]);
            //printf("^^^^0x%x\n",*(int*)str);
            break;
        case Update_finish:
            ui->btn_Update->setText("升级程序");
            isRunning = false;
            Is_Display = true;
            ui->btn_Close->setEnabled(true);
            ui->btn_Open->setEnabled(true);
            ui->btn_ReadAddr->setEnabled(true);
            ui->btn_ReadBound->setEnabled(true);
            ui->btn_Readelec->setEnabled(true);
            ui->btn_ReadPower->setEnabled(true);
            ui->btn_ReadVol->setEnabled(true);
            ui->btn_test->setEnabled(true);
            ui->btn_WriteInfo->setEnabled(true);
            ui->btn_Send->setEnabled(true);
            ui->ck_newtalk->setEnabled(true);
            break;
        default:break;
    }
    Signal_OK = true;
}

void SingleUp::ProductionResponse(int cmd,int data)
{
    char buf[24];
    switch(cmd){
        case WriteBaseInfo:
            if(data){
                ui->radio_baseinfo->setStyleSheet("border-image: url(:/on.png)");
            }else{
                ui->radio_baseinfo->setStyleSheet("border-image: url(:/off.png)");
            }
            break;
        case OpenLight:
            if(data){
                ui->radio_open->setStyleSheet("border-image: url(:/on.png)");
            }else{
                ui->radio_open->setStyleSheet("border-image: url(:/off.png)");
            }
            break;
        case CloseLight:
            if(data){
                ui->radio_close->setStyleSheet("border-image: url(:/on.png)");
            }else{
                ui->radio_close->setStyleSheet("border-image: url(:/off.png)");
            }
            break;
        case ReadBaud:
            if(data){
                ui->radio_bound->setStyleSheet("border-image: url(:/on.png)");
            }else{
                ui->radio_bound->setStyleSheet("border-image: url(:/off.png)");
            }
            break;
        case ReadAddr:
            if(data){
                memset(buf,0,sizeof(buf));
                sprintf(buf,"0X%04x",data);
                ui->lab_ReadAddr->setText(buf);
            }else{
                ui->lab_ReadAddr->setText("Unknow!");
            }
            break;
        case ReadVol:
            if(data){
                memset(buf,0,sizeof(buf));
                sprintf(buf,"%.2f V",data/100.0);
                ui->lab_ReadVol->setText(buf);
            }else{
                ui->lab_ReadVol->setText("0.0000 V!");
            }
            break;
        case ReadElec:
            if(data){
                memset(buf,0,sizeof(buf));
                sprintf(buf,"%.2f mA",data/100.0);
                ui->lab_Readelec->setText(buf);
            }else{
                ui->lab_Readelec->setText("0.0000 mA!");
            }
            break;
        case ReadPower:
            if(data){
                memset(buf,0,sizeof(buf));
                sprintf(buf,"%.4f W",data/1000.0);
                ui->lab_ReadPower->setText(buf);
            }else{
                ui->lab_ReadPower->setText("0.0000 W!");
            }
            break;
        default:break;
    }
}

void SingleUp::SendData(void *buf,int*nLen)
{
    int len;
    len   = Send(buf,*nLen);
    *nLen = len;
    Signal_OK = true;
}

void SingleUp::DisplaySlots(int cmd,const void *buff, int nLen)
{
    QString str;
    char *Pbuf = (char*)malloc(nLen*2+1);
    if(NULL == Pbuf){
        cout << "malloc err in DisplaySlots!" << endl;
        return ;
    }
    memset(Pbuf,0,nLen*2+1);
    switch(cmd){
        case Dsplay_onlab:break;
        case Dsplay_onwindow:
            str = (char*)buff;
            ui->Info_Show->append(str);
        break;
        case Dsplay_Hexonlab:
             HexToStr(Pbuf,(unsigned char*)buff,nLen);
             while(*Pbuf != '\0'){
                str += *Pbuf++;
                str += *Pbuf++;
                str += " ";
             }
             ui->lab_show->setText(str);
        break;
        case Dsplay_Hexonwindow:
            HexToStr(Pbuf,(unsigned char*)buff,nLen);
            while(*Pbuf != '\0'){
               str += *Pbuf++;
               str += *Pbuf++;
               str += " ";
            }
            ui->Info_Show->append(str);
        break;
    }
    Signal_OK = true;
    free(Pbuf);
}

/* 系统自带槽函数 */
void SingleUp::readyReadSlot()
{
    char StrBuf[128];
    unsigned char HexBuf[64];
    QByteArray str = MyCom.readAll();
    QString sstr = "";
    WriteData(str);
    int i = 0;

    if(ui->ck_displayhex->checkState() == Qt::Checked){//16进账显示
        if(Is_Display){
            memset(StrBuf,0,sizeof(StrBuf));
            memset(HexBuf,0,sizeof(HexBuf));
            memcpy(HexBuf,str.toStdString().c_str(),str.length());
            HexToStr(StrBuf, HexBuf,str.length());
            while(StrBuf[i] !='\0'){
                sstr += StrBuf[i++];
                sstr += StrBuf[i++];
                sstr += " ";
            }
            if(ui->ck_autoenter->checkState() == Qt::Checked)
                sstr += "\n";
            ui->Info_Show->insertPlainText(sstr);
        }

    }else{
        if(Is_Display){
            if(ui->ck_autoenter->checkState() == Qt::Checked)
                ui->Info_Show->append(str);
            else
                ui->Info_Show->insertPlainText(str);
        }
    }
}

void SingleUp::on_toolButton_clicked()
{
    if(Filefd.exec() == QDialog::Accepted ){  //如果成功的执行
        UpdateFilename = Filefd.selectedFiles()[0];
        cout<<"file name:"<<UpdateFilename.toStdString()<<endl;
        ui->lab_show->setText(UpdateFilename);
    }else Filefd.close();
}

void SingleUp::on_btn_opencom_clicked()
{
    if(false == Is_connect){
            ui->cb_bound->setEnabled(false);
            ui->cb_com->setEnabled(false);
            ui->cb_bits->setEnabled(false);
            ui->cb_check->setEnabled(false);
            ui->cb_stop->setEnabled(false);
            if(Open_Uart()){
                ui->btn_Close->setEnabled(true);
                ui->btn_Open->setEnabled(true);
                ui->btn_ReadAddr->setEnabled(true);
                ui->btn_ReadBound->setEnabled(true);
                ui->btn_Readelec->setEnabled(true);
                ui->btn_ReadPower->setEnabled(true);
                ui->btn_ReadVol->setEnabled(true);
                ui->btn_test->setEnabled(true);
                ui->btn_WriteInfo->setEnabled(true);
                ui->btn_Update->setEnabled(true);
                ui->btn_Send->setEnabled(true);
                ui->le_send->setEnabled(true);
                ui->toolButton->setEnabled(true);
                ui->ck_newtalk->setEnabled(true);
            }else{
                ui->cb_bound->setEnabled(true);
                ui->cb_com->setEnabled(true);
                ui->cb_bits->setEnabled(true);
                ui->cb_check->setEnabled(true);
                ui->cb_stop->setEnabled(true);
            }

        }else{
            Close_Uart();
            ui->cb_bound->setEnabled(true);
            ui->cb_com->setEnabled(true);
            ui->cb_bits->setEnabled(true);
            ui->cb_check->setEnabled(true);
            ui->cb_stop->setEnabled(true);

            ui->btn_Close->setEnabled(false);
            ui->btn_Open->setEnabled(false);
            ui->btn_ReadAddr->setEnabled(false);
            ui->btn_ReadBound->setEnabled(false);
            ui->btn_Readelec->setEnabled(false);
            ui->btn_ReadPower->setEnabled(false);
            ui->ck_newtalk->setEnabled(false);
            ui->btn_ReadVol->setEnabled(false);
            ui->btn_WriteInfo->setEnabled(false);
            ui->btn_Send->setEnabled(false);
            ui->le_send->setEnabled(false);
            ui->toolButton->setEnabled(false);
            startProduc = true;
            on_btn_WriteInfo_clicked();
            ui->btn_test->setEnabled(false);
            ui->btn_Update->setEnabled(false);
        }
}

void SingleUp::on_btn_Update_clicked()
{
    if(!isRunning){
        Is_Display = false;
        ui->btn_Update->setText("升级中...");
        thred_update->start();
        isRunning = true;

        ui->btn_Close->setEnabled(false);
        ui->btn_Open->setEnabled(false);
        ui->btn_ReadAddr->setEnabled(false);
        ui->btn_ReadBound->setEnabled(false);
        ui->btn_Readelec->setEnabled(false);
        ui->btn_ReadPower->setEnabled(false);
        ui->btn_ReadVol->setEnabled(false);
        ui->ck_newtalk->setEnabled(false);
        ui->btn_test->setEnabled(false);
        ui->btn_WriteInfo->setEnabled(false);
        ui->btn_Send->setEnabled(false);
    }else{
        ui->btn_Update->setText("升级程序");
        isRunning = false;
        Is_Display = true;
        thred_update->quit();

        ui->btn_Close->setEnabled(true);
        ui->btn_Open->setEnabled(true);
        ui->btn_ReadAddr->setEnabled(true);
        ui->btn_ReadBound->setEnabled(true);
        ui->btn_Readelec->setEnabled(true);
        ui->btn_ReadPower->setEnabled(true);
        ui->btn_ReadVol->setEnabled(true);
        ui->ck_newtalk->setEnabled(true);
        ui->btn_test->setEnabled(true);
        ui->btn_WriteInfo->setEnabled(true);
        ui->btn_Send->setEnabled(true);
    }
}

void SingleUp::on_btn_clean_clicked()
{
    ui->Info_Show->setText("");
}

void SingleUp::on_btn_Send_clicked()
{
    unsigned int i = 0;
    char StrBuf[128];
    unsigned char HexBuf[64];
    memset(StrBuf,0,sizeof(StrBuf));
    strcpy(StrBuf,ui->le_send->text().remove(" ").toStdString().c_str());
    cout<<"StrBuf:"<<StrBuf<<",StrBuf len:"<<strlen(StrBuf)<<endl;
    if(ui->ck_sendHex->checkState() == Qt::Checked){
        memset(HexBuf,0,sizeof(HexBuf));
        StrToHex(HexBuf,StrBuf,strlen(StrBuf)/2);
        printf("HexBuf:");
        while(i < strlen(StrBuf)/2){
            printf("%02X ",HexBuf[i++]);
        }printf("\n");
        Send(HexBuf,strlen(StrBuf)/2);
    }else{
        Send(StrBuf,strlen(StrBuf));
    }
}

void SingleUp::on_btn_WriteInfo_clicked()
{
    u8 Addr[2];
    QByteArray Text;
    char buf[24];

    if(!startProduc){
        startProduc = true;
        ui->btn_WriteInfo->setText("停止测试");
        /* 获取协调器地址 */
        memset(buf,0,sizeof(buf));
        Text = ui->le_CoordiAddr->text().toLatin1();
        strcpy(buf,Text.toStdString().c_str());
        StrToHex(Addr,buf,strlen(buf)/2);
        CoordiAddr = Addr[0];
        cout<<"Coordi Addr:0x"<<hex<<CoordiAddr<<endl;

        /* 获取单灯地址 */
        memset(buf,0,sizeof(buf));
        Text = ui->le_SingleAddr->text().toLatin1();
        strcpy(buf,Text.toStdString().c_str());
        StrToHex(Addr,buf,strlen(buf)/2);
        SingleAddr = (int)((Addr[0]<<8) | Addr[1]);
        cout<<"single Addr:0x"<<hex<<SingleAddr<<endl;

        K_Vol  = ui->le_voltage->text().toInt();
        K_Elec = ui->le_electric->text().toInt();
        Baud   = ui->le_SingleBound->text().toInt();
        cout<<"vol:"<<dec<<K_Vol<<",elec:"<<K_Elec<<",baud:"<<Baud<<endl;

        /* 启动线程 */
        ProductionRun = true;
        thread_Production->start();
        emit ProductionSignal(WriteBaseInfo);
        ui->btn_Update->setEnabled(false);
    }else{
        ui->btn_WriteInfo->setText("开始测试");
        startProduc   = false;
        ProductionRun = false;
        thread_Production->quit();
        ui->btn_Update->setEnabled(true);
    }
    ui->btn_test->setEnabled(true);
    ui->radio_baseinfo->setStyleSheet("border-image: url(:/off.png)");
    ui->radio_open->setStyleSheet("border-image: url(:/off.png)");
    ui->radio_close->setStyleSheet("border-image: url(:/off.png)");
    ui->radio_bound->setStyleSheet("border-image: url(:/off.png)");
    ui->lab_ReadAddr->setText("");
    ui->lab_ReadPower->setText("");
    ui->lab_Readelec->setText("");
    ui->lab_ReadVol->setText("");

}

void SingleUp::on_btn_Open_clicked()
{
    emit ProductionSignal(OpenLight);
}

void SingleUp::on_btn_Close_clicked()
{
    emit ProductionSignal(CloseLight);
}

void SingleUp::on_btn_ReadBound_clicked()
{
    emit ProductionSignal(ReadBaud);
}

void SingleUp::on_btn_ReadAddr_clicked()
{
    emit ProductionSignal(ReadAddr);
}

void SingleUp::on_btn_Readelec_clicked()
{
    emit ProductionSignal(ReadElec);
}

void SingleUp::on_btn_ReadVol_clicked()
{
    emit ProductionSignal(ReadVol);
}

void SingleUp::on_btn_ReadPower_clicked()
{
    emit ProductionSignal(ReadPower);
}

void SingleUp::on_ck_newtalk_clicked()
{
    emit ProductionSignal(AdjustLight);
}

void SingleUp::on_btn_test_clicked()
{
    u8 Addr[2];
    QByteArray Text;
    char buf[24];

    startProduc = true;
    ui->btn_WriteInfo->setText("停止测试");
    ui->radio_baseinfo->setStyleSheet("border-image: url(:/off.png)");
    ui->radio_open->setStyleSheet("border-image: url(:/off.png)");
    ui->radio_close->setStyleSheet("border-image: url(:/off.png)");
    ui->radio_bound->setStyleSheet("border-image: url(:/off.png)");
    ui->lab_ReadAddr->setText("");
    ui->lab_ReadPower->setText("");
    ui->lab_Readelec->setText("");
    ui->lab_ReadVol->setText("");

    /* 获取协调器地址 */
    memset(buf,0,sizeof(buf));
    Text = ui->le_CoordiAddr->text().toLatin1();
    strcpy(buf,Text.toStdString().c_str());
    StrToHex(Addr,buf,strlen(buf)/2);
    CoordiAddr = Addr[0];
    cout<<"Coordi Addr:0x"<<hex<<CoordiAddr<<endl;

    /* 获取单灯地址 */
    memset(buf,0,sizeof(buf));
    Text = ui->le_SingleAddr->text().toLatin1();
    strcpy(buf,Text.toStdString().c_str());
    StrToHex(Addr,buf,strlen(buf)/2);
    SingleAddr = (int)((Addr[0]<<8) | Addr[1]);
    cout<<"single Addr:0x"<<hex<<SingleAddr<<endl;

    /* 获取电流电压等信息 */
    K_Vol  = ui->le_voltage->text().toInt();
    K_Elec = ui->le_electric->text().toInt();
    Baud   = ui->le_SingleBound->text().toInt();
    cout<<"vol:"<<dec<<K_Vol<<",elec:"<<K_Elec<<",baud:"<<Baud<<endl;

    /* 启动线程 */
    ProductionRun = true;
    thread_Production->start();
    emit ProductionSignal(OneTest);
    ui->btn_Update->setEnabled(false);
}


