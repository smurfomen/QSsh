#include "qsshconnection.h"

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
                return new QSshConnection(session, host, user, pass);
        }

        return None();
    }
}


QSshConnection::QSshConnection(ssh_session s, const QString& host, const QString& user, const QString& pass, QObject* parent)
    : QObject(parent),
      m_session(s),
      m_host(host),
      m_user(user),
      m_pass(pass)
{

}

QSshConnection::~QSshConnection()
{
    if (m_session != NULL) {
        ssh_disconnect(m_session);
        ssh_free(m_session);
    }

    m_session = NULL;
}

QSftpFolder QSshConnection::folder(const QString& path) const
{
    return QSftpFolder(path, m_session);
}

