#include "qsftpfolder.h"
#include <libssh/sftp.h>
#include <fcntl.h>

#include <QFile>
#include <QDir>

QSftpFolder::QSftpFolder(const QString& path, ssh_session s)
    : m_path(path), m_session(s)
{

}

QStringList QSftpFolder::content(Filter filter, const QString& match) const {
    QStringList list;
    if(m_path.isEmpty() || m_session == NULL)
        return list;

    sftp_session sftp;
    sftp = sftp_new(m_session);
    if (sftp != NULL)
    {
        if (sftp_init(sftp) == SSH_OK)
        {
            sftp_dir dir;
            sftp_attributes attributes;
            dir = sftp_opendir(sftp, m_path.toStdString().c_str());
            if (dir != NULL)
            {
                while ((attributes = sftp_readdir(sftp, dir)) != NULL)
                {
                    if(QString(attributes->name) == "." || QString(attributes->name) == "..")
                        continue;

                    if(!match.isEmpty() && !QString(attributes->name).contains(match))
                        continue;

                    if(filter.testFlag(Dir) && attributes->type == SSH_FILEXFER_TYPE_DIRECTORY)
                        list.append(attributes->name);
                    else if(filter.testFlag(File) && attributes->type == SSH_FILEXFER_TYPE_REGULAR)
                        list.append(attributes->name);

                    //       attributes->name,
                    //       (long long unsigned int) attributes->size,
                    //       attributes->permissions,
                    //       attributes->owner,
                    //       attributes->uid,
                    //       attributes->group,
                    //       attributes->gid);

                    sftp_attributes_free(attributes);
                }
            }
            sftp_dir_eof(dir);
            sftp_closedir(dir);
        }

        sftp_free(sftp);
    }

    return list;
}

bool QSftpFolder::exists() const {
    bool ok = false;
    sftp_session sftp = sftp_new(m_session);
    if(sftp != NULL)
    {
        if (sftp_init(sftp) == SSH_OK)
        {
            sftp_dir d = sftp_opendir(sftp, m_path.toStdString().c_str());
            ok = d != NULL && strlen(d->name);
        }
        sftp_free(sftp);
    }
    return ok;
}


bool QSftpFolder::download(const QString& dest, Filter filter, const QString& match) const {

    bool ok = false;
    if(filter.testFlag(Dir))
    {
        QStringList list = content(Dir);
        for(auto it : list)
        {
            if(QDir(dest).mkpath(dest + "/" + it))
            {
                QSftpFolder f(m_path + "/" + it, m_session);
                ok = f.download(dest + "/" + it, Dir | File);
            }
        }
    }

    if(filter.testFlag(File))
    {
        QStringList list = content(File, match);
        sftp_session sftp = sftp_new(m_session);
        if (sftp != NULL)
        {
            if (sftp_init(sftp) == SSH_OK)
            {
                for(auto fileName : list)
                {
                    // отсечь неподходящие по имени
                    if(!match.isEmpty() && !fileName.contains(match))
                        continue;

                    std::array<char, 4096> buffer;

                    QString fpath = QString("%1/%2").arg(m_path).arg(fileName);
                    sftp_file file = sftp_open(sftp, fpath.toStdString().c_str(), O_RDONLY, 0);
                    if (file == NULL)
                        continue;

                    QFile destFile(dest + "/" + fileName);
                    if(destFile.open(QIODevice::Truncate | QIODevice::Append))
                    {
                        for (;;)
                        {
                            int readed = sftp_read(file, &buffer[0], buffer.max_size());
                            if (readed <= 0)
                                break;

                            destFile.write(buffer.data(), readed);
                        }
                        destFile.close();
                        ok = true;
                    }

                    ok = sftp_close(file) == SSH_OK;
                }
            }
            sftp_free(sftp);
        }
    }

    return ok;
}

bool QSftpFolder::upload(const QString& from, Filter filter, const QString& match) const
{
    bool ok = false;
    QFileInfo fi(from);

    // создать директорию если копируем целую директорию
    if(fi.isDir())
    {
        QDir d (from);
        auto list = d.entryList(QDir::Dirs | QDir::Files);

        if(!exists() && list.size())
        {
            sftp_session sftp;
            sftp = sftp_new(m_session);
            if(sftp != NULL)
            {
                if (sftp_init(sftp) == SSH_OK)
                    sftp_mkdir(sftp, m_path.toStdString().c_str(), S_IRWXU);

                sftp_free(sftp);
            }
        }

        if((ok = exists()))
        {
            for(auto it : list)
            {
                if(it == "." || it == "..")
                    continue;

                QFileInfo info (d.absolutePath() + "/" + it);

                if(info.isDir() && filter.testFlag(Dir))
                {
                    QSftpFolder f(m_path + "/" + it, m_session);
                    ok = f.upload(from + "/" +it, Dir | File);
                }

                else if(info.isFile() && filter.testFlag(File))
                {
                    ok = upload(info.absoluteFilePath(), File);
                }
            }
        }
    }


    else if(fi.isFile() && (match.isEmpty() || fi.fileName().contains(match)))
    {
        sftp_session sftp = sftp_new(m_session);
        if (sftp != NULL)
        {
            if (sftp_init(sftp) == SSH_OK)
            {
                QFile f(from);
                if(f.open(QIODevice::ReadOnly))
                {
                    QString destPath = m_path + "/" + fi.fileName();
                    QByteArray data = f.readAll();
                    f.close();

                    sftp_file file = sftp_open(sftp, destPath.toStdString().c_str(),
                                     O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

                    if(file != NULL)
                    {
                        int nwritten = sftp_write(file, data.data(), data.length());
                        ok = nwritten == data.length() && sftp_close(file) == SSH_OK;
                    }
                }
            }
            sftp_free(sftp);
        }
    }

    return ok;
}


