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

void WhatSonCppRegressionTests::sourceTree_keepsEditorModelBackendRegistered()
{
    const QDir repositoryRoot(repositoryRootPath());
    QVERIFY(repositoryRoot.exists());

    const QStringList requiredDirectories{
        QStringLiteral("src/app/models/editor"),
        QStringLiteral("docs/src/app/models/editor")
    };
    for (const QString& relativePath : requiredDirectories)
    {
        QVERIFY2(
            QDir(repositoryRoot.filePath(relativePath)).exists(),
            qPrintable(QStringLiteral("Editor model shard must stay present: %1").arg(relativePath)));
    }

    const QStringList requiredFiles{
        QStringLiteral("src/app/models/editor/CMakeLists.txt"),
        QStringLiteral("src/app/models/editor/GetProperty.cpp"),
        QStringLiteral("src/app/models/editor/GetProperty.h"),
        QStringLiteral("src/app/models/editor/component/Callout.cpp"),
        QStringLiteral("src/app/models/editor/component/Callout.h"),
        QStringLiteral("src/app/models/editor/TagInsertionWriter.cpp"),
        QStringLiteral("src/app/models/editor/TagInsertionWriter.hpp"),
        QStringLiteral("src/app/models/editor/SetProperty.cpp"),
        QStringLiteral("src/app/models/editor/SetProperty.h"),
        QStringLiteral("src/app/models/editor/SetTag.cpp"),
        QStringLiteral("src/app/models/editor/SetTag.h"),
        QStringLiteral("docs/src/app/models/editor/README.md"),
        QStringLiteral("docs/src/app/models/editor/GetProperty.cpp.md"),
        QStringLiteral("docs/src/app/models/editor/GetProperty.h.md"),
        QStringLiteral("docs/src/app/models/editor/component/Callout.cpp.md"),
        QStringLiteral("docs/src/app/models/editor/component/Callout.h.md"),
        QStringLiteral("docs/src/app/models/editor/TagInsertionWriter.cpp.md"),
        QStringLiteral("docs/src/app/models/editor/TagInsertionWriter.hpp.md"),
        QStringLiteral("docs/src/app/models/editor/SetProperty.cpp.md"),
        QStringLiteral("docs/src/app/models/editor/SetProperty.h.md"),
        QStringLiteral("docs/src/app/models/editor/SetTag.cpp.md"),
        QStringLiteral("docs/src/app/models/editor/SetTag.h.md")
    };
    for (const QString& relativePath : requiredFiles)
    {
        QVERIFY2(
            QFileInfo::exists(repositoryRoot.filePath(relativePath)),
            qPrintable(QStringLiteral("Editor model shard file must stay present: %1").arg(relativePath)));
    }

    const QString appCmakeSource = readUtf8SourceFile(QStringLiteral("src/app/CMakeLists.txt"));
    QVERIFY(appCmakeSource.contains(QStringLiteral("add_subdirectory(models/editor)")));
    QVERIFY(!appCmakeSource.contains(QStringLiteral("models/editor/GetProperty.cpp")));
    QVERIFY(!appCmakeSource.contains(QStringLiteral("models/editor/GetProperty.h")));
    QVERIFY(!appCmakeSource.contains(QStringLiteral("models/editor/component/Callout.cpp")));
    QVERIFY(!appCmakeSource.contains(QStringLiteral("models/editor/component/Callout.h")));
    QVERIFY(!appCmakeSource.contains(QStringLiteral("models/editor/TagInsertionWriter.cpp")));
    QVERIFY(!appCmakeSource.contains(QStringLiteral("models/editor/TagInsertionWriter.hpp")));
    QVERIFY(!appCmakeSource.contains(QStringLiteral("models/editor/SetProperty.cpp")));
    QVERIFY(!appCmakeSource.contains(QStringLiteral("models/editor/SetProperty.h")));
    QVERIFY(!appCmakeSource.contains(QStringLiteral("models/editor/SetTag.cpp")));
    QVERIFY(!appCmakeSource.contains(QStringLiteral("models/editor/SetTag.h")));

    const QString editorCmakeSource = readUtf8SourceFile(QStringLiteral("src/app/models/editor/CMakeLists.txt"));
    QVERIFY(editorCmakeSource.contains(QStringLiteral(
        "whatson_app_register_directory_sources(\"${CMAKE_CURRENT_SOURCE_DIR}\" RECURSE)")));
    QVERIFY(editorCmakeSource.contains(QStringLiteral(
        "whatson_app_register_directory_include_directories(\"${CMAKE_CURRENT_SOURCE_DIR}\")")));

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
            qPrintable(QStringLiteral("Removed editor minimap backend directory must stay absent: %1").arg(relativePath)));
    }

    QVERIFY(!appCmakeSource.contains(QStringLiteral("models/minimap")));

    const QStringList requiredEditorViewModeFiles{
        QStringLiteral("src/app/models/navigationbar/EditorViewModeController.cpp"),
        QStringLiteral("src/app/models/navigationbar/EditorViewModeController.hpp"),
        QStringLiteral("src/app/models/navigationbar/EditorViewSectionController.cpp"),
        QStringLiteral("src/app/models/navigationbar/EditorViewSectionController.hpp"),
        QStringLiteral("src/app/models/navigationbar/EditorViewState.cpp"),
        QStringLiteral("src/app/models/navigationbar/EditorViewState.hpp"),
        QStringLiteral("src/app/qml/view/panels/navigation/NavigationEditorViewBar.qml"),
        QStringLiteral("docs/src/app/models/navigationbar/EditorViewModeController.cpp.md"),
        QStringLiteral("docs/src/app/models/navigationbar/EditorViewModeController.hpp.md"),
        QStringLiteral("docs/src/app/models/navigationbar/EditorViewSectionController.cpp.md"),
        QStringLiteral("docs/src/app/models/navigationbar/EditorViewSectionController.hpp.md"),
        QStringLiteral("docs/src/app/models/navigationbar/EditorViewState.cpp.md"),
        QStringLiteral("docs/src/app/models/navigationbar/EditorViewState.hpp.md"),
        QStringLiteral("docs/src/app/qml/view/panels/navigation/NavigationEditorViewBar.qml.md"),
        QStringLiteral("test/cpp/suites/editor_view_mode_controller_tests.cpp")
    };
    for (const QString& relativePath : requiredEditorViewModeFiles)
    {
        QVERIFY2(
            QFileInfo::exists(repositoryRoot.filePath(relativePath)),
            qPrintable(QStringLiteral("Restored editor view-mode contract must stay present: %1").arg(relativePath)));
    }

    const QString qmlContextBinderHeader = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/bootstrap/WhatSonQmlContextBinder.hpp"));
    const QString qmlContextBinderSource = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/bootstrap/WhatSonQmlContextBinder.cpp"));
    QVERIFY(qmlContextBinderHeader.contains(QStringLiteral("editorViewModeController")));
    QVERIFY(qmlContextBinderSource.contains(QStringLiteral("editorViewModeController")));

    const QString testCmakeSource = readUtf8SourceFile(QStringLiteral("test/cpp/CMakeLists.txt"));
    QVERIFY(testCmakeSource.contains(QStringLiteral("EditorViewModeController.cpp")));
    QVERIFY(testCmakeSource.contains(QStringLiteral("EditorViewSectionController.cpp")));
    QVERIFY(testCmakeSource.contains(QStringLiteral("EditorViewState.cpp")));
}

