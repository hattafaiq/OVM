#include "data.h"
#include "setting.h"

extern struct d_global global;

data::data(QObject *parent) : QObject(parent)
{
  threadku = new threader();
  //spektrum_points = 6144;
  panjang_buffer_waveform= 10 * 10240 ;
  panjang_buffer_spektrum = MAX_FFT_POINT;
  Temp = new init_setting_k;
   //QFile input("/home/cloud/OVM/setting.ini");
    QFile input("/home/fh/runServer/setup/setting.ini");
    if(input.exists())
    {
       cek_settings(Temp);
    }
    else
    {
       init_setting(Temp);
    }
    count_db = 1;
    pernah_penuh = 0;
    flagsave=0;
    flagtimestart=0;
    counterCH1=0;
    kirimclient = 0;
    counterK1=0;
    counterK2=0;
    counterK3=0;
    counterK4=0;
    counterK5=0;
    counterK6=0;
    counterK7=0;
    counterK8=0;
    flagK1=0;
    flagK2=0;
    flagK3=0;
    flagK4=0;
    flagK5=0;
    flagK6=0;
    flagK7=0;
    flagK8=0;
    spektrum_points = 2.56 * Temp->line_dbSpect;
    //INIT_udp
    socket = new QUdpSocket(this);
    qDebug()<<"status awal "<<socket->state();
    socket->bind(QHostAddress::Any, 5008);
    qDebug()<<"status bind "<<socket->state();
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyReady()));
    qDebug()<<"status connect "<<socket->state();
    //INIT_websocket
    m_pWebSocketServer1 = new QWebSocketServer(QStringLiteral("OVM"),QWebSocketServer::NonSecureMode,this);
    m_pWebSocketServer1->listen(QHostAddress::Any, 2121);
    connect(m_pWebSocketServer1, SIGNAL(newConnection()),this, SLOT(onNewConnection()));
    //INIT_database
    dbase->check_db_exist(tmp.dir_DB,count_db);

    set_memory();
    for(int i =0; i<JUM_KANAL;i++)
    {
        cnt_ch[i] = 0;
    }
        init_time();
}

data::~data()
{
    m_pWebSocketServer1->close();
    qDeleteAll(Subcribe_wave1.begin(), Subcribe_wave1.end());//paket10
    qDeleteAll(Subcribe_wave2.begin(), Subcribe_wave2.end());//paket10
    delete threadku;
    //delete Temp;
    free_memory();
}
void data::free_memory()
{
    int i;
    for (i = 0; i < JUM_KANAL; i++)
    {
        free(data_save[i]);
        free(data_get[i]);
    }
}
void data::cek_settings(init_setting_k *Temp)
{
    QString pth = "/home/cloud/OVM/setup/setting.ini";
    //QString pth = "/home/fh/runServer/setting.ini";
    QSettings settings(pth, QSettings::IniFormat);
    tmp.dir_ini = settings.value("Dir_ini").toString();
    tmp.modulIP1k = settings.value("IP1").toString();
    tmp.modulIP2k = settings.value("IP2").toString();
    Temp->fmax = settings.value("Fmax").toInt() ;
    Temp->timerdbk = settings.value("TimeSaveDB").toInt();
    Temp->timereq = settings.value("TimeReq").toInt();
    Temp->timerclient = settings.value("TimeClien").toInt();
    //Temp->sps = settings.value("SPS").toInt();
    Temp->line_dbSpect = settings.value("Lines").toInt();
    //Temp->jum_kanal = settings.value("JUM_KANAL").toInt();
    tmp.dir_DB = settings.value("Dir_DB").toString();
}

void data::flagdatabase()
{
    if(flagsave==1)
        {
            start_database();
            flagsave=0;
        }
    else
    {
        //qDebug()<<"data not save to database";
    }
}

void data::flagclient()
{
    if(flagtimestart==1)
    {
        datamanagement();
        flagtimestart=0;
    }
    else
    {
       // qDebug()<<"data not send to client";
    }
}

