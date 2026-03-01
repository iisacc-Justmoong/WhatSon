#pragma once

#include "WhatSonHubStore.hpp"

#include <QDir>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVector>

struct WhatSonFolderDepthEntry;
struct WhatSonTagDepthEntry;

class WhatSonHubParser final : public QObject
{
    Q_OBJECT

public:
    explicit WhatSonHubParser(QObject* parent = nullptr);
    ~WhatSonHubParser() override;

    bool parseFromWshub(
        const QString& wshubPath,
        WhatSonHubStore* outStore,
        QString* errorMessage = nullptr) const;

    static QString normalizeHubPath(const QString& hubPath);

public
    slots  :



    void requestParseFromWshub(const QString& wshubPath);

    signals  :



    void hubParsed(const QString& wshubPath, const QVariantMap& hubPayload);
    void hubStatParsed(const QString& wshubPath, const QVariantMap& statPayload);
    void hubDomainsParsed(const QString& wshubPath, const QVariantMap& domainPayload);
    void parseFailed(const QString& wshubPath, const QString& errorMessage);

private:
    static QString resolvePrimaryDirectory(
        const QDir& hubDir,
        const QString& fixedDirectoryName,
        const QString& dynamicPattern);
    static QString resolveStatPath(const QDir& hubDir, const QString& hubName);
    static QString firstString(const QJsonObject& object, const QStringList& keys);
    static int firstInt(const QJsonObject& object, const QStringList& keys, int fallbackValue = -1);
    static QStringList firstStringList(const QJsonObject& object, const QStringList& keys);
    static QVariantMap firstObjectMap(const QJsonObject& object, const QStringList& keys);
    static int countFilesRecursive(const QString& rootPath, const QStringList& nameFilters);
    static int countNoteDirectories(const QString& libraryPath);
    static int countWsnbodyCharacters(const QString& libraryPath);
    static QStringList listRelativeFilesRecursive(const QString& rootPath);
    static QVariantMap buildHubPayload(const WhatSonHubStore& store);
    static QVariantMap buildStatPayload(const WhatSonHubStat& stat);
    static QVariantMap buildDomainPayload(
        const QString& contentsPath,
        const QString& libraryPath,
        const QString& resourcesPath,
        QString* errorMessage);
    static bool readUtf8File(const QString& filePath, QString* outText, QString* errorMessage);
    static QString requireEntryPath(
        const QString& basePath,
        const QString& entryName,
        bool allowDirectory,
        bool allowFile,
        QString* errorMessage);
    static QVariantList toFolderEntryList(const QVector<WhatSonFolderDepthEntry>& entries);
    static QVariantList toTagEntryList(const QVector<WhatSonTagDepthEntry>& entries);

    bool parseStatText(
        const QString& rawText,
        const QString& hubName,
        const QString& libraryPath,
        const QString& resourcesPath,
        WhatSonHubStat* outStat,
        QString* errorMessage) const;
};
