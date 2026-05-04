#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QDirIterator>

#include <algorithm>
#include <utility>

namespace
{
    QString repositoryRootPath()
    {
        QDir repositoryRoot(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath());
        repositoryRoot.cdUp();
        repositoryRoot.cdUp();
        repositoryRoot.cdUp();
        return repositoryRoot.absolutePath();
    }

    bool isHeaderExtension(const QString& suffix)
    {
        const QString normalizedSuffix = suffix.trimmed().toLower();
        return normalizedSuffix == QStringLiteral("h")
            || normalizedSuffix == QStringLiteral("hh")
            || normalizedSuffix == QStringLiteral("hpp")
            || normalizedSuffix == QStringLiteral("hxx");
    }

    bool isSourceExtension(const QString& suffix)
    {
        const QString normalizedSuffix = suffix.trimmed().toLower();
        return normalizedSuffix == QStringLiteral("c")
            || normalizedSuffix == QStringLiteral("cc")
            || normalizedSuffix == QStringLiteral("cpp")
            || normalizedSuffix == QStringLiteral("cxx")
            || normalizedSuffix == QStringLiteral("h")
            || normalizedSuffix == QStringLiteral("hh")
            || normalizedSuffix == QStringLiteral("hpp")
            || normalizedSuffix == QStringLiteral("hxx")
            || normalizedSuffix == QStringLiteral("mm");
    }

    QString canonicalProjectInclude(const QString& repositoryRelativePath)
    {
        static const QString srcAppPrefix = QStringLiteral("src/app/");
        static const QString srcExtensionPrefix = QStringLiteral("src/extension/");
        static const QString testPrefix = QStringLiteral("test/");

        if (repositoryRelativePath.startsWith(srcAppPrefix))
        {
            return QStringLiteral("app/") + repositoryRelativePath.mid(srcAppPrefix.size());
        }
        if (repositoryRelativePath.startsWith(srcExtensionPrefix))
        {
            return QStringLiteral("extension/") + repositoryRelativePath.mid(srcExtensionPrefix.size());
        }
        if (repositoryRelativePath.startsWith(testPrefix))
        {
            return repositoryRelativePath;
        }
        return {};
    }
} // namespace