void data::init_setting(init_setting_k *Temp)
{
    QString pth = "/home/cloud/OVM/setup/setting.ini";
    //QString pth = "/home/fh/runServer/setting.ini";
    QSettings settings(pth, QSettings::IniFormat);
    qDebug()<<"tulis";

    memset((char *) Temp, 0, sizeof (struct init_setting_k));
    tmp.dir_DB = QString::fromUtf8("/home/cloud/OVM/setup/ovm_dbe");
      //tmp.dir_DB = QString::fromUtf8("/home/fh/runServer/ovm_dbe");
    //tmp.dir_ini = QString::fromUtf8("/home/fh/runServer/setting.ini");
    tmp.modulIP1k = QString::fromUtf8("192.168.0.101");
    tmp.modulIP2k = QString::fromUtf8("192.168.0.102");

    Temp->line_dbSpect = 800;
    Temp->fmax= 1000;
    Temp->timerdbk =5;
    //Temp->sps = 2560;
    Temp->timereq = 2;
    Temp->timerclient = 1;
    //Temp->jum_kanal = 8;
    //Temp->jum_kanal = settings.value("JUM_KANAL").toInt();
    //settings.setValue("JUM_KANAL",Temp->jum_kanal);
    settings.setValue("TimeClien",Temp->timerclient);
    settings.setValue("Dir_ini", tmp.dir_ini);
    settings.setValue("IP1",tmp.modulIP1k);
    settings.setValue("IP2",tmp.modulIP2k);
    settings.setValue("Dir_DB",tmp.dir_DB);
    settings.setValue("Lines",Temp->line_dbSpect);
    settings.setValue("Fmax",Temp->fmax);
    settings.setValue("TimeSaveDB",Temp->timerdbk);
    settings.setValue("TimeReq",Temp->timereq);
   // settings.setValue("SPS",Temp->sps);
//    qDebug()<<"selesai tulis";
//    qDebug() << "==============================<>";
//    qDebug() << Temp->modulIP1k << " data ip1";
//    qDebug() << Temp->modulIP2k << " data ip1";
//    qDebug() << Temp->fmax << " data Fmax";
//    qDebug() << Temp->timerdbk<< " data Interval DB";
//    qDebug() << Temp->timereq<< " time request";
//    qDebug() << Temp->sps<< " sps";
//    qDebug() << Temp->line_dbSpect<< "len data wave";
//    qDebug() << Temp->dir_DB<< " directory database";
//    qDebug() << "==============================<>";

}

void data::set_memory()
{
    int i;
    for (i=0; i<JUM_KANAL; i++)
    {
        data_save[i] = (float *) malloc((((2560/BESAR_PAKET_F)*JUM_KANAL)*BESAR_PAKET*Temp->timerdbk*10) * sizeof(float));
        memset( (char *) data_save[i], 0, ((((2560/BESAR_PAKET_F)*JUM_KANAL)*BESAR_PAKET*Temp->timerdbk*10)*sizeof(float)));
        data_get[i] = (float *) malloc(MAX_FFT_POINT * sizeof(float));
        memset( (char *) data_get[i], 0, MAX_FFT_POINT * sizeof(float));
    }
}


void data::init_time()
{
    timer = new QTimer(this);
    QObject::connect(timer,SIGNAL(timeout()),this, SLOT(refresh_plot()));
    timer->start(Temp->timereq*1100);
    timera = new QTimer(this);
    QObject::connect(timera,SIGNAL(timeout()),this, SLOT(flagdatabase()));
    timera->start(Temp->timerdbk*1000);
    TMclient = new QTimer(this);
    QObject::connect(TMclient,SIGNAL(timeout()),this, SLOT(flagclient()));
    TMclient->start(Temp->timerclient*1000);
}

