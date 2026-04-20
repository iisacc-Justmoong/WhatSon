#include "whatson_cpp_regression_tests.hpp"

QString WhatSonCppRegressionTests::createMinimalHubFixture(
    const QString& workspaceRootPath,
    const QString& hubDirectoryName,
    QString* errorMessage)
{
    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }

    const QString hubPath = QDir(workspaceRootPath).filePath(hubDirectoryName);
    const QString contentsDirectoryPath = QDir(hubPath).filePath(QStringLiteral(".wscontents"));
    const QString libraryDirectoryPath = QDir(contentsDirectoryPath).filePath(QStringLiteral("Library.wslibrary"));
    const QString presetDirectoryPath = QDir(contentsDirectoryPath).filePath(QStringLiteral("Preset.wspreset"));
    const QString resourcesDirectoryPath = QDir(hubPath).filePath(QStringLiteral(".wsresources"));
    for (const QString& directoryPath : {
             hubPath,
             contentsDirectoryPath,
             libraryDirectoryPath,
             presetDirectoryPath,
             resourcesDirectoryPath })
    {
        if (QDir().mkpath(directoryPath))
        {
            continue;
        }

        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to create regression directory: %1").arg(directoryPath);
        }
        return {};
    }

    const auto writeFixtureFile = [errorMessage](const QString& filePath) -> bool
    {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to open regression fixture file: %1").arg(filePath);
            }
            return false;
        }
        if (file.write(QByteArray()) < 0)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to write regression fixture file: %1").arg(filePath);
            }
            return false;
        }
        file.close();
        return true;
    };

    const QString hubBaseName = QFileInfo(hubPath).completeBaseName();
    for (const QString& filePath : {
             QDir(contentsDirectoryPath).filePath(QStringLiteral("Folders.wsfolders")),
             QDir(contentsDirectoryPath).filePath(QStringLiteral("ProjectLists.wsproj")),
             QDir(contentsDirectoryPath).filePath(QStringLiteral("Bookmarks.wsbookmarks")),
             QDir(contentsDirectoryPath).filePath(QStringLiteral("Tags.wstags")),
             QDir(contentsDirectoryPath).filePath(QStringLiteral("Progress.wsprogress")),
             QDir(libraryDirectoryPath).filePath(QStringLiteral("index.wsnindex")),
             QDir(hubPath).filePath(QStringLiteral("%1.wsstat").arg(hubBaseName)) })
    {
        if (writeFixtureFile(filePath))
        {
            continue;
        }
        return {};
    }

    return hubPath;
}

QString WhatSonCppRegressionTests::createLocalNoteForRegression(
    const QString& parentDirectoryPath,
    const QString& noteId,
    const QString& bodySourceText,
    QString* errorMessage)
{
    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }

    const QString normalizedParentDirectoryPath = QDir::cleanPath(parentDirectoryPath.trimmed());
    const QString normalizedNoteId = noteId.trimmed();
    const QString noteDirectoryPath =
        QDir(normalizedParentDirectoryPath).filePath(QStringLiteral("%1.wsnote").arg(normalizedNoteId));

    WhatSonNoteHeaderStore headerStore;
    headerStore.setNoteId(normalizedNoteId);
    headerStore.setCreatedAt(QStringLiteral("2026-04-18-00-00-00"));
    headerStore.setAuthor(QStringLiteral("WhatSonCppRegressionTests"));
    headerStore.setLastModifiedAt(QStringLiteral("2026-04-18-00-00-00"));
    headerStore.setModifiedBy(QStringLiteral("WhatSonCppRegressionTests"));

    WhatSonLocalNoteFileStore fileStore;
    WhatSonLocalNoteFileStore::CreateRequest request;
    request.noteId = normalizedNoteId;
    request.noteDirectoryPath = noteDirectoryPath;
    request.headerStore = headerStore;
    request.bodyPlainText = bodySourceText;

    if (!fileStore.createNote(std::move(request), nullptr, errorMessage))
    {
        return {};
    }
    return noteDirectoryPath;
}

QJSValue WhatSonCppRegressionTests::evaluateQmlJsLibrary(
    QJSEngine* engine,
    const QString& relativeSourcePath)
{
    if (engine == nullptr)
    {
        return {};
    }

    QFile scriptFile(relativeSourcePath);
    if (!scriptFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return {};
    }

    QString scriptSource = QString::fromUtf8(scriptFile.readAll());
    scriptSource.remove(
        QRegularExpression(QStringLiteral(R"(^\s*\.pragma\s+library\s*\n?)")));

    const QJSValue evaluationResult = engine->evaluate(scriptSource, relativeSourcePath);
    if (evaluationResult.isError())
    {
        return evaluationResult;
    }
    return engine->globalObject();
}

QJSValue WhatSonCppRegressionTests::jsArrayEntry(const QJSValue& arrayValue, const int index)
{
    return arrayValue.property(QString::number(index));
}

QString WhatSonCppRegressionTests::readUtf8SourceFile(const QString& relativeSourcePath)
{
    QFile sourceFile(relativeSourcePath);
    if (!sourceFile.exists())
    {
        QDir repositoryRoot(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath());
        repositoryRoot.cdUp();
        repositoryRoot.cdUp();
        sourceFile.setFileName(repositoryRoot.filePath(relativeSourcePath));
    }
    if (!sourceFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return {};
    }
    return QString::fromUtf8(sourceFile.readAll());
}

QCoreApplication* WhatSonCppRegressionTests::ensureCoreApplication()
{
    if (QCoreApplication::instance() != nullptr)
    {
        return QCoreApplication::instance();
    }

    static int argc = 1;
    static char applicationName[] = "whatson_cpp_regression_tests";
    static char* argv[] = {applicationName, nullptr};
    static std::unique_ptr<QCoreApplication> application;
    application = std::make_unique<QCoreApplication>(argc, argv);
    return application.get();
}