void WhatSonCppRegressionTests::sourceTree_keepsContentsQmlUnderViewContents()
{
    const QDir repositoryRoot(repositoryRootPath());
    QVERIFY(repositoryRoot.exists());

    const QString contentsRoot = QStringLiteral("src/app/qml/view/contents");
    const QString docsContentsRoot = QStringLiteral("docs/src/app/qml/view/contents");
    const QStringList requiredPaths{
        contentsRoot + QStringLiteral("/Gutter.qml"),
        contentsRoot + QStringLiteral("/ImageEditor.qml"),
        contentsRoot + QStringLiteral("/TextEditor.qml"),
        contentsRoot + QStringLiteral("/Minimap.qml"),
        docsContentsRoot + QStringLiteral("/README.md"),
        docsContentsRoot + QStringLiteral("/Gutter.qml.md"),
        docsContentsRoot + QStringLiteral("/ImageEditor.qml.md"),
        docsContentsRoot + QStringLiteral("/TextEditor.qml.md"),
        docsContentsRoot + QStringLiteral("/Minimap.qml.md")
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
        contentsRoot + QStringLiteral("/editor"),
        docsQmlRoot + QStringLiteral("contents"),
        docsQmlRoot + QStringLiteral("view/content"),
        docsContentsRoot + QStringLiteral("/editor")
    };
    for (const QString& relativePath : forbiddenDirectories)
    {
        QVERIFY2(
            !QDir(repositoryRoot.filePath(relativePath)).exists(),
            qPrintable(QStringLiteral("Contents QML must stay under view/contents: %1").arg(relativePath)));
    }

    const QStringList contentsQmlFiles =
        QDir(repositoryRoot.filePath(contentsRoot)).entryList(QStringList{QStringLiteral("*.qml")}, QDir::Files, QDir::Name);
    QCOMPARE(contentsQmlFiles, QStringList({
        QStringLiteral("Gutter.qml"),
        QStringLiteral("ImageEditor.qml"),
        QStringLiteral("Minimap.qml"),
        QStringLiteral("TextEditor.qml")
    }));
    const QStringList contentsDocsFiles =
        QDir(repositoryRoot.filePath(docsContentsRoot)).entryList(QStringList{QStringLiteral("*.md")}, QDir::Files, QDir::Name);
    QCOMPARE(contentsDocsFiles, QStringList({
        QStringLiteral("Gutter.qml.md"),
        QStringLiteral("ImageEditor.qml.md"),
        QStringLiteral("Minimap.qml.md"),
        QStringLiteral("README.md"),
        QStringLiteral("TextEditor.qml.md")
    }));

    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(contentsRoot + QStringLiteral("/ContentsView.qml"))));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(contentsRoot + QStringLiteral("/EditorView.qml"))));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(contentsRoot + QStringLiteral("/editor/ContentsInlineFormatEditor.qml"))));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(
        contentsRoot + QStringLiteral("/editor/ContentsDisplayView.qml"))));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(contentsRoot + QStringLiteral("/editor/ContentsLineNumberRail.qml"))));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(contentsRoot + QStringLiteral("/editor/ContentsStructuredDocumentFlow.qml"))));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(contentsRoot + QStringLiteral("/editor/ContentsResourceEditorView.qml"))));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(contentsRoot + QStringLiteral("/editor/ContentsResourceViewer.qml"))));
}

