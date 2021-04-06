#include "qsshconnection.h"
#include <QFileInfo>
#include <QDir>
#include <fcntl.h>

namespace qssh {
    QOption<QSshConnection*> connect(const QString& host, const QString& user, const QString& pass) {
        ssh_session session = ssh_new();

        //set logging to verbose so all errors can be debugged if crash happens
        int verbosity = SSH_LOG_PROTOCOL;

        ushort port = 22;
        // set the pertinant ssh session options
        ssh_options_set(session, SSH_OPTIONS_HOST, host.toStdString().c_str());
        ssh_options_set(session, SSH_OPTIONS_USER, user.toStdString().c_str());
        ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
        ssh_options_set(session, SSH_OPTIONS_PORT, &port);

        // try to connect given host, user, port
        // if connection is Successful keep track of connection info.
        if (ssh_connect(session) == SSH_OK) {
            // try authenticating current user at remote host
            int worked = ssh_userauth_password(session, user.toStdString().c_str(), pass.toStdString().c_str());

            // if successful, store user password.
            if (worked == SSH_OK)
            {
                sftp_session sftp = sftp_new(session);
                if(sftp != NULL)
                {
                    if(sftp_init(sftp) == SSH_OK)
                        return new QSshConnection(session, sftp, host, user, pass);
                }
            }
        }

        return None();
    }
}


QSshConnection::QSshConnection(ssh_session s, sftp_session sftp, const QString& host, const QString& user, const QString& pass, QObject* parent)
    : QObject(parent),
      m_ssh(s),
      m_sftp(sftp),
      m_host(host),
      m_user(user),
      m_pass(pass)
{

}

QSshConnection::~QSshConnection()
{   
    if (m_ssh != NULL) {
        ssh_disconnect(m_ssh);
        ssh_free(m_ssh);
    }

    m_ssh = NULL;
}



QList<QSftpItem> QSshConnection::items(const QString & path, QDir::Filters filter, const QString& match) const {
    QList<QSftpItem> list;
    if(path.isEmpty() || m_ssh == NULL)
        return list;

    sftp_dir dir = sftp_opendir(m_sftp, path.toStdString().c_str());
    if (dir != NULL)
    {
        sftp_attributes attributes;
        while ((attributes = sftp_readdir(m_sftp, dir)) != NULL)
        {
            if(QString(attributes->name) == "." || QString(attributes->name) == "..")
                continue;

            if(!match.isEmpty() && !QString(attributes->name).contains(match))
                continue;

            QSftpItem item(path, attributes, m_ssh, m_sftp);

            bool suit = (filter.testFlag(QDir::Dirs) && item.isDir()) || (filter.testFlag(QDir::Files) && item.isFile());

            if(filter.testFlag(QDir::NoSymLinks))
                suit &= !item.isSymlink();

            if(filter.testFlag(QDir::Executable))
                suit &= item.isExecutable();

            if(filter.testFlag(QDir::Readable))
                suit &= item.isReadable();

            if(filter.testFlag(QDir::Writable))
                suit &= item.isWritable();

            if(suit)
                list.append(item);

            sftp_attributes_free(attributes);
        }

        sftp_dir_eof(dir);
        sftp_closedir(dir);
    }

    return list;
}

bool QSshConnection::upload(const QString& from, const QString & to) const
{
    bool ok = false;
    QFileInfo info(from);

    if(info.isDir())
    {
        QDir d (info.absoluteFilePath());
        auto files = d.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
        ok = upload(files, to);
    }
    else if (info.isFile())
    {
        ok = upload(info, to);
    }

    return ok;
}

bool QSshConnection::upload(QFileInfoList files, const QString& to) const
{
    bool ok = false;

    // проверяем есть ли целевая директория на ремоуте
    sftp_dir d = sftp_opendir(m_sftp, to.toStdString().c_str());

    // если нет - создаем
    if(d == NULL || strlen(d->name))
        sftp_mkdir(m_sftp, to.toStdString().c_str(), S_IRWXU);

    if(d != NULL)
        sftp_closedir(d);

    for(auto fi : files)
    {
        if(fi.isDir())
        {
            QDir d(fi.absoluteFilePath());
            auto innerFiles = d.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
            ok = upload(innerFiles, to + "/" + fi.baseName());
        }
        else if(fi.isFile())
        {
           ok = upload(fi, to);
        }
    }

    return ok;
}

bool QSshConnection::upload(QFileInfo file, const QString& to) const
{
    bool ok = false;
    QFile f(file.absoluteFilePath());
    if(f.open(QIODevice::ReadOnly))
    {
        QString remoteFile = to + "/" + file.fileName();
        QByteArray data = f.readAll();
        f.close();

        mode_t mode = 0;
        if(file.isExecutable())
            mode |= S_IXUSR;
        if(file.isWritable())
            mode |= S_IWUSR;
        if(file.isReadable())
            mode |= S_IRUSR;

        sftp_file file = sftp_open(m_sftp, remoteFile.toStdString().c_str(),
                                   O_WRONLY | O_CREAT | O_TRUNC, mode);

        if(file != NULL)
        {
            int nwritten = sftp_write(file, data.data(), data.length());
            ok = sftp_close(file) == SSH_OK && nwritten == data.length();
        }
    }
    return ok;
}
