#include <QDir>
#include <QFile>
#include <QtTest>

class PermissionBridgeSourceGuardTest final : public QObject
{
    Q_OBJECT

private
    slots  :


    void permissionBridge_freeFunctions_mustNotUseTraceSelfWithThis();
};

void PermissionBridgeSourceGuardTest::permissionBridge_freeFunctions_mustNotUseTraceSelfWithThis()
{
    const QDir testSourceDir(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath());
    const QDir repoRoot(QDir::cleanPath(testSourceDir.absoluteFilePath(QStringLiteral("../.."))));
    const QStringList sourcePaths{
        repoRoot.absoluteFilePath(QStringLiteral("src/app/permissions/ApplePermissionBridge.mm")),
        repoRoot.absoluteFilePath(QStringLiteral("src/app/permissions/ApplePermissionBridge_stub.cpp"))
    };

    for (const QString& sourcePath : sourcePaths)
    {
        QFile file(sourcePath);
        QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(sourcePath));
        const QString sourceText = QString::fromUtf8(file.readAll());
        QVERIFY2(
            !sourceText.contains(QStringLiteral("traceSelf(this")),
            qPrintable(QStringLiteral(
                    "Permission bridge source must not call traceSelf(this, ...) from free functions: %1")
                .arg(sourcePath)));
    }
}

QTEST_APPLESS_MAIN(PermissionBridgeSourceGuardTest)

#include "test_permission_bridge_source_guard.moc"
