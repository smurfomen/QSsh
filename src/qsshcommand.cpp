#include "qsshcommand.h"
#include <QDebug>
QSshCommand::QSshCommand(const QString & cmd, ssh_session s)
    : m_cmd(cmd), m_session(s), dir(".")
{
}

bool QSshCommand::handle()
{
    bool ok = false;
    // attempt to open ssh shell channel
    ssh_channel channel = ssh_channel_new(m_session);

    if(channel == NULL)
        error(this);

    else {
        int rc = ssh_channel_open_session(channel);

        // if attempt fails,return
        if (rc != SSH_OK)
            error(this);

        else {
            QString command = QString("cd %1 && %2").arg(dir).arg(m_cmd);

            // attempt to execute shell command
            do {
                rc = ssh_channel_request_exec(channel, command.toStdString().c_str());
            } while (rc == SSH_AGAIN);

            // if attempt not executed, close connection then return
            if (rc != SSH_OK)
                error(this);

            else {
                QByteArray buffer;
                // read in command result
                int newBytes = 0;
                do
                {
                    std::array<uchar, 1000> arr;
                    newBytes = ssh_channel_read(channel, &arr[0], arr.max_size(), 0);
                    if (newBytes > 0)
                    {
                        buffer.append((const char*)arr.data(), newBytes);
                    }

                } while (newBytes > 0);

                m_response = buffer;
                ok = true;
                success(this);
            }

            // close channel
            ssh_channel_send_eof(channel);
        }

        ssh_channel_close(channel);
        ssh_channel_free(channel);
    }
    return ok;
}
