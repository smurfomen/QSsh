#ifndef QSSHCONNECTION_H
#define QSSHCONNECTION_H

#include <libssh/libssh.h>
#include <QObject>
#include <qoption.h>
#include <qsshtask.h>
#include <QQueue>

class QSshConnection : public QObject
{
    Q_OBJECT
public:
    QSshConnection(ssh_session s, const QString & host, const QString & user, const QString & pass, QObject * parent = Q_NULLPTR);

    ~QSshConnection();

    void pushTask(QSshTask * task) {
        tasks.append(task);
    }

    template<typename T, class = typename std::enable_if<std::is_base_of<QSshTask, T>::value, bool>::type>
    T * push(const QString & arg1, const QString & arg2) {
        T * task = new T(arg1, arg2, m_session);
        tasks.append(task);
        return task;
    }

    template<typename T, class = typename std::enable_if<std::is_base_of<QSshTask, T>::value, bool>::type>
    T * push(const QString & arg1) {
        T * task = new T(arg1, m_session);
        tasks.append(task);
        return task;
    }

    void run() {
        for(auto t : tasks) {
            t->handle();
        }
    }

    ssh_session session() const {
        return m_session;
    }

    QString user() const {
        return m_user;
    }

    QString host() const {
        return m_host;
    }

    QString password() const {
        return m_pass;
    }

private:
    ssh_session m_session;
    QString m_host;
    QString m_user;
    QString m_pass;
    QQueue<QSshTask*> tasks;
};



namespace qssh {
    QOption<QSshConnection *> connect(const QString & host, const QString & user, const QString & pass);
}


#endif // QSSHCONNECTION_H
