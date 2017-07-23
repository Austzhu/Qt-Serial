#ifndef QTHREAD_UPDATE_H
#define QTHREAD_UPDATE_H
#include <QThread>

class Qthread_Update : public QThread
{
public:
    Qthread_Update();
    void run();
};

#endif // QTHREAD_UPDATE_H
