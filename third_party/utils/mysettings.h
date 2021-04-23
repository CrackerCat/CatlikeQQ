#ifndef MYSETTINGS_H
#define MYSETTINGS_H

#include <QSettings>

class MySettings : public QSettings
{
public:

    MySettings(QSettings::Scope scope, QObject *parent = nullptr)
        : QSettings(scope, parent)
    {}

    MySettings(QObject *parent = nullptr)
        : QSettings(parent)
    {}

    MySettings(const QString &fileName, QSettings::Format format, QObject *parent = nullptr)
        : QSettings(fileName, format, parent)
    {}

    MySettings(QSettings::Format format, QSettings::Scope scope, const QString &organization, const QString &application = QString(), QObject *parent = nullptr)
        : QSettings(format, scope, organization, application, parent)
    {}

    MySettings(QSettings::Scope scope, const QString &organization, const QString &application = QString(), QObject *parent = nullptr)
        : QSettings(scope, organization, application, parent)
    {}

    MySettings(const QString &organization, const QString &application = QString(), QObject *parent = nullptr)
        : QSettings(organization, application, parent)
    {}

    void set(QString key, QVariant var)
    {
        QSettings::setValue(key, var);
    }

    bool b(QString key, QVariant def = QVariant())
    {
        return QSettings::value(key, def).toBool();
    }

    double d(QString key, QVariant def = QVariant())
    {
        return QSettings::value(key, def).toDouble();
    }

    int i(QString key, QVariant def = QVariant())
    {
        return QSettings::value(key, def).toInt();
    }

    qint64 l(QString key, QVariant def = QVariant())
    {
        return QSettings::value(key, def).toLongLong();
    }

    QString s(QString key, QVariant def = QVariant())
    {
        return QSettings::value(key, def).toString();
    }

    void assign(bool& val, QString key)
    {
        val = value(key, val).toBool();
    }

    void assign(double& val, QString key)
    {
        val = value(key, val).toDouble();
    }

    void assign(int& val, QString key)
    {
        val = value(key, val).toInt();
    }

    void assign(qint64& val, QString key)
    {
        val = value(key, val).toLongLong();
    }

    void assign(QString& val, QString key)
    {
        val = value(key, val).toString();
    }
};

#endif // MYSETTINGS_H