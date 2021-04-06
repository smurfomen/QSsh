#ifndef QSFTPITEM_H
#define QSFTPITEM_H

#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <QTypeInfo>
#include <QString>
#include <qoption.h>
#include <QDir>
#include <fcntl.h>
struct QSftpItem {
    QSftpItem(const QString & folder, sftp_attributes m_attr, ssh_session ssh, sftp_session sftp);

    /// \brief Download item and inner items to dest directory
    bool download(const QString & dest) const;

     /// \brief Item is directory
    bool isDir() const {
        return m_type == SSH_FILEXFER_TYPE_DIRECTORY;
    }

    /// \brief Item is file
    bool isFile() const {
        return m_type == SSH_FILEXFER_TYPE_REGULAR;
    }

    bool isSymlink() const {
        return m_type == SSH_FILEXFER_TYPE_SYMLINK;
    }

    bool isExecutable() const {
        return m_perm & S_IXUSR;
    }

    bool isReadable() const {
        return m_perm & S_IRUSR;
    }

    bool isWritable() const {
        return m_perm & S_IWUSR;
    }

    /// \brief absolute path /[path to folder]/[name item]
    QString absolutePath() const {
        return QString("%1/%2").arg(m_folder).arg(m_name);
    }

    /// \brief folder contained this item
    QString folder() const {
        return m_folder;
    }

    /// \brief name this item
    QString name() const {
        return m_name;
    }

    /// \brief user who owned this item
    QString owner() const {
        return m_owner;
    }

    /// \brief group who owned this item
    QString group() const {
        return m_group;
    }

    /*!
     * \brief type
     * \return  1 - file,
     *          2 - dir,
     *          3 - symlink,
     *          4 - special,
     *          5 - unknown
     */
    quint8 type() const {
        return m_type;
    }

    /// \brief size of this item in bytes
    size_t size() const {
        return m_size;
    }

    /// \brief user id, who owned this item
    qint64 uid() const {
        return m_uid;
    }

    /// \brief group id, who owned this item
    qint64 gid() const {
        return m_gid;
    }

    /// \brief create unix-time when this item will be created
    quint64 create_time() const {
        return m_create_time;
    }

    /*!
     * \brief items
     * \param filter - may filtered by dirs and files
     * \param match - phrase for matching by inner items names
     * \return items, located in current item
     */
    QList<QSftpItem> items(QDir::Filters filter = QDir::Files | QDir::Dirs, const QString& match = QString()) const;

private:
    QString m_folder;
    QString m_name;
    QString m_owner;
    QString m_group;
    quint8 m_type;
    quint32 m_perm;
    size_t m_size;
    qint64 m_gid, m_uid;
    quint64 m_create_time;
    ssh_session m_ssh;
    sftp_session m_sftp;
};

#endif // QSFTPITEM_H
