#pragma once

#include <QObject>
#include <QPointer>
#include <QUrl>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <qqmlparserstatus.h>

class QQmlContext;

class ContentsQmlBackedInputControllerBase : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

public:
    explicit ContentsQmlBackedInputControllerBase(QObject* parent = nullptr);

    void classBegin() override;
    void componentComplete() override;

protected:
    virtual QUrl helperSourceUrl() const = 0;

signals:
    void helperReady();

protected:

    QObject* helperObject() const noexcept;
    void syncHelperProperty(const char* propertyName, const QVariant& value);
    bool connectHelperPropertyNotify(const char* helperPropertyName, const char* signalSignature);

    QVariant helperProperty(const char* propertyName) const;
    QVariant invokeHelperVariant(const char* methodName, const QVariantList& arguments = {}) const;
    QVariantMap invokeHelperVariantMap(const char* methodName, const QVariantList& arguments = {}) const;
    QVariantList invokeHelperVariantList(const char* methodName, const QVariantList& arguments = {}) const;
    bool invokeHelperBool(const char* methodName, const QVariantList& arguments = {}, bool defaultValue = false) const;
    int invokeHelperInt(const char* methodName, const QVariantList& arguments = {}, int defaultValue = 0) const;
    QString invokeHelperString(const char* methodName, const QVariantList& arguments = {}, const QString& defaultValue = {}) const;
    void invokeHelperVoid(const char* methodName, const QVariantList& arguments = {}) const;

private:
    void createHelper();

    QPointer<QObject> m_helperObject;
    bool m_componentComplete = false;
};
