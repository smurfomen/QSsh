#ifndef QSSHCOMMAND_H
#define QSSHCOMMAND_H

#include <qsshtask.h>
#include <libssh/libssh.h>
class QSshCommand : public QSshTask {
    Q_OBJECT
public:
    QSshCommand(const QString & m_cmd, ssh_session s);
    bool handle() override;
    QString command() const {
        return m_cmd;
    }

    QString response() const {
        return m_response;
    }

    void setWorkingDirectory(const QString & directory) {
        dir = directory;
    }

private:
    QString m_cmd;
    ssh_session m_session;
    QString m_response;
    QString dir;
};

#endif // QSSHCOMMAND_H
