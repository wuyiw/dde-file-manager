#include "dbusfiledialoghandle.h"

DBusFileDialogHandle::DBusFileDialogHandle(QWidget *parent)
    : DFileDialogHandle(parent)
{

}

QString DBusFileDialogHandle::directory() const
{
    return DFileDialogHandle::directory().absolutePath();
}

void DBusFileDialogHandle::setDirectoryUrl(const QString &directory)
{
    DFileDialogHandle::setDirectoryUrl(QUrl(directory));
}

QString DBusFileDialogHandle::directoryUrl() const
{
    return DFileDialogHandle::directoryUrl().toString();
}

void DBusFileDialogHandle::selectUrl(const QString &url)
{
    DFileDialogHandle::selectUrl(QUrl(url));
}

QStringList DBusFileDialogHandle::selectedUrls() const
{
    QStringList list;

    for (const QUrl &url : DFileDialogHandle::selectedUrls())
        list << url.toString();

    return list;
}

int DBusFileDialogHandle::filter() const
{
    return DFileDialogHandle::filter();
}

void DBusFileDialogHandle::setFilter(int filters)
{
    DFileDialogHandle::setFilter(static_cast<QDir::Filters>(filters));
}

void DBusFileDialogHandle::setViewMode(int mode)
{
    DFileDialogHandle::setViewMode(static_cast<QFileDialog::ViewMode>(mode));
}

int DBusFileDialogHandle::viewMode() const
{
    return DFileDialogHandle::viewMode();
}

void DBusFileDialogHandle::setFileMode(int mode)
{
    DFileDialogHandle::setFileMode(static_cast<QFileDialog::FileMode>(mode));
}

void DBusFileDialogHandle::setAcceptMode(int mode)
{
    DFileDialogHandle::setAcceptMode(static_cast<QFileDialog::AcceptMode>(mode));
}

int DBusFileDialogHandle::acceptMode() const
{
    return DFileDialogHandle::acceptMode();
}

void DBusFileDialogHandle::setLabelText(int label, const QString &text)
{
    DFileDialogHandle::setLabelText(static_cast<QFileDialog::DialogLabel>(label), text);
}

QString DBusFileDialogHandle::labelText(int label) const
{
    return DFileDialogHandle::labelText(static_cast<QFileDialog::DialogLabel>(label));
}

void DBusFileDialogHandle::setOptions(int options)
{
    DFileDialogHandle::setOptions(static_cast<QFileDialog::Options>(options));
}

WId DBusFileDialogHandle::winId() const
{
    return widget()->winId();
}

void DBusFileDialogHandle::setWindowTitle(const QString &title)
{
    widget()->setWindowTitle(title);
}