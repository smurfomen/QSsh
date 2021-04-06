#ifndef QSSHPULL_H
#define QSSHPULL_H


#include <qsshtask.h>
#include <libssh/libssh.h>
class QSshPull : public QSshTask
{
    Q_OBJECT
public:
    QSshPull(const QString & from_remotePath, const QString & to_localPath, ssh_session s);

    bool handle() override;

    QString from() const {
        return remotePath;
    }

    QString to() const {
        return localPath;
    }

private:
    QString remotePath;
    QString localPath;
    ssh_session m_session;

};

#endif // QSSHPULL_H