void WhatSonCppRegressionTests::sourceTree_keepsHierarchyBackendDecomposed()
{
    const QDir repositoryRoot(repositoryRootPath());
    QVERIFY(repositoryRoot.exists());

    QVERIFY2(
        QDir(repositoryRoot.filePath(QStringLiteral("src/app/models/hierarchy"))).exists(),
        "Hierarchy backend must live in its own model shard.");
    QVERIFY2(
        QDir(repositoryRoot.filePath(QStringLiteral("docs/src/app/models/hierarchy"))).exists(),
        "Hierarchy backend documentation must mirror the standalone model shard.");
    QVERIFY2(
        !QDir(repositoryRoot.filePath(QStringLiteral("src/app/models/file/hierarchy"))).exists(),
        "Hierarchy backend must not be nested under the persistent file-storage shard.");
    QVERIFY2(
        !QDir(repositoryRoot.filePath(QStringLiteral("docs/src/app/models/file/hierarchy"))).exists(),
        "Hierarchy backend docs must not be nested under the persistent file-storage docs.");
    QVERIFY2(
        !QDir(repositoryRoot.filePath(QStringLiteral("src/app/models/file/resource"))).exists()
            || QDir(repositoryRoot.filePath(QStringLiteral("src/app/models/file/resource"))).entryList(
                   QStringList{
                       QStringLiteral("ResourcesImportController.*"),
                       QStringLiteral("WhatSonResourceClipboardImportSupport.*"),
                       QStringLiteral("WhatSonClipboardResourceImportFileNamePolicy.*")
                   },
                   QDir::Files | QDir::NoDotAndDotDot).isEmpty(),
        "The deleted resource import controller files must not remain in the file/resource shard.");
    QVERIFY2(
        !QDir(repositoryRoot.filePath(QStringLiteral("docs/src/app/models/file/resource"))).exists()
            || QDir(repositoryRoot.filePath(QStringLiteral("docs/src/app/models/file/resource"))).entryList(
                   QStringList{
                       QStringLiteral("ResourcesImportController.*"),
                       QStringLiteral("WhatSonResourceClipboardImportSupport.*"),
                       QStringLiteral("WhatSonClipboardResourceImportFileNamePolicy.*")
                   },
                   QDir::Files | QDir::NoDotAndDotDot).isEmpty(),
        "The deleted resource import controller docs must not remain in file/resource docs.");
    QVERIFY2(
        QDir(repositoryRoot.filePath(QStringLiteral("docs/src/app/models/clipboard"))).exists(),
        "Clipboard import documentation must mirror the clipboard model shard.");
    const QString legacyResourceImportSourceDirectory =
        QStringLiteral("src/app/models/file") + QStringLiteral("/import");
    const QString legacyResourceImportDocsDirectory =
        QStringLiteral("docs/src/app/models/file") + QStringLiteral("/import");
    QVERIFY2(
        !QDir(repositoryRoot.filePath(legacyResourceImportSourceDirectory)).exists(),
        "Resource file support must not use the old import model shard.");
    QVERIFY2(
        !QDir(repositoryRoot.filePath(legacyResourceImportDocsDirectory)).exists(),
        "Resource file support docs must not use the old import model shard.");

    const QString namedSupportPath =
        QStringLiteral("src/app/models/hierarchy/WhatSonNamedStringHierarchySupport.hpp");
    const QString projectionHeaderPath =
        QStringLiteral("src/app/models/hierarchy/library/WhatSonLibraryNoteListProjection.hpp");
    const QString projectionSourcePath =
        QStringLiteral("src/app/models/hierarchy/library/WhatSonLibraryNoteListProjection.cpp");
    const QString noteRecordSupportHeaderPath =
        QStringLiteral("src/app/models/hierarchy/WhatSonHierarchyNoteRecordSupport.hpp");
    const QString noteRecordSupportSourcePath =
        QStringLiteral("src/app/models/hierarchy/WhatSonHierarchyNoteRecordSupport.cpp");

    QVERIFY2(
        QFileInfo::exists(repositoryRoot.filePath(namedSupportPath)),
        "Named string hierarchy support must stay shared across simple hierarchy domains.");
    QVERIFY2(
        QFileInfo::exists(repositoryRoot.filePath(projectionHeaderPath)),
        "Library note-list projection must stay out of the LibraryHierarchyController header.");
    QVERIFY2(
        QFileInfo::exists(repositoryRoot.filePath(projectionSourcePath)),
        "Library note-list projection must stay out of the LibraryHierarchyController implementation.");
    QVERIFY2(
        QFileInfo::exists(repositoryRoot.filePath(noteRecordSupportHeaderPath)),
        "Repeated note-record lookup/body-state support must stay out of hierarchy controllers.");
    QVERIFY2(
        QFileInfo::exists(repositoryRoot.filePath(noteRecordSupportSourcePath)),
        "Repeated note-record lookup/body-state support must stay out of hierarchy controllers.");
    const QString eventSupportSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/hierarchy/event/EventHierarchyControllerSupport.hpp"));
    const QString presetSupportSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/hierarchy/preset/PresetHierarchyControllerSupport.hpp"));
    QVERIFY(eventSupportSource.contains(QStringLiteral("WhatSonNamedStringHierarchySupport.hpp")));
    QVERIFY(presetSupportSource.contains(QStringLiteral("WhatSonNamedStringHierarchySupport.hpp")));

    const QString eventControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/hierarchy/event/EventHierarchyController.cpp"));
    const QString presetControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/hierarchy/preset/PresetHierarchyController.cpp"));
    QVERIFY(!eventControllerSource.contains(QStringLiteral("expandedEventItemKeys")));
    QVERIFY(!eventControllerSource.contains(QStringLiteral("selectedEventIndexForKey")));
    QVERIFY(!presetControllerSource.contains(QStringLiteral("expandedPresetItemKeys")));
    QVERIFY(!presetControllerSource.contains(QStringLiteral("selectedPresetIndexForKey")));

    const QString libraryControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/hierarchy/library/LibraryHierarchyController.cpp"));
    const QString libraryControllerHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/hierarchy/library/LibraryHierarchyController.hpp"));
    const QString projectionSource = readUtf8SourceFile(projectionSourcePath);
    QVERIFY(libraryControllerHeader.contains(QStringLiteral("WhatSonLibraryNoteListProjection")));
    QVERIFY(!libraryControllerHeader.contains(QStringLiteral("m_noteListItemCache")));
    QVERIFY(!libraryControllerSource.contains(QStringLiteral("LibraryHierarchyController::buildNoteListItem(")));
    QVERIFY(projectionSource.contains(QStringLiteral("WhatSonLibraryNoteListProjection::buildNoteListItem")));
    QVERIFY(libraryControllerSource.contains(QStringLiteral("WhatSonHierarchyNoteRecordSupport.hpp")));
    QVERIFY(libraryControllerSource.contains(QStringLiteral("NoteRecordSupport::applyPersistedBodyState")));

    const QString projectsControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/hierarchy/projects/ProjectsHierarchyController.cpp"));
    QVERIFY(projectsControllerSource.contains(QStringLiteral("WhatSonHierarchyNoteRecordSupport.hpp")));
    QVERIFY(projectsControllerSource.contains(QStringLiteral("NoteRecordSupport::applyPersistedBodyState")));
    QVERIFY(!projectsControllerSource.contains(QStringLiteral("int indexOfNoteRecordById(")));

    const QString bookmarksControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/hierarchy/bookmarks/BookmarksHierarchyController.cpp"));
    const QString progressControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/hierarchy/progress/ProgressHierarchyController.cpp"));
    QVERIFY(bookmarksControllerSource.contains(QStringLiteral("WhatSonHierarchyNoteRecordSupport.hpp")));
    QVERIFY(bookmarksControllerSource.contains(QStringLiteral("NoteRecordSupport::applyPersistedBodyState")));
    QVERIFY(progressControllerSource.contains(QStringLiteral("WhatSonHierarchyNoteRecordSupport.hpp")));
    QVERIFY(progressControllerSource.contains(QStringLiteral("NoteRecordSupport::applyPersistedBodyState")));

    const QString clipboardSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/clipboard/InAppClipboardManager.cpp"));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("src/app/models/file/resource/ResourcesImportController.hpp"))));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("src/app/models/file/resource/ResourcesImportController.cpp"))));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("docs/src/app/models/file/resource/ResourcesImportController.hpp.md"))));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("docs/src/app/models/file/resource/ResourcesImportController.cpp.md"))));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("src/app/models/file/resource/WhatSonResourceClipboardImportSupport.hpp"))));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("src/app/models/file/resource/WhatSonResourceClipboardImportSupport.cpp"))));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("src/app/models/file/resource/WhatSonClipboardResourceImportFileNamePolicy.hpp"))));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("src/app/models/file/resource/WhatSonClipboardResourceImportFileNamePolicy.cpp"))));
    QVERIFY(QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("src/app/models/clipboard/InAppClipboardManager.h"))));
    QVERIFY(QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("src/app/models/clipboard/InAppClipboardManager.cpp"))));
    QVERIFY(QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("src/app/models/clipboard/InAppClipboardStore.h"))));
    QVERIFY(QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("src/app/models/clipboard/InAppClipboardStore.cpp"))));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("src/app/models/clipboard/InAppClipboard.h"))));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("src/app/models/clipboard/InAppClipboard.cpp"))));
    QVERIFY(QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("src/app/models/clipboard/ClipboardResourceImport.h"))));
    QVERIFY(QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("src/app/models/clipboard/ClipboardResourceImport.cpp"))));
    QVERIFY(QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("src/app/models/clipboard/FiletypeCapture.h"))));
    QVERIFY(QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("src/app/models/clipboard/FiletypeCapture.cpp"))));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("src/app/models/clipboard/InAppClipboardResourceImport.cpp"))));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("src/app/models/clipboard/ClipboardResourcePackageImport.cpp"))));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("docs/src/app/models/clipboard/ClipboardResourcePackageImport.cpp.md"))));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("docs/src/app/models/clipboard/InAppClipboard.h.md"))));
    QVERIFY(!QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("docs/src/app/models/clipboard/InAppClipboard.cpp.md"))));
    QVERIFY(QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("docs/src/app/models/clipboard/InAppClipboardManager.h.md"))));
    QVERIFY(QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("docs/src/app/models/clipboard/InAppClipboardManager.cpp.md"))));
    QVERIFY(QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("docs/src/app/models/clipboard/InAppClipboardStore.h.md"))));
    QVERIFY(QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("docs/src/app/models/clipboard/InAppClipboardStore.cpp.md"))));
    QVERIFY(QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("docs/src/app/models/clipboard/FiletypeCapture.h.md"))));
    QVERIFY(QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("docs/src/app/models/clipboard/FiletypeCapture.cpp.md"))));
    QVERIFY(QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("docs/src/app/models/clipboard/ClipboardEditorPaste.h.md"))));
    QVERIFY(QFileInfo::exists(repositoryRoot.filePath(
        QStringLiteral("docs/src/app/models/clipboard/ClipboardEditorPaste.cpp.md"))));
    QVERIFY(!clipboardSource.contains(QStringLiteral("WhatSonResourceClipboardImportSupport")));
    QVERIFY(!clipboardSource.contains(QStringLiteral("ClipboardImportSupport::")));
    QVERIFY(!clipboardSource.contains(QStringLiteral("importClipboardImage")));
    QVERIFY(!clipboardSource.contains(QStringLiteral("refreshClipboardImageAvailability")));
    QVERIFY(!clipboardSource.contains(QStringLiteral("bool mimeFormatLooksLikeImage(")));
    QVERIFY(!clipboardSource.contains(QStringLiteral("bool extractClipboardImage(")));
    QVERIFY(!clipboardSource.contains(QStringLiteral("ResourcesImportController")));
    QVERIFY(!clipboardSource.contains(QStringLiteral("resources.import")));
    QVERIFY(clipboardSource.contains(QStringLiteral("clipboard.import")));
    QVERIFY(clipboardSource.contains(QStringLiteral("InAppClipboardManager::importClipboardResourceForEditor")));
    QVERIFY(clipboardSource.contains(QStringLiteral("InAppClipboardManager::importUrlsForEditor")));

    const QString appCmakeSource = readUtf8SourceFile(QStringLiteral("src/app/CMakeLists.txt"));
    QVERIFY(appCmakeSource.contains(QStringLiteral("add_subdirectory(models/file)")));
    QVERIFY(appCmakeSource.contains(QStringLiteral("add_subdirectory(models/hierarchy)")));
    const QString hierarchyCmakeSource = readUtf8SourceFile(QStringLiteral("src/app/models/hierarchy/CMakeLists.txt"));
    QVERIFY(hierarchyCmakeSource.contains(QStringLiteral(
        "whatson_app_register_directory_sources(\"${CMAKE_CURRENT_SOURCE_DIR}\" RECURSE)")));
    QVERIFY(hierarchyCmakeSource.contains(QStringLiteral(
        "whatson_app_register_directory_include_directories(\"${CMAKE_CURRENT_SOURCE_DIR}\")")));

    const QString testCMakeSource = readUtf8SourceFile(QStringLiteral("test/cpp/CMakeLists.txt"));
    QVERIFY(!testCMakeSource.contains(QStringLiteral(
        "src/app/models/file/resource/WhatSonClipboardResourceImportFileNamePolicy.cpp")));
    QVERIFY(!testCMakeSource.contains(QStringLiteral(
        "src/app/models/file/resource/WhatSonResourceClipboardImportSupport.cpp")));
    QVERIFY(!testCMakeSource.contains(legacyResourceImportSourceDirectory));
    QVERIFY(testCMakeSource.contains(QStringLiteral("src/app/models/clipboard/FiletypeCapture.cpp")));
    QVERIFY(testCMakeSource.contains(QStringLiteral("src/app/models/clipboard/ClipboardEditorPaste.cpp")));
    QVERIFY(testCMakeSource.contains(QStringLiteral("src/app/models/clipboard/InAppClipboardStore.cpp")));
    QVERIFY(testCMakeSource.contains(QStringLiteral("src/app/models/clipboard/InAppClipboardManager.cpp")));
    QVERIFY(!testCMakeSource.contains(QStringLiteral("src/app/models/clipboard/InAppClipboard.cpp")));
    QVERIFY(!testCMakeSource.contains(QStringLiteral("src/app/models/clipboard/ClipboardResourcePackageImport.cpp")));
    QVERIFY(!testCMakeSource.contains(QStringLiteral("src/app/models/clipboard/InAppClipboardResourceImport.cpp")));
    QVERIFY(testCMakeSource.contains(QStringLiteral("src/app/models/clipboard/ClipboardResourceImport.cpp")));
    QVERIFY(!testCMakeSource.contains(QStringLiteral("src/app/models/file/resource/ResourcesImportController.cpp")));
    QVERIFY(testCMakeSource.contains(QStringLiteral("src/app/models/editor/GetProperty.cpp")));
    QVERIFY(testCMakeSource.contains(QStringLiteral("src/app/models/editor/component/Callout.cpp")));
    QVERIFY(testCMakeSource.contains(QStringLiteral("src/app/models/editor/TagInsertionWriter.cpp")));
    QVERIFY(testCMakeSource.contains(QStringLiteral("src/app/models/editor/SetProperty.cpp")));
    QVERIFY(testCMakeSource.contains(QStringLiteral("src/app/models/editor/SetTag.cpp")));
    QVERIFY(!testCMakeSource.contains(QStringLiteral("src/app/models/editor/display/minimap")));
}

