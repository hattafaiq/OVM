#ifndef DATA_H
#define DATA_H

#include <QUdpSocket>
#include <QTimer>
#include <QObject>
#include <QList>
#include <QByteArray>
#include <QWebSocket>
#include <QTimer>
#include <QUdpSocket>
#include <QDate>
#include <QString>
#include "QWebSocketServer"
#include "setting.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSqlDatabase>
#include <threader.h>
#include "database.h"
#include <QVector>

class data : public QObject
{
    Q_OBJECT
public:
    QByteArray simpanan;
    explicit data(QObject *parent = nullptr);
    void req_UDP();
    virtual ~data();
    unsigned short spsX;
    int kirim;
    void set_memory();
    void free_memory();
    void init_setting(init_setting_k *Temp);
    void cek_settings(init_setting_k *Temp);
    //struct init_setting_k *Temp;


signals:

public slots:
    void readyReady(); //(QByteArray datagram);//data tidak mau masuk
    void init_time();
    void refresh_plot();
    void datamanagement();
   // void datamanagement2();
    void start_database();
    void flagdatabase();
    void flagclient();


private slots:
    void onNewConnection();
    void processMessage(QByteArray message);
    void socketDisconnected();
    void showTime();
    void sendDataClient1(QByteArray isipesan);
   // void sendDataClient2(QString isipesan2);


private:
    //
    QVector <float> vec;
    struct init_setting_k tmp, *Temp;
    int flagsave;
    int flagtimestart;

    int kirimclient;
    int penuh;
    int penuh2;
    unsigned int panjang_buffer_waveform;
    unsigned int panjang_buffer_spektrum;
    int period_simpan;
    int spektrum_points;
    int paket_dikirim;
    float *data_save[8];
    float *data_get[8];
    float *data_prekirim[8];
    float *data_kirim1[2560];
    float *data_kirim2[2560];
    float *data_kirim3[2560];
    float *data_kirim4[2560];
    float *data_kirim5[2560];
    float *data_kirim6[2560];
    float *data_kirim7[2560];
    float *data_kirim8[2560];
    float data_client[20480];
    int cnt_ch[8];
    int cnt_cha[8];

    int counterCH1;
    ///
    int pernah_penuh;
    //timer
    QTimer *timer;
    QTimer *timera;
    QTimer *TMclient;

    //websocket
    QWebSocketServer *m_pWebSocketServer1;
    QList<QWebSocket *> CG_NewClient;
    QList<QWebSocket *> CG_PenSub;
    QList<QWebSocket *> Subcribe_wave1;
    QList<QWebSocket *> Subcribe_wave2;

    QWebSocket *ind_Client;
    QWebSocket *C_NewCon;

    QWebSocket *pClient1;
    QWebSocket *pClientkirim;

    QString *datas;
    QTimer *jam;
    QTimer *timers;
    QDate date;
    QString dateTimeText;
    QString time_text;
    //
    //parsing UDP
    QByteArray datagram;
    quint16 senderPort;
    QHostAddress sendera;
    QUdpSocket *socket;
    int tim_count;
    // inisial data
    double *data_y_voltage[JUM_KANAL];
    float kanal1[2560];
    float kanal2[2560];
    float kanal3[2560];
    float kanal4[2560];
    float kanal5[2560];
    float kanal6[2560];
    float kanal7[2560];
    float kanal8[2560];
    int counterK1;
    int counterK2;
    int counterK3;
    int counterK4;
    int counterK5;
    int counterK6;
    int counterK7;
    int counterK8;
    int flagK1;
    int flagK2;
    int flagK3;
    int flagK4;
    int flagK5;
    int flagK6;
    int flagK7;
    int flagK8;
    // SQL
//    QSqlDatabase m_db;
//    QString simpanaja;
//    static QList<QWebSocket*> plist;
    threader *threadku;
    database *dbase;
    int count_db;


};
#endif // DATA_H
