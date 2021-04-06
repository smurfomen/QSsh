#ifndef QSSHPUSH_H
#define QSSHPUSH_H

#include <qsshtask.h>
#include <libssh/libssh.h>
class QSshPush : public QSshTask
{
    Q_OBJECT
public:
    QSshPush(const QString & from_localPath, const QString & to_remotePath, ssh_session s);
    bool handle() override;
    QString from() const {
        return localPath;
    };
    QString to() const {
        return remotePath;
    }
private:
    ssh_session m_session;
    QString localPath;
    QString remotePath;
};

#endif // QSSHPUSH_H
