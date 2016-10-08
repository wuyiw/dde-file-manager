#include "usersharemanager.h"
#include <QProcess>
#include <QStandardPaths>
#include <QFile>
#include <QApplication>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

#include <simpleini/SimpleIni.h>
#include "shareinfo.h"

UserShareManager::UserShareManager(QObject *parent) : QObject(parent)
{
    m_fileSystemWatcher = new QFileSystemWatcher(this);
    m_fileSystemWatcher->addPath(UserSharePath());
    initConnect();
    updateUserShareInfo();
    loadUserShareInfoPathNames();

//    ShareInfo info;
//    info.setShareName("11111");
//    info.setPath("/home/djf/Desktop");
//    addUserShare(info);
//    deleteUserShareByPath("/home/djf/Desktop");
}

UserShareManager::~UserShareManager()
{

}

void UserShareManager::initConnect()
{
    connect(m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &UserShareManager::handleShareChanged);
}

QString UserShareManager::getCacehPath()
{
    return QString("%1/.cache/%2/usershare.json").arg(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0),
                                                      qApp->applicationName());
}

ShareInfo UserShareManager::getShareInfoByPath(const QString &path) const
{
    QString shareName = getShareNameByPath(path);
    if (!shareName.isEmpty()){
        if (m_shareInfos.contains(shareName)){
            return m_shareInfos.value(shareName);
        }
    }
    return ShareInfo();
}

QString UserShareManager::getShareNameByPath(const QString &path) const
{
    QString shareName;

    if (m_sharePathByFileManager.contains(path)){
        shareName = m_sharePathByFileManager.value(path);
    }else{
        if (m_sharePathToNames.contains(path)){
            QStringList shareNames = m_sharePathToNames.value(path);
            if (shareNames.count() > 0){
                shareName = shareNames.last();
            }
        }
    }
    return shareName;
}

void UserShareManager::loadUserShareInfoPathNames()
{
    QString cache = readCacheFromFile(getCacehPath());
    if (!cache.isEmpty()){
        QJsonParseError error;
        QJsonDocument doc=QJsonDocument::fromJson(cache.toLocal8Bit(),&error);
        if (error.error == QJsonParseError::NoError){
            QJsonObject obj = doc.object();
            foreach (QString key, obj.keys()) {
                m_sharePathByFileManager.insert(key, obj.value(key).toString());
            }
        }else{
            qDebug() << "load cache file: " << getCacehPath() << error.errorString();
        }
    }
}

void UserShareManager::saveUserShareInfoPathNames()
{
    QVariantMap cache;
    foreach (const QString& path, m_sharePathByFileManager.keys()) {
        cache.insert(path, m_sharePathByFileManager.value(path));
    }

    QJsonDocument doc(QJsonObject::fromVariantMap(cache));
    writeCacheToFile(getCacehPath(), doc.toJson());
}

void UserShareManager::writeCacheToFile(const QString &path, const QString &content)
{
    QFile file(path);
    if (file.open(QFile::WriteOnly)){
        file.write(content.toLocal8Bit());
    }
    file.close();
}

QString UserShareManager::readCacheFromFile(const QString &path)
{
    QFile file(path);
    if(!file.open(QFile::ReadOnly))
    {
        qDebug() << path << "isn't exists!";
        return QString();
    }
    QByteArray content = file.readAll();
    file.close();
    return QString(content);
}

void UserShareManager::handleShareChanged()
{
    updateUserShareInfo();
}

void UserShareManager::updateUserShareInfo()
{
    QProcess net_usershare_info;
    net_usershare_info.start("net usershare info");
    if (net_usershare_info.waitForFinished()){

        CSimpleIniA settings;
        settings.SetUnicode(true);
        QString content(net_usershare_info.readAll());
        settings.LoadData(content.toStdString().c_str(), content.length());

        CSimpleIniA::TNamesDepend sections;
        settings.GetAllSections(sections);
        CSimpleIniA::TNamesDepend::iterator i;

        m_shareInfos.clear();
        m_sharePathToNames.clear();

        for (i = sections.begin(); i != sections.end(); ++i){
            CSimpleIniA::Entry sectionEntry = *i;
            CSimpleIniA::TNamesDepend keys;
            settings.GetAllKeys(sectionEntry.pItem, keys);
            CSimpleIniA::TNamesDepend::iterator j;

            ShareInfo info;
            info.setShareName(sectionEntry.pItem);
            for (j = keys.begin(); j != keys.end(); ++j){
                CSimpleIniA::Entry keyEntry = *j;
                const char * value = settings.GetValue(sectionEntry.pItem, keyEntry.pItem);
                if (QString(keyEntry.pItem) == "comment"){
                    info.setComment(value);
                }else if (QString(keyEntry.pItem) == "path"){
                    info.setPath(value);
                }else if (QString(keyEntry.pItem) == "usershare_acl"){
                    info.setUsershare_acl(value);
                }else if (QString(keyEntry.pItem) == "guest_ok"){
                    info.setGuest_ok(value);
                }
            }
            m_shareInfos.insert(info.shareName(), info);

            if (m_sharePathToNames.contains(info.path())){
                QStringList names = m_sharePathToNames.value(info.path());
                names.append(info.shareName());
                m_sharePathToNames.insert(info.path(), names);
            }else{
                QStringList names;
                names.append(info.shareName());
                m_sharePathToNames.insert(info.path(), names);
            }
        }
    }
    qDebug() << m_sharePathToNames;
}

void UserShareManager::addUserShare(const ShareInfo &info)
{
    if (!info.shareName().isEmpty() && QFile(info.path()).exists()){
        QString cmd = "net";
        QStringList args;
        args << "usershare" << "add"
             << info.shareName() << info.path()
             << info.comment() << info.usershare_acl()
             << info.guest_ok();
        bool ret = QProcess::startDetached(cmd, args);
        if (ret){
            qDebug() << info.path();
            m_sharePathByFileManager.insert(info.path(), info.shareName());
            saveUserShareInfoPathNames();
        }
    }
}

void UserShareManager::deleteUserShareByShareName(const QString &shareName)
{
    QString cmd = "net";
    QStringList args;
    args << "usershare" << "delete"
         << shareName;
    QProcess::startDetached(cmd, args);
}

void UserShareManager::deleteUserShare(const ShareInfo &info)
{
    if (!info.shareName().isEmpty()){
        deleteUserShareByShareName(info.shareName());
    }
}

void UserShareManager::deleteUserShareByPath(const QString &path)
{
    if (m_sharePathByFileManager.contains(path)){
        deleteUserShareByShareName(m_sharePathByFileManager.value(path));
        m_sharePathByFileManager.remove(path);
        saveUserShareInfoPathNames();
    }else{
        QString shareName = getShareNameByPath(path);
        if (!shareName.isEmpty()){
            deleteUserShareByShareName(shareName);
        }
    }
}