void data::req_UDP()
{
    QByteArray Data;
    Data.append("getdata");
    QHostAddress ip_modul_1, ip_modul_2;
    ip_modul_1.setAddress(tmp.modulIP1k);
    ip_modul_2.setAddress(tmp.modulIP2k);
    socket->writeDatagram(Data,ip_modul_1, 5006);
    socket->writeDatagram(Data,ip_modul_2, 5006);
}

void data::showTime()
{
    QTime time = QTime::currentTime();
    time_text = time.toString("hh:mm:ss:z");
}


void data::readyReady()
{
    struct tt_req2 *p_req2;
    float *p_data;
    int i_kanal;
    int req;
    int xsps;
    int i;
    int datakebutuhan;

    while (socket->hasPendingDatagrams())
    {
        datagram.resize(socket->pendingDatagramSize());
        socket->readDatagram(datagram.data(), datagram.size(), &sendera, &senderPort);
        QHostAddress ip_modul_1, ip_modul_2;
        ip_modul_1.setAddress(tmp.modulIP1k);
        ip_modul_2.setAddress(tmp.modulIP2k);
         p_req2 = (struct tt_req2 *) datagram.data();
         p_data = (float *) p_req2->buf;
         req = p_req2->request_sample;
         i_kanal = p_req2->cur_kanal;
         xsps = p_req2->sps;
         datakebutuhan = (xsps/BESAR_PAKET_F)+1;

        // penuh= ((req*8*PAKET_BUFF)%xsps)+1;
        // penuh= ((req*4*PAKET_BUFF)%xsps)+1;
         int datayangdiharapkan = ((800*2,56)/256*8);
         int datasyarat= 2560;
         int datayangdikirim=datakebutuhan;
         //int datayangdiharapkan=2048;
         penuh = (cnt_ch[i_kanal]%datasyarat)+1;
//         kirimclient = (cnt_ch[i_kanal]%datayangdikirim)+1;

         int no_module = -1;

         if(sendera.toIPv4Address() == ip_modul_1.toIPv4Address())
         {
             no_module = 0;
         }
         else if(sendera.toIPv4Address() == ip_modul_2.toIPv4Address())
         {
             i_kanal = i_kanal+4;
             no_module = 1;
         }
             if((no_module==0)||(no_module==1))
             {
                 for (i=0; i<BESAR_PAKET_F; i++)
                   {
                     // data_send[i]= p_data[i];
                      cnt_ch[i_kanal]++;
                      data_save[i_kanal][cnt_ch[i_kanal]] = p_data[i];
                     // data_client[(counterCH1-1)*BESAR_PAKET_F+i]=p_data[i];
                    }

                if(i_kanal==0)
                {
                    counterK1++;
                    if(counterK1<11)
                    {
                        for (int i=0; i<BESAR_PAKET_F; i++){
                        kanal1[(counterK1-1)*BESAR_PAKET_F+i]=p_data[i];
                        }
                    }
                }
                else if(i_kanal==1)
                {
                    counterK2++;
                    if(counterK2<11)
                    {
                        for (int i=0; i<BESAR_PAKET_F; i++){
                        kanal2[(counterK2-1)*BESAR_PAKET_F+i]=p_data[i];
                        }
                    }
                }
                else if(i_kanal==2)
                {
                    counterK3++;
                    if(counterK3<11)
                    {   for (int i=0; i<BESAR_PAKET_F; i++){
                        kanal3[(counterK3-1)*BESAR_PAKET_F+i]=p_data[i];
                        }
                    }
                }
                else if(i_kanal==3)
                {
                    counterK4++;
                    if(counterK4<11)
                    {   for (int i=0; i<BESAR_PAKET_F; i++){
                        kanal4[(counterK4-1)*BESAR_PAKET_F+i]=p_data[i];
                        }
                    }
                }
                else if(i_kanal==4)
                {
                    counterK5++;
                    if(counterK5<11)
                    {   for (int i=0; i<BESAR_PAKET_F; i++){
                        kanal5[(counterK5-1)*BESAR_PAKET_F+i]=p_data[i];
                        }
                    }
                }
                else if(i_kanal==5)
                {
                    counterK6++;
                    if(counterK6<11)
                    {
                        for (int i=0; i<BESAR_PAKET_F; i++){
                        kanal6[(counterK6-1)*BESAR_PAKET_F+i]=p_data[i];
                        }
                    }
                }
                else if(i_kanal==6)
                {
                    counterK7++;
                    if(counterK7<11)
                    {   for (int i=0; i<BESAR_PAKET_F; i++){
                        kanal7[(counterK7-1)*BESAR_PAKET_F+i]=p_data[i];
                        }
                    }
                }
                else if(i_kanal==7)
                {
                    counterK8++;
                    if(counterK8<11)
                    {   for (int i=0; i<BESAR_PAKET_F; i++){
                        kanal8[(counterK8-1)*BESAR_PAKET_F+i]=p_data[i];
                        }
                    }
                }
            }//ip
             if(penuh==1)
                {
                 //counterCH1=0;
                    flagsave=1;
                  //qDebug()<<"penuh simpan";
                }
             if((flagK1==1&&flagK2==1&&flagK3==1&&flagK4==1)||(flagK5==1&&flagK6==1&&flagK7==1&&flagK8==1))//counterCH1==80)
             {
                flagtimestart=1;
               //Debug()<<"penuh semua, siap kirim";
                flagK1=0;
                flagK2=0;
                flagK3=0;
                flagK4=0;
                flagK5=0;
                flagK6=0;
                flagK7=0;
                flagK8=0;
             }
             if(counterK1==10)
             {
                 flagK1=1;
                 counterK1=0;
                //Debug()<<"penuh k1";
             }
             if(counterK2==10)
             {
                 flagK2=1;
                 counterK2=0;
                //Debug()<<"penuh k2";
             }
             if(counterK3==10)
             {
                 flagK3=1;
                 counterK3=0;
                //Debug()<<"penuh k3";
             }
             if(counterK4==10)
             {
                 flagK4=1;
                 counterK4=0;
               //qDebug()<<"penuh k4";
             }
             if(counterK5==10)
             {
                 flagK5=1;
                 counterK5=0;
               //qDebug()<<"penuh k5";
             }
             if(counterK6==10)
             {
                 flagK6=1;
                 counterK6=0;
                //Debug()<<"penuh k6";
             }
             if(counterK7==10)
             {
                 flagK7=1;
                 counterK7=0;
               //qDebug()<<"penuh k7";
             }
             if(counterK8==10)
             {
                 flagK8=1;
                 counterK8=0;
               //qDebug()<<"penuh k8";
             }
    }// while
}//void


