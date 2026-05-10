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
        QStringLiteral("src/app/models/editor/SetProperty.cpp"),
        QStringLiteral("src/app/models/editor/SetProperty.h"),
        QStringLiteral("src/app/models/editor/SetTag.cpp"),
        QStringLiteral("src/app/models/editor/SetTag.h"),
        QStringLiteral("docs/src/app/models/editor/README.md"),
        QStringLiteral("docs/src/app/models/editor/GetProperty.cpp.md"),
        QStringLiteral("docs/src/app/models/editor/GetProperty.h.md"),
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

    const QStringList forbiddenFiles{
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
    for (const QString& relativePath : forbiddenFiles)
    {
        QVERIFY2(
            !QFileInfo::exists(repositoryRoot.filePath(relativePath)),
            qPrintable(QStringLiteral("Removed editor view-mode contract must stay absent: %1").arg(relativePath)));
    }

    const QString qmlContextBinderHeader = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/bootstrap/WhatSonQmlContextBinder.hpp"));
    const QString qmlContextBinderSource = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/bootstrap/WhatSonQmlContextBinder.cpp"));
    QVERIFY(!qmlContextBinderHeader.contains(QStringLiteral("editorViewModeController")));
    QVERIFY(!qmlContextBinderSource.contains(QStringLiteral("editorViewModeController")));
}

