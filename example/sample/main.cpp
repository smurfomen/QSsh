#include <QCoreApplication>
#include <QDebug>

#include <qsshconnection.h>
#include <qsshcommand.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QSshConnection * ssh = qssh::connect("192.168.0.108", "pcuser", "pcuser").expect("SSH connection doesn't established");
//    auto f = ssh->folder("/home/pcuser/tobuild/kitconfig");

//    f.download("/home/pcuser/testdir", QSftpFolder::File | QSftpFolder::Dir);


    auto f1 = ssh->folder("/home/pcuser/tobuild/kitconfig_TEST1");

    qDebug()<<f1.upload("/home/pcuser/testdir", QSftpFolder::File | QSftpFolder::Dir);

    qDebug()<<"!!!END!!!";

    return a.exec();
}