void WhatSonCppRegressionTests::sourceTree_usesRepositoryAbsoluteProjectIncludes()
{
    const QDir repositoryRoot(repositoryRootPath());
    QVERIFY(repositoryRoot.exists());

    QStringList repositoryHeaderPaths;
    for (const QString& rootPath : {QStringLiteral("src"), QStringLiteral("test")})
    {
        QDirIterator it(
            repositoryRoot.filePath(rootPath),
            QDir::Files | QDir::NoDotAndDotDot,
            QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            const QString absolutePath = it.next();
            const QFileInfo fileInfo(absolutePath);
            if (!isHeaderExtension(fileInfo.suffix()))
            {
                continue;
            }

            repositoryHeaderPaths.append(QDir::cleanPath(repositoryRoot.relativeFilePath(absolutePath)));
        }
    }
    repositoryHeaderPaths.removeDuplicates();
    std::sort(repositoryHeaderPaths.begin(), repositoryHeaderPaths.end());

    auto resolveCanonicalProjectInclude = [&](const QString& sourceFilePath, const QString& includeLiteral) {
        const QFileInfo sourceFileInfo(sourceFilePath);
        const QFileInfo relativeCandidate(sourceFileInfo.absoluteDir(), includeLiteral);
        if (relativeCandidate.exists() && relativeCandidate.isFile() && isHeaderExtension(relativeCandidate.suffix()))
        {
            const QString repositoryRelativeCandidate =
                QDir::cleanPath(repositoryRoot.relativeFilePath(relativeCandidate.canonicalFilePath()));
            const QString canonical = canonicalProjectInclude(repositoryRelativeCandidate);
            if (!canonical.isEmpty())
            {
                return canonical;
            }
        }

        const QString normalizedLiteral = QDir::cleanPath(includeLiteral.trimmed());
        for (const QString& repositoryHeaderPath : std::as_const(repositoryHeaderPaths))
        {
            if (repositoryHeaderPath != normalizedLiteral
                && !repositoryHeaderPath.endsWith(QStringLiteral("/") + normalizedLiteral))
            {
                continue;
            }

            const QString canonical = canonicalProjectInclude(repositoryHeaderPath);
            if (!canonical.isEmpty())
            {
                return canonical;
            }
        }

        return QString{};
    };

    QStringList violations;
    const QRegularExpression includePattern(QStringLiteral(R"include(^\s*#include\s+"([^"]+)")include"));
    for (const QString& rootPath : {QStringLiteral("src"), QStringLiteral("test")})
    {
        QDirIterator it(
            repositoryRoot.filePath(rootPath),
            QDir::Files | QDir::NoDotAndDotDot,
            QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            const QString absolutePath = it.next();
            const QFileInfo fileInfo(absolutePath);
            if (!isSourceExtension(fileInfo.suffix()))
            {
                continue;
            }

            QFile sourceFile(absolutePath);
            if (!sourceFile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                continue;
            }

            int lineNumber = 0;
            while (!sourceFile.atEnd())
            {
                const QString line = QString::fromUtf8(sourceFile.readLine());
                ++lineNumber;

                const QRegularExpressionMatch match = includePattern.match(line);
                if (!match.hasMatch())
                {
                    continue;
                }

                const QString includeLiteral = match.captured(1).trimmed();
                if (includeLiteral.startsWith(QStringLiteral("app/"))
                    || includeLiteral.startsWith(QStringLiteral("extension/"))
                    || includeLiteral.startsWith(QStringLiteral("test/")))
                {
                    continue;
                }

                const QString canonicalInclude =
                    resolveCanonicalProjectInclude(absolutePath, includeLiteral);
                if (canonicalInclude.isEmpty())
                {
                    continue;
                }

                violations.append(
                    QStringLiteral("%1:%2 uses \"%3\" instead of \"%4\"")
                        .arg(QDir::cleanPath(repositoryRoot.relativeFilePath(absolutePath)))
                        .arg(lineNumber)
                        .arg(includeLiteral)
                        .arg(canonicalInclude));
            }
        }
    }

    QVERIFY2(violations.isEmpty(), qPrintable(violations.join(QLatin1Char('\n'))));
}

void WhatSonCppRegressionTests::sourceTree_forbidsDeprecatedPresentationLayerVocabulary()
{
    const QDir repositoryRoot(repositoryRootPath());
    QVERIFY(repositoryRoot.exists());
    QVERIFY2(
        !QDir(repositoryRoot.filePath(QStringLiteral("src/app/") + QStringLiteral("viewmodel"))).exists(),
        "src/app/viewmodel must not be reintroduced");

    const QStringList forbiddenFragments{
        QStringLiteral("View") + QStringLiteral("Model"),
        QStringLiteral("view") + QStringLiteral("Model"),
        QStringLiteral("view") + QStringLiteral("model"),
        QStringLiteral("LV.") + QStringLiteral("ViewModels"),
        QStringLiteral("LV.") + QStringLiteral("Controllers"),
        QStringLiteral("Qml") + QStringLiteral("ViewModel") + QStringLiteral("Binding"),
        QStringLiteral("Qml") + QStringLiteral("Controller") + QStringLiteral("Binding"),
        QStringLiteral("app/") + QStringLiteral("viewmodel"),
        QStringLiteral("src/app/") + QStringLiteral("viewmodel"),
        QStringLiteral("Contents") + QStringLiteral("Gut") + QStringLiteral("ter"),
        QStringLiteral("ContentsRemaining") + QStringLiteral("InputControllers"),
        QStringLiteral("Contents") + QStringLiteral("BreakBlockController"),
        QStringLiteral("ContentsLogical") + QStringLiteral("LineLayoutSupport"),
        QStringLiteral("logicalLine") + QStringLiteral("StartOffsets"),
        QStringLiteral("adoptIncremental") + QStringLiteral("State")
    };
    const QRegularExpression qmlAliasPattern(QStringLiteral(R"(\b[A-Za-z_][A-Za-z0-9_]*Vm\b|\bvm\b)"));

    QStringList violations;
    QDirIterator it(
        repositoryRoot.filePath(QStringLiteral("src/app")),
        QDir::Files | QDir::NoDotAndDotDot,
        QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        const QString absolutePath = it.next();
        const QFileInfo fileInfo(absolutePath);
        const QString suffix = fileInfo.suffix().toLower();
        if (suffix != QStringLiteral("cpp")
            && suffix != QStringLiteral("hpp")
            && suffix != QStringLiteral("qml")
            && suffix != QStringLiteral("js")
            && suffix != QStringLiteral("cmake")
            && fileInfo.fileName() != QStringLiteral("CMakeLists.txt"))
        {
            continue;
        }

        QFile sourceFile(absolutePath);
        if (!sourceFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            continue;
        }

        const QString relativePath = QDir::cleanPath(repositoryRoot.relativeFilePath(absolutePath));
        int lineNumber = 0;
        while (!sourceFile.atEnd())
        {
            const QString line = QString::fromUtf8(sourceFile.readLine());
            ++lineNumber;
            for (const QString& forbiddenFragment : forbiddenFragments)
            {
                if (line.contains(forbiddenFragment))
                {
                    violations.append(
                        QStringLiteral("%1:%2 contains deprecated presentation-layer vocabulary: %3")
                            .arg(relativePath)
                            .arg(lineNumber)
                            .arg(forbiddenFragment));
                }
            }

            if (suffix == QStringLiteral("qml") && qmlAliasPattern.match(line).hasMatch())
            {
                violations.append(
                    QStringLiteral("%1:%2 contains deprecated QML alias suffix")
                        .arg(relativePath)
                        .arg(lineNumber));
            }
        }
    }

    QVERIFY2(violations.isEmpty(), qPrintable(violations.join(QLatin1Char('\n'))));
}

void WhatSonCppRegressionTests::sourceTree_keepsMinimapUnderEditorChromeModels()
{
    const QDir repositoryRoot(repositoryRootPath());
    QVERIFY(repositoryRoot.exists());

    const QStringList requiredPaths{
        QStringLiteral("src/app/models/editor/minimap/ContentsMinimapLayoutMetrics.hpp"),
        QStringLiteral("docs/src/app/models/editor/minimap/README.md")
    };
    for (const QString& relativePath : requiredPaths)
    {
        QVERIFY2(
            QFileInfo::exists(repositoryRoot.filePath(relativePath)),
            qPrintable(QStringLiteral("Missing expected editor chrome model path: %1").arg(relativePath)));
    }

    const QStringList forbiddenDirectories{
        QStringLiteral("src/app/models/minimap"),
        QStringLiteral("src/app/models/editor/display/minimap"),
        QStringLiteral("docs/src/app/models/minimap"),
        QStringLiteral("docs/src/app/models/editor/display/minimap")
    };
    for (const QString& relativePath : forbiddenDirectories)
    {
        QVERIFY2(
            !QDir(repositoryRoot.filePath(relativePath)).exists(),
            qPrintable(QStringLiteral("Minimap model directory must stay under models/editor: %1").arg(relativePath)));
    }

    const QString appCmakeSource = readUtf8SourceFile(QStringLiteral("src/app/CMakeLists.txt"));
    QVERIFY(appCmakeSource.contains(QStringLiteral("models/editor")));
    QVERIFY(!appCmakeSource.contains(QStringLiteral("models/minimap")));
}

void WhatSonCppRegressionTests::sourceTree_keepsContentsQmlUnderViewContents()
{
    const QDir repositoryRoot(repositoryRootPath());
    QVERIFY(repositoryRoot.exists());

    const QString contentsRoot = QStringLiteral("src/app/qml/view/contents");
    const QString docsContentsRoot = QStringLiteral("docs/src/app/qml/view/contents");
    const QStringList requiredPaths{
        contentsRoot + QStringLiteral("/ContentsView.qml"),
        contentsRoot + QStringLiteral("/EditorView.qml"),
        contentsRoot + QStringLiteral("/Minimap.qml"),
        contentsRoot + QStringLiteral("/editor/ContentsDisplayView.qml"),
        contentsRoot + QStringLiteral("/editor/ContentsStructuredDocumentFlow.qml"),
        docsContentsRoot + QStringLiteral("/README.md"),
        docsContentsRoot + QStringLiteral("/editor/README.md")
    };
    for (const QString& relativePath : requiredPaths)
    {
        QVERIFY2(
            QFileInfo::exists(repositoryRoot.filePath(relativePath)),
            qPrintable(QStringLiteral("Missing expected contents QML path: %1").arg(relativePath)));
    }

    const QString qmlRoot = QStringLiteral("src/app/qml/");
    const QString docsQmlRoot = QStringLiteral("docs/src/app/qml/");
    const QStringList forbiddenDirectories{
        qmlRoot + QStringLiteral("contents"),
        qmlRoot + QStringLiteral("view/content"),
        docsQmlRoot + QStringLiteral("contents"),
        docsQmlRoot + QStringLiteral("view/content")
    };
    for (const QString& relativePath : forbiddenDirectories)
    {
        QVERIFY2(
            !QDir(repositoryRoot.filePath(relativePath)).exists(),
            qPrintable(QStringLiteral("Contents QML must stay under view/contents: %1").arg(relativePath)));
    }

    const QString displayViewSource = readUtf8SourceFile(contentsRoot + QStringLiteral("/editor/ContentsDisplayView.qml"));
    QVERIFY(displayViewSource.contains(QStringLiteral("import \"..\" as ContentsChrome")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("../../../") + QStringLiteral("contents")));

    const QString inlineFormatEditorControllerSource =
        readUtf8SourceFile(QStringLiteral("src/app/models/editor/input/ContentsInlineFormatEditorController.cpp"));
    QVERIFY(inlineFormatEditorControllerSource.contains(QStringLiteral("qrc:/qt/qml/WhatSon/App/view/contents/editor/")));
    QVERIFY(!inlineFormatEditorControllerSource.contains(
        QStringLiteral("qrc:/qt/qml/WhatSon/App/view/") + QStringLiteral("content/editor/")));
}
