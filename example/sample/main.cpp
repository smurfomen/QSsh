#include <QCoreApplication>
#include <QDebug>

#include <qsshconnection.h>
#include <qsshcommand.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QSshConnection * ssh = qssh::connect("192.168.0.108", "pcuser", "pcuser").expect("SSH connection doesn't established");

    QSshCommand * command = ssh->push<QSshCommand>("cat /home/pcuser/tobuild/kp_nix/main.cpp");

    QObject::connect(command, &QSshTask::success, [&](QSshTask * t){
        QSshCommand * c = (QSshCommand*)t;
        qDebug()<< "executed:" << c->command() << ": " << c->response();
    });

    ssh->run();


    return a.exec();
}