void WhatSonCppRegressionTests::sourceTree_keepsNoteFileShardClassifiedByResponsibility()
{
    const QDir repositoryRoot(repositoryRootPath());
    QVERIFY(repositoryRoot.exists());

    const QString noteRoot = QStringLiteral("src/app/models/file/note");
    const QString docsNoteRoot = QStringLiteral("docs/src/app/models/file/note");
    const QStringList requiredSourceDirectories{
        noteRoot + QStringLiteral("/body"),
        noteRoot + QStringLiteral("/folder"),
        noteRoot + QStringLiteral("/header"),
        noteRoot + QStringLiteral("/hub"),
        noteRoot + QStringLiteral("/local"),
        noteRoot + QStringLiteral("/package"),
        noteRoot + QStringLiteral("/session"),
        noteRoot + QStringLiteral("/support")
    };
    for (const QString& relativePath : requiredSourceDirectories)
    {
        QVERIFY2(
            QDir(repositoryRoot.filePath(relativePath)).exists(),
            qPrintable(QStringLiteral("Note file shard directory must stay classified: %1").arg(relativePath)));
    }

    const QStringList requiredDocsDirectories{
        docsNoteRoot + QStringLiteral("/body"),
        docsNoteRoot + QStringLiteral("/folder"),
        docsNoteRoot + QStringLiteral("/header"),
        docsNoteRoot + QStringLiteral("/hub"),
        docsNoteRoot + QStringLiteral("/local"),
        docsNoteRoot + QStringLiteral("/package"),
        docsNoteRoot + QStringLiteral("/session"),
        docsNoteRoot + QStringLiteral("/support")
    };
    for (const QString& relativePath : requiredDocsDirectories)
    {
        QVERIFY2(
            QDir(repositoryRoot.filePath(relativePath)).exists(),
            qPrintable(QStringLiteral("Note file documentation shard directory must stay classified: %1").arg(relativePath)));
    }

    const QStringList requiredFiles{
        noteRoot + QStringLiteral("/body/WhatSonNoteBodyPersistence.cpp"),
        noteRoot + QStringLiteral("/body/WhatSonNoteBodyPersistence.hpp"),
        noteRoot + QStringLiteral("/body/WhatSonNoteBodyResourceTagGenerator.cpp"),
        noteRoot + QStringLiteral("/body/WhatSonNoteBodyResourceTagGenerator.hpp"),
        noteRoot + QStringLiteral("/body/WhatSonNoteBodySemanticTagSupport.cpp"),
        noteRoot + QStringLiteral("/body/WhatSonNoteBodySemanticTagSupport.hpp"),
        noteRoot + QStringLiteral("/body/WhatSonNoteBodyWebLinkSupport.cpp"),
        noteRoot + QStringLiteral("/body/WhatSonNoteBodyWebLinkSupport.hpp"),
        noteRoot + QStringLiteral("/body/WhatSonNoteMarkdownStyleObject.cpp"),
        noteRoot + QStringLiteral("/body/WhatSonNoteMarkdownStyleObject.hpp"),
        noteRoot + QStringLiteral("/folder/WhatSonNoteFolderBindingRepository.cpp"),
        noteRoot + QStringLiteral("/folder/WhatSonNoteFolderBindingRepository.hpp"),
        noteRoot + QStringLiteral("/folder/WhatSonNoteFolderBindingService.cpp"),
        noteRoot + QStringLiteral("/folder/WhatSonNoteFolderBindingService.hpp"),
        noteRoot + QStringLiteral("/folder/WhatSonNoteFolderSemantics.hpp"),
        noteRoot + QStringLiteral("/header/WhatSonBookmarkColorPalette.hpp"),
        noteRoot + QStringLiteral("/header/WhatSonNoteHeaderCreator.cpp"),
        noteRoot + QStringLiteral("/header/WhatSonNoteHeaderCreator.hpp"),
        noteRoot + QStringLiteral("/header/WhatSonNoteHeaderParser.cpp"),
        noteRoot + QStringLiteral("/header/WhatSonNoteHeaderParser.hpp"),
        noteRoot + QStringLiteral("/header/WhatSonNoteHeaderStore.cpp"),
        noteRoot + QStringLiteral("/header/WhatSonNoteHeaderStore.hpp"),
        noteRoot + QStringLiteral("/hub/WhatSonHubNoteCreationService.cpp"),
        noteRoot + QStringLiteral("/hub/WhatSonHubNoteCreationService.hpp"),
        noteRoot + QStringLiteral("/hub/WhatSonHubNoteDeletionService.cpp"),
        noteRoot + QStringLiteral("/hub/WhatSonHubNoteDeletionService.hpp"),
        noteRoot + QStringLiteral("/hub/WhatSonHubNoteFolderClearService.cpp"),
        noteRoot + QStringLiteral("/hub/WhatSonHubNoteFolderClearService.hpp"),
        noteRoot + QStringLiteral("/hub/WhatSonHubNoteMutationSupport.cpp"),
        noteRoot + QStringLiteral("/hub/WhatSonHubNoteMutationSupport.hpp"),
        noteRoot + QStringLiteral("/local/WhatSonLocalNoteDocument.hpp"),
        noteRoot + QStringLiteral("/local/WhatSonLocalNoteFileStore.cpp"),
        noteRoot + QStringLiteral("/local/WhatSonLocalNoteFileStore.hpp"),
        noteRoot + QStringLiteral("/package/WhatSonNoteBodyCreator.cpp"),
        noteRoot + QStringLiteral("/package/WhatSonNoteBodyCreator.hpp"),
        noteRoot + QStringLiteral("/package/WhatSonNoteCreator.cpp"),
        noteRoot + QStringLiteral("/package/WhatSonNoteCreator.hpp"),
        noteRoot + QStringLiteral("/session/ContentsNoteManagementCoordinator.cpp"),
        noteRoot + QStringLiteral("/session/ContentsNoteManagementCoordinator.hpp"),
        noteRoot + QStringLiteral("/support/WhatSonIiXmlDocumentSupport.cpp"),
        noteRoot + QStringLiteral("/support/WhatSonIiXmlDocumentSupport.hpp")
    };
    for (const QString& relativePath : requiredFiles)
    {
        QVERIFY2(
            QFileInfo::exists(repositoryRoot.filePath(relativePath)),
            qPrintable(QStringLiteral("Classified note file must stay present: %1").arg(relativePath)));
    }

    const QStringList flatNoteFiles =
        QDir(repositoryRoot.filePath(noteRoot)).entryList(
            QStringList{QStringLiteral("*.cpp"), QStringLiteral("*.hpp"), QStringLiteral("*.h")},
            QDir::Files,
            QDir::Name);
    QVERIFY2(
        flatNoteFiles.isEmpty(),
        qPrintable(QStringLiteral("Note file shard root must not keep flat C++ files: %1").arg(flatNoteFiles.join(QLatin1Char(',')))));

    const QString testCMakeSource = readUtf8SourceFile(QStringLiteral("test/cpp/CMakeLists.txt"));
    QVERIFY(testCMakeSource.contains(QStringLiteral("src/app/models/file/note/body/WhatSonNoteBodyPersistence.cpp")));
    QVERIFY(testCMakeSource.contains(QStringLiteral("src/app/models/file/note/header/WhatSonNoteHeaderCreator.cpp")));
    QVERIFY(testCMakeSource.contains(QStringLiteral("src/app/models/file/note/session/ContentsNoteManagementCoordinator.cpp")));
    const QString flatNoteBodyPersistenceSource =
        QStringLiteral("src/app/models/file/note/") + QStringLiteral("WhatSonNoteBodyPersistence.cpp");
    QVERIFY(!testCMakeSource.contains(flatNoteBodyPersistenceSource));
}

