#include "qsshcommand.h"
#include <QDebug>
QSshCommand::QSshCommand(const QString & cmd, ssh_session s)
    : m_cmd(cmd), m_session(s), dir(".")
{
}

void QSshCommand::handle()
{
    // attempt to open ssh shell channel
    ssh_channel channel = ssh_channel_new(m_session);

    // if attempt fails,return
    if (ssh_channel_open_session(channel) != SSH_OK)
    {
        error(this);
        return;
    }

    int requestResponse = SSH_AGAIN;

    QString command = QString("cd %1 && %2").arg(dir).arg(m_cmd);

    // attempt to execute shell command
    while (requestResponse == SSH_AGAIN /*&& time < timeout */)
        requestResponse = ssh_channel_request_exec(channel, command.toStdString().c_str());

    // if attempt not executed, close connection then return
    if (requestResponse != SSH_OK)
    {
        error(this);
        return;
    }



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

    // close channel
    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    m_response = buffer;
    success(this);



//    // attempt to open ssh shell channel
//    ssh_channel channel = ssh_channel_new(m_session);

//    // if attempt fails,return
//    if (ssh_channel_open_session(channel) != SSH_OK)
//    {
//        error(this);
//        return;
//    }

//    ssh_channel_request_shell(channel);
//    ssh_channel_write(channel, m_cmd.toStdString().c_str(), m_cmd.length());

////    int requestResponse = SSH_AGAIN;
////    // attempt to execute shell command
////    while (requestResponse == SSH_AGAIN /*&& time < timeout */)
////        requestResponse = ssh_channel_request_exec(channel, m_cmd.toStdString().c_str());

////    // if attempt not executed, close connection then return
////    if (requestResponse != SSH_OK)
////    {
////        error(this);
////        return;
////    }



//    QByteArray buffer;
//    // read in command result
//    int newBytes = 0;
//    do
//    {
//        std::array<uchar, 1000> arr;
//        newBytes = ssh_channel_read(channel, &arr[0], arr.max_size(), 0);
//        if (newBytes > 0)
//        {
//            buffer.append((const char*)arr.data(), newBytes);
//        }

//    } while (newBytes > 0);

//    // close channel
//    ssh_channel_send_eof(channel);
//    ssh_channel_close(channel);
//    ssh_channel_free(channel);

//    m_response = buffer;
//    success(this);
}