void data::datamanagement()
{
    struct kirim kri;
   // strcpy(kri.IP, "192.168.0.101");
   // kri.kanal1 = 2;
   // memcpy(kri.a, &data_client, sizeof(data_client));
    memcpy(kri.k1,&kanal1,sizeof(kanal1));
    memcpy(kri.k2,&kanal2,sizeof(kanal2));
    memcpy(kri.k3,&kanal3,sizeof(kanal3));
    memcpy(kri.k4,&kanal4,sizeof(kanal4));
    memcpy(kri.k5,&kanal5,sizeof(kanal5));
    memcpy(kri.k6,&kanal6,sizeof(kanal6));
    memcpy(kri.k7,&kanal7,sizeof(kanal7));
    memcpy(kri.k8,&kanal8,sizeof(kanal8));
    QByteArray datagrama = QByteArray(static_cast<char*>((void*)&kri), sizeof(kri));
  //qDebug()<<"data siap kirim"<<datagrama.size();
    sendDataClient1(datagrama);
    memset(kri.k1,0,sizeof(float)*2560);
    memset(kri.k2,0,sizeof(float)*2560);
    memset(kri.k3,0,sizeof(float)*2560);
    memset(kri.k4,0,sizeof(float)*2560);
    memset(kri.k5,0,sizeof(float)*2560);
    memset(kri.k6,0,sizeof(float)*2560);
    memset(kri.k7,0,sizeof(float)*2560);
    memset(kri.k8,0,sizeof(float)*2560);
}

