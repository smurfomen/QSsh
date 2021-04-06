#ifndef QSSHTASK_H
#define QSSHTASK_H

#include <QObject>

class QSshTask : public QObject
{
    Q_OBJECT
public:
    QSshTask();

    virtual bool handle() = 0;

Q_SIGNALS:
    void error(QSshTask *);
    void success(QSshTask *);
};

#endif // QSSHTASK_H