void WhatSonCppRegressionTests::sourceTree_keepsContentsQmlUnderViewContents()
{
    const QDir repositoryRoot(repositoryRootPath());
    QVERIFY(repositoryRoot.exists());

    const QString contentsRoot = QStringLiteral("src/app/qml/view/contents");
    const QString docsContentsRoot = QStringLiteral("docs/src/app/qml/view/contents");
    const QStringList requiredPaths{
        contentsRoot + QStringLiteral("/Gutter.qml"),
        contentsRoot + QStringLiteral("/TextEditor.qml"),
        contentsRoot + QStringLiteral("/Minimap.qml"),
        docsContentsRoot + QStringLiteral("/README.md"),
        docsContentsRoot + QStringLiteral("/Gutter.qml.md"),
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
    QCOMPARE(contentsQmlFiles, QStringList({QStringLiteral("Gutter.qml"), QStringLiteral("Minimap.qml"), QStringLiteral("TextEditor.qml")}));
    const QStringList contentsDocsFiles =
        QDir(repositoryRoot.filePath(docsContentsRoot)).entryList(QStringList{QStringLiteral("*.md")}, QDir::Files, QDir::Name);
    QCOMPARE(contentsDocsFiles, QStringList({
        QStringLiteral("Gutter.qml.md"),
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

    const QString namedSupportPath =
        QStringLiteral("src/app/models/file/hierarchy/WhatSonNamedStringHierarchySupport.hpp");
    const QString projectionHeaderPath =
        QStringLiteral("src/app/models/file/hierarchy/library/WhatSonLibraryNoteListProjection.hpp");
    const QString projectionSourcePath =
        QStringLiteral("src/app/models/file/hierarchy/library/WhatSonLibraryNoteListProjection.cpp");
    const QString noteRecordSupportHeaderPath =
        QStringLiteral("src/app/models/file/hierarchy/WhatSonHierarchyNoteRecordSupport.hpp");
    const QString noteRecordSupportSourcePath =
        QStringLiteral("src/app/models/file/hierarchy/WhatSonHierarchyNoteRecordSupport.cpp");
    const QString clipboardSupportHeaderPath =
        QStringLiteral("src/app/models/file/import/WhatSonResourceClipboardImportSupport.hpp");
    const QString clipboardSupportSourcePath =
        QStringLiteral("src/app/models/file/import/WhatSonResourceClipboardImportSupport.cpp");

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
    QVERIFY2(
        QFileInfo::exists(repositoryRoot.filePath(clipboardSupportHeaderPath)),
        "Clipboard image extraction support must stay out of the ResourcesImportController.");
    QVERIFY2(
        QFileInfo::exists(repositoryRoot.filePath(clipboardSupportSourcePath)),
        "Clipboard image extraction support must stay out of the ResourcesImportController.");
    const QString eventSupportSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/hierarchy/event/EventHierarchyControllerSupport.hpp"));
    const QString presetSupportSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/hierarchy/preset/PresetHierarchyControllerSupport.hpp"));
    QVERIFY(eventSupportSource.contains(QStringLiteral("WhatSonNamedStringHierarchySupport.hpp")));
    QVERIFY(presetSupportSource.contains(QStringLiteral("WhatSonNamedStringHierarchySupport.hpp")));

    const QString eventControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/hierarchy/event/EventHierarchyController.cpp"));
    const QString presetControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/hierarchy/preset/PresetHierarchyController.cpp"));
    QVERIFY(!eventControllerSource.contains(QStringLiteral("expandedEventItemKeys")));
    QVERIFY(!eventControllerSource.contains(QStringLiteral("selectedEventIndexForKey")));
    QVERIFY(!presetControllerSource.contains(QStringLiteral("expandedPresetItemKeys")));
    QVERIFY(!presetControllerSource.contains(QStringLiteral("selectedPresetIndexForKey")));

    const QString libraryControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/hierarchy/library/LibraryHierarchyController.cpp"));
    const QString libraryControllerHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/hierarchy/library/LibraryHierarchyController.hpp"));
    const QString projectionSource = readUtf8SourceFile(projectionSourcePath);
    QVERIFY(libraryControllerHeader.contains(QStringLiteral("WhatSonLibraryNoteListProjection")));
    QVERIFY(!libraryControllerHeader.contains(QStringLiteral("m_noteListItemCache")));
    QVERIFY(!libraryControllerSource.contains(QStringLiteral("LibraryHierarchyController::buildNoteListItem(")));
    QVERIFY(projectionSource.contains(QStringLiteral("WhatSonLibraryNoteListProjection::buildNoteListItem")));
    QVERIFY(libraryControllerSource.contains(QStringLiteral("WhatSonHierarchyNoteRecordSupport.hpp")));
    QVERIFY(libraryControllerSource.contains(QStringLiteral("NoteRecordSupport::applyPersistedBodyState")));

    const QString projectsControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/hierarchy/projects/ProjectsHierarchyController.cpp"));
    QVERIFY(projectsControllerSource.contains(QStringLiteral("WhatSonHierarchyNoteRecordSupport.hpp")));
    QVERIFY(projectsControllerSource.contains(QStringLiteral("NoteRecordSupport::applyPersistedBodyState")));
    QVERIFY(!projectsControllerSource.contains(QStringLiteral("int indexOfNoteRecordById(")));

    const QString bookmarksControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/hierarchy/bookmarks/BookmarksHierarchyController.cpp"));
    const QString progressControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/hierarchy/progress/ProgressHierarchyController.cpp"));
    QVERIFY(bookmarksControllerSource.contains(QStringLiteral("WhatSonHierarchyNoteRecordSupport.hpp")));
    QVERIFY(bookmarksControllerSource.contains(QStringLiteral("NoteRecordSupport::applyPersistedBodyState")));
    QVERIFY(progressControllerSource.contains(QStringLiteral("WhatSonHierarchyNoteRecordSupport.hpp")));
    QVERIFY(progressControllerSource.contains(QStringLiteral("NoteRecordSupport::applyPersistedBodyState")));

    const QString resourcesImportControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/import/ResourcesImportController.cpp"));
    QVERIFY(resourcesImportControllerSource.contains(QStringLiteral("WhatSonResourceClipboardImportSupport.hpp")));
    QVERIFY(resourcesImportControllerSource.contains(
        QStringLiteral("ClipboardImportSupport::clipboardContainsImportableImage")));
    QVERIFY(resourcesImportControllerSource.contains(QStringLiteral("ClipboardImportSupport::extractClipboardImage")));
    QVERIFY(!resourcesImportControllerSource.contains(QStringLiteral("bool mimeFormatLooksLikeImage(")));
    QVERIFY(!resourcesImportControllerSource.contains(QStringLiteral("bool extractClipboardImage(")));

    const QString testCMakeSource = readUtf8SourceFile(QStringLiteral("test/cpp/CMakeLists.txt"));
    QVERIFY(testCMakeSource.contains(QStringLiteral("src/app/models/editor/GetProperty.cpp")));
    QVERIFY(testCMakeSource.contains(QStringLiteral("src/app/models/editor/SetProperty.cpp")));
    QVERIFY(testCMakeSource.contains(QStringLiteral("src/app/models/editor/SetTag.cpp")));
    QVERIFY(!testCMakeSource.contains(QStringLiteral("src/app/models/editor/display/minimap")));
}
