#ifndef QSSHCONNECTION_H
#define QSSHCONNECTION_H

#include <QObject>
#include <QQueue>
#include <QFileInfo>

#include <libssh/libssh.h>
#include <qsftpitem.h>
#include <qsshcommand.h>
#include <qsshtask.h>
#include <qoption.h>

class QSshConnection : public QObject
{
    Q_OBJECT
public:
    QSshConnection(ssh_session s, sftp_session sftp, const QString & host, const QString & user, const QString & pass, QObject * parent = Q_NULLPTR);

    ~QSshConnection();

    /// \brief Push task to queue tasks for execute
    template<typename T, class = typename std::enable_if<std::is_base_of<QSshTask, T>::value, bool>::type>
    void push(T * t) {
        std::shared_ptr<T> task = std::shared_ptr<T>(t);
        tasks.append(task);
    }

    /// \brief Create command for execute to remote machine
    QSshCommand * command(const QString & cmd, const QString & wd = ".") const {
        QSshCommand * c = new QSshCommand(cmd, m_ssh);
        c->setWorkingDirectory(wd);
        return c;
    }

    /// \brief Run command to remote machine
    template<typename T = QSshCommand, class = typename std::enable_if<std::is_base_of<QSshTask, T>::value, bool>::type>
    bool run(const QString & arg1) const {
        std::shared_ptr<T> task = std::shared_ptr<T>(new T(arg1, m_ssh));
        return task->handle();
    }

    /// \brief run queue commands to remote machine
    bool run() {
        bool ok = tasks.size();
        for(auto t : tasks) {
            ok &= t->handle();
        }
        tasks.clear();
        return ok;
    }


    /*!
     * \brief Return items in folder on remote machine
     * \param path - remote folder
     * \param filter - filter by dirs or files
     * \param match - match phrase for filter
     * \return list of items, who contained in remote \e path folder
     */
    QList<class QSftpItem> items(const QString& path, QDir::Filters filter = QDir::Dirs | QDir::Files, const QString& match = QString()) const;

    /// \brief Current SSH session
    ssh_session ssh() const {
        return m_ssh;
    }

    /// \brief Current SFTP session
    sftp_session sftp() const {
        return m_sftp;
    }

    /// \brief Current user opened SSH session
    QString user() const {
        return m_user;
    }

    /// \brief Current user opened SSH session
    QString host() const {
        return m_host;
    }

    /// \brief Current user opened SSH session
    QString password() const {
        return m_pass;
    }

    /*!
     * \brief Upload files to remote machine
     * \param from - folder or file on local machine
     * \param to - folder on remote machine
     * \return status of upload files
     */
    bool upload(const QString & from, const QString& to) const;

    /*!
     * \brief Upload files to remote machine
     * \param files - QFileInfo list files on local machine
     * \param to - folder on remote machine
     * \return status of upload files
     */
    bool upload(QFileInfoList files, const QString & to) const;

    /*!
     * \brief Upload file to remote machine
     * \param fi - QFileInfo of file on local machine
     * \param to - folder on remote machine
     * \return status of upload file
     */
    bool upload(QFileInfo file, const QString & to) const;

private:
    ssh_session m_ssh;
    sftp_session m_sftp;
    QString m_host;
    QString m_user;
    QString m_pass;
    QQueue<std::shared_ptr<QSshTask>> tasks;
};



namespace qssh {
    QOption<QSshConnection *> connect(const QString & host, const QString & user, const QString & pass);
}

#endif // QSSHCONNECTION_H
