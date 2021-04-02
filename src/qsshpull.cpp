#include "qsshpull.h"
#include <QFile>
#include  <QDebug>

QSshPull::QSshPull(const QString& from_remotePath, const QString& to_localPath, ssh_session s)
    : remotePath(from_remotePath), localPath(to_localPath), m_session(s)
{

}

void QSshPull::handle()
{
    ssh_scp scpSession = ssh_scp_new(m_session,SSH_SCP_READ, remotePath.toStdString().c_str());

    if (scpSession == NULL)
        error(this);

    else {
        // attempt to initialize new scp session.
        int scpInitialized = ssh_scp_init(scpSession);
        if(scpInitialized != SSH_OK)
            error(this);

        // attempt to authorize new scp pull
        else if(ssh_scp_pull_request(scpSession) != SSH_SCP_REQUEST_NEWFILE)
            error(this);

        else
        {
            // accept authorization
            ssh_scp_accept_request(scpSession);

            // get remote file size
            int size = ssh_scp_request_get_size(scpSession);

            // resize buffer, read remote file into buffer
            QByteArray buffer;
            buffer.resize(size);

            // if an error happens while reading, close the scp session and return
            if (ssh_scp_read(scpSession, buffer.data(), size) == SSH_ERROR)
                error(this);

            // loop until eof flag
            else if (ssh_scp_pull_request(scpSession)  != SSH_SCP_REQUEST_EOF)
                error(this);

            else {
                // open up local file and write contents of buffer to it.
                QFile file(localPath);
                if(file.open(QIODevice::WriteOnly | QIODevice::Truncate))
                {
                    file.write(buffer);
                    file.close();
                    success(this);
                }
                else
                    error(this);
            }
        }

        ssh_scp_close(scpSession);
        ssh_scp_free(scpSession);
    }
}
