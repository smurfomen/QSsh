#ifndef QSFTPFOLDER_H
#define QSFTPFOLDER_H


#include <QObject>
#include <libssh/libssh.h>

class QSftpFolder {
public:
    enum filter_t {
        Dir = 1 << 0,
        File = 1 << 1,
    };
    Q_DECLARE_FLAGS(Filter, filter_t);

    QSftpFolder(const QString & m_path, ssh_session s);

    QStringList content(Filter filter = Filter(), const QString & match = QString()) const;

    QString path() const {
        return m_path;
    }

    bool exists() const;

    bool download(const QString & dest, Filter filter = Filter(), const QString & match = QString()) const;

    bool upload(const QString & from, Filter filter = Filter(), const QString & match = QString()) const;

private:
    QString m_path;
    ssh_session m_session;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QSftpFolder::Filter);


#endif // QSFTPFOLDER_H