void data::start_database()
{
    for(int i =0; i<JUM_KANAL; i++)//8
    {
      //  qDebug()<<"jumlah data kanal "<< i << "=" <<cnt_ch[i];
        if(cnt_ch[i] < spektrum_points)
        {
           // qDebug()<<cnt_ch[i]<<"data kurang dari spektrum";
            threadku->safe_to_save_ch[i] = 0;
            continue;
        }
        else
        {
            threadku->safe_to_save_ch[i] = 1;
        }
        memcpy(&data_get[i][0], &data_save[i][cnt_ch[i]-(spektrum_points)], spektrum_points * (sizeof(float)));
        QByteArray array0((char *) &data_get[i][0], spektrum_points * sizeof(float));
        threadku->bb1[i] = array0;
        threadku->ref_rpm = 6000;
        threadku->num = spektrum_points;
        threadku->fmax = Temp->fmax;

        threadku->start();

        array0.clear();
        }
        for(int i =0; i<JUM_KANAL; i++)//8
        {
            cnt_ch[i] =0;
        }
       //Debug()<<"data save ";
}

void data::refresh_plot()
{
    req_UDP();
    tim_count++;
}

void data::onNewConnection()
{
    C_NewCon = m_pWebSocketServer1->nextPendingConnection();
    connect(C_NewCon, &QWebSocket::binaryMessageReceived, this, &data::processMessage);
  //  connect(C_NewCon, &QWebSocket::textMessageReceived, this, &data::processMessage);
    connect(C_NewCon, &QWebSocket::disconnected, this, &data::socketDisconnected);
    //CG_NewClient << C_NewCon; //grup conneksi
}

void data::processMessage(QByteArray message)
{
    QWebSocket *C_NewReq = qobject_cast<QWebSocket *>(sender());
       // qDebug()<<message;

        QByteArray ba1;
        QByteArray bal1;
        QByteArray unsub;
        QString unsub_wave1 ="unsub";
        QString wave1 ="wave1";
        QString wave2 ="wave2";
        ba1 += wave1;
        bal1 += wave2;
        unsub += unsub_wave1;

        if((C_NewReq)&&(message==ba1))
            {
               Subcribe_wave1.removeAll(C_NewReq);
               Subcribe_wave1 << C_NewReq;
               // C_NewReq->sendBinaryMessage(ba1);
             //qDebug()<<"req wave 1 dari:"<<C_NewReq->peerAddress().toString();
            }
        if((C_NewReq)&&(message==unsub))
            {
                Subcribe_wave1.removeAll(C_NewReq);
               //Debug()<<"unsub scribe dari:"<<C_NewReq->peerAddress().toString();
            }

}

void data::sendDataClient1(QByteArray isipesan)
{
    Q_FOREACH (pClientkirim, Subcribe_wave1)//paket10
    {
        QHostAddress join=pClientkirim->peerAddress();
        QString joinstr=join.toString();
      //qDebug() << "kirim paket 1----ke : "<<joinstr;
        pClientkirim->sendBinaryMessage(isipesan);
    }
}

void data::socketDisconnected()
{
    pClient1 = qobject_cast<QWebSocket *>(sender());
    if (pClient1)
    {
        //pClient1->peerAddress();
        Subcribe_wave1.removeAll(pClient1);//paket10
        Subcribe_wave2.removeAll(pClient1);//paket10
        pClient1->deleteLater();
        C_NewCon->deleteLater();
        pClientkirim->deleteLater();
        QHostAddress join=pClient1->peerAddress();
        QString loststr=join.toString();
      //qDebug()<<"client loss" << loststr;
    }
}