void WhatSonCppRegressionTests::sourceTree_forbidsSharedFileIoObjectLayer()
{
    const QDir repositoryRoot(repositoryRootPath());
    QVERIFY(repositoryRoot.exists());

    const QString obsoleteSourceDirectory =
        QStringLiteral("src/app/models/file") + QStringLiteral("/IO");
    const QString obsoleteDocsDirectory =
        QStringLiteral("docs/src/app/models/file") + QStringLiteral("/IO");
    QVERIFY2(
        !QDir(repositoryRoot.filePath(obsoleteSourceDirectory)).exists(),
        "The old shared file IO model directory must stay removed.");
    QVERIFY2(
        !QDir(repositoryRoot.filePath(obsoleteDocsDirectory)).exists(),
        "The old shared file IO documentation directory must stay removed.");

    const QString obsoleteGatewayName =
        QStringLiteral("WhatSonSystem") + QStringLiteral("IoGateway");
    const QString obsoleteRuntimeName =
        QStringLiteral("WhatSonIo") + QStringLiteral("RuntimeController");
    const QString testCMakeSource = readUtf8SourceFile(QStringLiteral("test/cpp/CMakeLists.txt"));
    QVERIFY(!testCMakeSource.contains(obsoleteGatewayName));
    QVERIFY(!testCMakeSource.contains(obsoleteRuntimeName));
}
