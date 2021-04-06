#include <QCoreApplication>
#include <QDebug>

#include <qsshconnection.h>
#include <qsshcommand.h>

QSshConnection * ssh = qssh::connect("192.168.0.108", "pcuser", "pcuser").expect("SSH connection doesn't established");

void command() {
    QSshCommand * c = ssh->command("cat /home/pcuser/tobuild/kitconfig1/configs.json");
    c->handle();
    qDebug()<< c->response();
    delete c;
}

void items() {
    auto items = ssh->items("/home/pcuser/tobuild/kitconfig1", QDir::Files | QDir::Executable);
    for(auto i : items)
        qDebug()<<i.name() << (i.isFile() ? "FILE" : i.isDir() ?  "DIR" : "") << (i.isSymlink() ? "SYMLINK" : "");
}


void download() {
    auto items = ssh->items("/home/pcuser/tobuild", QDir::Dirs, "kitconfig1");

    //download all items to testdir1
    for(auto i : items)
        i.download("/home/pcuser/testdir1");
}

void upload() {
    // upload all content testdir1 to remote path kitconfig1
    qDebug()<<ssh->upload("/home/pcuser/testdir1", "/home/pcuser/tobuild/kitconfig1");
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    items();

    return a.exec();
}
