#include "qsshpush.h"
#include <sys/stat.h>
#include <QFile>
QSshPush::QSshPush(const QString& from_localPath, const QString& to_remotePath, ssh_session s)
    : m_session(s), localPath(from_localPath), remotePath(to_remotePath)
{

}

void QSshPush::handle()
{
    // attempt to create new scp from ssh session.
    ssh_scp scpSession = ssh_scp_new(m_session, SSH_SCP_WRITE, remotePath.toStdString().c_str());

    // if creation failed, return
    if (scpSession == NULL)
        error(this);


    // attempt to initialize new scp session.
    int scpInitialized = ssh_scp_init(scpSession);


    // if failed, close scp session and return.
    if( scpInitialized != SSH_OK )
    {
        ssh_scp_close(scpSession);
        ssh_scp_free(scpSession);
        error(this);
    }


    // open the local file and check to make sure it exists
    // if not, close scp session and return.
    QFile file(localPath);
    if ( !file.exists() )
    {
        ssh_scp_close(scpSession);
        ssh_scp_free(scpSession);
        error(this);
        return;
    }

    // if the file does exist, read all contents as bytes
    file.open(QIODevice::ReadOnly);
    QByteArray buffer = file.readAll();
    file.close();

    // attempt to authorize pushing bytes over scp socket
    // if this fails, close scp session and return.
    if ( ssh_scp_push_file(scpSession, remotePath.toStdString().c_str(), buffer.size(), S_IRUSR | S_IWUSR) != SSH_OK)
    {
        ssh_scp_close(scpSession);
        ssh_scp_free(scpSession);
        error(this);
        return;
    }


    // once authorized to push bytes over scp socket, start writing
    // if an error is returned,  close scp session and return.
    if ( ssh_scp_write(scpSession, buffer.data(), buffer.size() ) != SSH_OK)
    {

        ssh_scp_close(scpSession);
        ssh_scp_free(scpSession);
        error(this);
        return;
    }

    // close scp session and return.
    ssh_scp_close(scpSession);
    ssh_scp_free(scpSession);

    success(this);
}
