#include "qsftpitem.h"
#include <QDir>
#include <fcntl.h>
#include <qsshconnection.h>

QSftpItem::QSftpItem(const QString& folder, sftp_attributes attr, ssh_session ssh, sftp_session sftp):
    m_folder(folder),
    m_name(attr->name),
    m_owner(attr->owner),
    m_group(attr->group),
    m_type(attr->type),
    m_perm(attr->permissions),
    m_size(attr->size),
    m_gid(attr->gid),
    m_uid(attr->uid),
    m_create_time(attr->createtime),
    m_ssh(ssh),
    m_sftp(sftp)
{

}

bool QSftpItem::download(const QString& dest) const
{
    // create dir for downloaded files
    if(!QFileInfo(dest).exists())
        QDir::home().mkpath(dest);

    bool ok = false;
    // dir items
    if(isDir())
    {
        QString innerDir = dest + "/" + name();
        if(QDir(dest).mkpath(innerDir))
        {
            auto list = items();
            for(auto it : list) {
                it.download(innerDir);
            }
        }
    }

    // file items
    else if(isFile())
    {
        std::array<char, 4096> buffer;
        QString p = absolutePath();
        sftp_file file = sftp_open(m_sftp, p.toStdString().c_str(), O_RDONLY, 0);
        if (file != NULL)
        {
            QFile destFile(dest + "/" + name());
            if(destFile.open(QIODevice::Truncate | QIODevice::Append))
            {
                for (;;)
                {
                    int readed = sftp_read(file, &buffer[0], buffer.max_size());
                    if (readed <= 0)
                    {
                        ok = true;
                        break;
                    }

                    if(destFile.write(buffer.data(), readed) != readed)
                        break;
                }
                destFile.close();
            }

            ok = sftp_close(file) == SSH_OK && ok;
        }
    }
    return ok;
}

QList<QSftpItem> QSftpItem::items(QDir::Filters filter, const QString& match) const
{
    QList<QSftpItem> list;

    // have inner items only if outer item is directory
    if(isDir())
    {
        sftp_dir dir = sftp_opendir(m_sftp, absolutePath().toStdString().c_str());
        if (dir != NULL)
        {
            // read all inner items
            sftp_attributes attributes;
            while ((attributes = sftp_readdir(m_sftp, dir)) != NULL)
            {
                // skip if dot or dot-dot
                if(QString(attributes->name) == "." || QString(attributes->name) == "..")
                    continue;

                // match
                if(!match.isEmpty() && !QString(attributes->name).contains(match))
                    continue;

                // create inner item
                QSftpItem item(absolutePath(), attributes, m_ssh, m_sftp);

                // append inner item to list
                bool suit = (filter.testFlag(QDir::Dirs) && item.isDir())               ||
                            (filter.testFlag(QDir::Files) && item.isFile())             ||
                            (filter.testFlag(QDir::Executable) && item.isExecutable())  ||
                            (filter.testFlag(QDir::Readable) && item.isReadable())      ||
                            (filter.testFlag(QDir::Writable) && item.isWritable());

                if(suit)
                    list.append(item);

                // free attribute
                sftp_attributes_free(attributes);
            }

            sftp_dir_eof(dir);
            sftp_closedir(dir);
        }
    }

    return list;
}
