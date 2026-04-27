#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QQmlComponent>
#include <QQuickItem>
#include <QQuickWindow>
#include <QtQml/qqml.h>

#include <memory>

namespace
{
    QString qmlStructuredEditorsRepositoryRootPath()
    {
        QDir repositoryRoot(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath());
        repositoryRoot.cdUp();
        repositoryRoot.cdUp();
        repositoryRoot.cdUp();
        return repositoryRoot.absolutePath();
    }

    QString qmlStructuredEditorErrorString(const QList<QQmlError>& errors)
    {
        QStringList messages;
        messages.reserve(errors.size());
        for (const QQmlError& error : errors)
        {
            messages.push_back(error.toString());
        }
        return messages.join(QLatin1Char('\n'));
    }

    void addWhatSonStructuredEditorQmlImportPaths(QQmlEngine& engine, const QString& repositoryRoot)
    {
        const QStringList candidatePaths{
            repositoryRoot + QStringLiteral("/src/app/qml"),
            repositoryRoot + QStringLiteral("/build/src/app"),
            repositoryRoot + QStringLiteral("/build/src/app/cmake/runtime/lvrs_runtime_qml"),
            repositoryRoot + QStringLiteral("/build/src/app/lvrs_runtime_qml"),
        };
        for (const QString& candidatePath : candidatePaths)
        {
            if (QFileInfo::exists(candidatePath))
            {
                engine.addImportPath(candidatePath);
            }
        }
    }

    void registerStructuredEditorRuntimeQmlTypes()
    {
        static const int textFormatRendererTypeId = qmlRegisterType<ContentsTextFormatRenderer>(
            "WhatSon.App.Internal",
            1,
            0,
            "ContentsTextFormatRenderer");
        static const int bodyTagInsertionPlannerTypeId = qmlRegisterType<ContentsEditorBodyTagInsertionPlanner>(
            "WhatSon.App.Internal",
            1,
            0,
            "ContentsEditorBodyTagInsertionPlanner");
        static const int structuredDocumentBlocksModelTypeId = qmlRegisterType<ContentsStructuredDocumentBlocksModel>(
            "WhatSon.App.Internal",
            1,
            0,
            "ContentsStructuredDocumentBlocksModel");
        static const int structuredDocumentHostTypeId = qmlRegisterType<ContentsStructuredDocumentHost>(
            "WhatSon.App.Internal",
            1,
            0,
            "ContentsStructuredDocumentHost");
        static const int resourceBitmapViewerTypeId = qmlRegisterType<ResourceBitmapViewer>(
            "WhatSon.App.Internal",
            1,
            0,
            "ResourceBitmapViewer");
        Q_UNUSED(textFormatRendererTypeId);
        Q_UNUSED(bodyTagInsertionPlannerTypeId);
        Q_UNUSED(structuredDocumentBlocksModelTypeId);
        Q_UNUSED(structuredDocumentHostTypeId);
        Q_UNUSED(resourceBitmapViewerTypeId);
    }

    QQuickItem* findStructuredEditorQuickItemByObjectName(QQuickItem* rootItem, const QString& objectName)
    {
        if (!rootItem)
        {
            return nullptr;
        }
        if (rootItem->objectName() == objectName)
        {
            return rootItem;
        }
        const QList<QQuickItem*> children = rootItem->childItems();
        for (QQuickItem* child : children)
        {
            if (QQuickItem* match = findStructuredEditorQuickItemByObjectName(child, objectName))
            {
                return match;
            }
        }
        return nullptr;
    }
} // namespace

void WhatSonCppRegressionTests::qmlStructuredEditors_bindPaperPaletteIntoPagePrintMode()
{
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));
    const QString documentBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentBlock.qml"));
    const QString textBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml"));
    const QString textBlockControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsDocumentTextBlockController.qml"));
    const QString agendaBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsAgendaBlock.qml"));
    const QString calloutBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsCalloutBlock.qml"));
    const QString agendaLayerSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsAgendaLayer.qml"));
    const QString calloutLayerSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsCalloutLayer.qml"));
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));

    QVERIFY(!structuredFlowSource.isEmpty());
    QVERIFY(!documentBlockSource.isEmpty());
    QVERIFY(!textBlockSource.isEmpty());
    QVERIFY(!agendaBlockSource.isEmpty());
    QVERIFY(!calloutBlockSource.isEmpty());
    QVERIFY(!agendaLayerSource.isEmpty());
    QVERIFY(!calloutLayerSource.isEmpty());
    QVERIFY(!displayViewSource.isEmpty());

    QVERIFY(structuredFlowSource.contains(QStringLiteral("property bool paperPaletteEnabled: false")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("paperPaletteEnabled: documentFlow.paperPaletteEnabled")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("property bool paperPaletteEnabled: false")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("paperPaletteEnabled: documentBlock.paperPaletteEnabled")));
    QVERIFY(textBlockSource.contains(
        QStringLiteral("readonly property color editorTextColor: paperPaletteEnabled ? \"#111111\" : LV.Theme.bodyColor")));
    QVERIFY(textBlockSource.contains(QStringLiteral("paperPaletteEnabled: textBlock.paperPaletteEnabled")));
    QVERIFY(textBlockSource.contains(QStringLiteral("textColor: textBlock.editorTextColor")));

    QVERIFY(!agendaBlockSource.contains(QStringLiteral("textColor: \"#FFFFFF\"")));
    QVERIFY(!agendaBlockSource.contains(QStringLiteral("color: \"#80FFFFFF\"")));
    QVERIFY(agendaBlockSource.contains(
        QStringLiteral("readonly property color taskTextColor: paperPaletteEnabled ? \"#111111\" : \"#FFFFFF\"")));
    QVERIFY(calloutBlockSource.contains(
        QStringLiteral("readonly property color bodyTextColor: paperPaletteEnabled ? \"#111111\" : \"#FFFFFF\"")));
    QVERIFY(!calloutBlockSource.contains(QStringLiteral("textColor: \"#FFFFFF\"")));
    QVERIFY(!agendaLayerSource.contains(QStringLiteral("color: \"#FFFFFF\"")));
    QVERIFY(agendaLayerSource.contains(
        QStringLiteral("readonly property color taskTextColor: paperPaletteEnabled ? \"#111111\" : \"#FFFFFF\"")));
    QVERIFY(calloutLayerSource.contains(
        QStringLiteral("readonly property color textColor: paperPaletteEnabled ? \"#111111\" : \"#FFFFFF\"")));

    QVERIFY(displayViewSource.contains(QStringLiteral("paperPaletteEnabled: contentsView.showPrintEditorLayout")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_clipInlineResourceCardsToMeasuredBlockBounds()
{
    const QString resourceBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsResourceBlock.qml"));
    const QString resourceCardSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsResourceRenderCard.qml"));

    QVERIFY(!resourceBlockSource.isEmpty());
    QVERIFY(!resourceCardSource.isEmpty());

    QVERIFY(resourceBlockSource.contains(QStringLiteral("implicitHeight: resourceCard.implicitHeight")));
    QVERIFY(resourceBlockSource.contains(QStringLiteral("clip: true")));
    QVERIFY(resourceBlockSource.contains(QStringLiteral("function shortcutInsertionSourceOffset()")));
    QVERIFY(resourceBlockSource.contains(QStringLiteral("return resourceBlock.sourceEnd")));
    QVERIFY(resourceCardSource.contains(QStringLiteral("clip: true")));
    QVERIFY(!resourceCardSource.contains(QStringLiteral("clip: false")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_renderInlineStyleOverlayAtRuntime()
{
    registerStructuredEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlStructuredEditorsRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonStructuredEditorQmlImportPaths(engine, repositoryRoot);

    QQmlComponent component(
        &engine,
        QUrl::fromLocalFile(
            repositoryRoot
            + QStringLiteral("/src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlStructuredEditorErrorString(component.errors())));
    }

    const QString sourceText =
        QStringLiteral("Plain <bold>strong</bold> and <highlight>glow</highlight>");
    QVariantMap blockData;
    blockData.insert(QStringLiteral("sourceStart"), 0);
    blockData.insert(QStringLiteral("sourceEnd"), sourceText.size());
    blockData.insert(QStringLiteral("sourceText"), sourceText);

    std::unique_ptr<QObject> textBlockObject(component.createWithInitialProperties({
        {QStringLiteral("blockData"), blockData},
    }));
    if (!textBlockObject)
    {
        QFAIL(qPrintable(qmlStructuredEditorErrorString(component.errors())));
    }

    auto* textBlockItem = qobject_cast<QQuickItem*>(textBlockObject.get());
    QVERIFY(textBlockItem != nullptr);
    textBlockItem->setWidth(420);
    textBlockItem->setHeight(80);

    QQuickWindow window;
    window.resize(420, 80);
    textBlockItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QTRY_VERIFY(textBlockObject->property("sourceContainsInlineStyleTags").toBool());
    QTRY_VERIFY(textBlockObject->property("inlineStyleOverlayVisible").toBool());

    QObject* inlineEditor = textBlockObject->findChild<QObject*>(QStringLiteral("contentsInlineFormatEditor"));
    QVERIFY(inlineEditor != nullptr);

    QTRY_VERIFY(inlineEditor->property("showRenderedOutput").toBool());
    const QString renderedText = inlineEditor->property("renderedText").toString();
    QVERIFY(renderedText.contains(QStringLiteral("<strong style=\"font-weight:900;\">strong</strong>")));
    QVERIFY(renderedText.contains(QStringLiteral("background-color:#8A4B00")));
    QCOMPARE(inlineEditor->property("text").toString(), QStringLiteral("Plain strong and glow"));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_commitsPlainTextBlocksDirectlyToRawSource()
{
    registerStructuredEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlStructuredEditorsRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonStructuredEditorQmlImportPaths(engine, repositoryRoot);

    QQmlComponent component(
        &engine,
        QUrl::fromLocalFile(
            repositoryRoot
            + QStringLiteral("/src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlStructuredEditorErrorString(component.errors())));
    }

    const QString sourceText = QStringLiteral("Alpha beta");
    const int sourceStart = 7;
    QVariantMap blockData;
    blockData.insert(QStringLiteral("sourceStart"), sourceStart);
    blockData.insert(QStringLiteral("sourceEnd"), sourceStart + sourceText.size());
    blockData.insert(QStringLiteral("sourceText"), sourceText);

    std::unique_ptr<QObject> textBlockObject(component.createWithInitialProperties({
        {QStringLiteral("blockData"), blockData},
    }));
    if (!textBlockObject)
    {
        QFAIL(qPrintable(qmlStructuredEditorErrorString(component.errors())));
    }

    auto* textBlockItem = qobject_cast<QQuickItem*>(textBlockObject.get());
    QVERIFY(textBlockItem != nullptr);
    textBlockItem->setWidth(420);
    textBlockItem->setHeight(80);

    QQuickWindow window;
    window.resize(420, 80);
    textBlockItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QObject* inlineEditor = textBlockObject->findChild<QObject*>(QStringLiteral("contentsInlineFormatEditor"));
    QVERIFY(inlineEditor != nullptr);
    QTRY_COMPARE(inlineEditor->property("text").toString(), sourceText);
    QVERIFY(!textBlockObject->property("sourceContainsInlineStyleTags").toBool());
    QVERIFY(!inlineEditor->property("showRenderedOutput").toBool());

    QSignalSpy mutationSpy(
        textBlockObject.get(),
        SIGNAL(sourceMutationRequested(QString,QVariant,QString)));
    QVERIFY2(mutationSpy.isValid(), "sourceMutationRequested signal must remain observable from C++ tests");

    QObject* editorItem = qvariant_cast<QObject*>(inlineEditor->property("editorItem"));
    QVERIFY(editorItem != nullptr);
    QObject* textInput = qvariant_cast<QObject*>(editorItem->property("inputItem"));
    QVERIFY(textInput != nullptr);

    textInput->setProperty("cursorPosition", sourceText.size());
    QVERIFY(QMetaObject::invokeMethod(
        textInput,
        "insert",
        Q_ARG(int, sourceText.size()),
        Q_ARG(QString, QStringLiteral(" gamma"))));

    const QString nextSourceText = QStringLiteral("Alpha beta gamma");
    QTRY_VERIFY(mutationSpy.count() >= 1);
    const QList<QVariant> mutationArguments = mutationSpy.takeFirst();
    QCOMPARE(mutationArguments.at(0).toString(), nextSourceText);
    QCOMPARE(mutationArguments.at(2).toString(), sourceText);

    const QVariantMap focusRequest = mutationArguments.at(1).toMap();
    QCOMPARE(focusRequest.value(QStringLiteral("reason")).toString(), QStringLiteral("text-edit"));
    const int sourceOffset = focusRequest.value(QStringLiteral("sourceOffset")).toInt();
    QVERIFY(sourceOffset >= sourceStart);
    QVERIFY(sourceOffset <= sourceStart + nextSourceText.size());
}

void WhatSonCppRegressionTests::qmlStructuredEditors_insertsInlineFormatTagsAtCollapsedCursor()
{
    registerStructuredEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlStructuredEditorsRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonStructuredEditorQmlImportPaths(engine, repositoryRoot);

    QQmlComponent component(
        &engine,
        QUrl::fromLocalFile(
            repositoryRoot
            + QStringLiteral("/src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlStructuredEditorErrorString(component.errors())));
    }

    const QString sourceText = QStringLiteral("Alpha beta");
    QVariantMap blockData;
    blockData.insert(QStringLiteral("atomicBlock"), false);
    blockData.insert(QStringLiteral("plainText"), sourceText);
    blockData.insert(QStringLiteral("sourceStart"), 0);
    blockData.insert(QStringLiteral("sourceEnd"), sourceText.size());
    blockData.insert(QStringLiteral("sourceText"), sourceText);
    blockData.insert(QStringLiteral("textEditable"), true);
    blockData.insert(QStringLiteral("type"), QStringLiteral("text-group"));
    QVariantList documentBlocks;
    documentBlocks.push_back(blockData);

    std::unique_ptr<QObject> flowObject(component.createWithInitialProperties({
        {QStringLiteral("documentBlocks"), documentBlocks},
        {QStringLiteral("sourceText"), sourceText},
    }));
    if (!flowObject)
    {
        QFAIL(qPrintable(qmlStructuredEditorErrorString(component.errors())));
    }

    QSignalSpy mutationSpy(
        flowObject.get(),
        SIGNAL(sourceMutationRequested(QString,QVariant)));
    QVERIFY2(mutationSpy.isValid(), "sourceMutationRequested signal must remain observable from C++ tests");

    QVariantMap collapsedSelectionSnapshot;
    collapsedSelectionSnapshot.insert(QStringLiteral("cursorPosition"), 5);
    collapsedSelectionSnapshot.insert(QStringLiteral("selectedText"), QString());
    collapsedSelectionSnapshot.insert(QStringLiteral("selectionStart"), 5);
    collapsedSelectionSnapshot.insert(QStringLiteral("selectionEnd"), 5);

    QVariant returnValue;
    QVERIFY(QMetaObject::invokeMethod(
        flowObject.get(),
        "applyInlineFormatToBlockSelection",
        Q_RETURN_ARG(QVariant, returnValue),
        Q_ARG(QVariant, QVariant(0)),
        Q_ARG(QVariant, QVariant(QStringLiteral("bold"))),
        Q_ARG(QVariant, QVariant(collapsedSelectionSnapshot))));
    QVERIFY(returnValue.toBool());

    QTRY_VERIFY(mutationSpy.count() >= 1);
    const QList<QVariant> mutationArguments = mutationSpy.takeFirst();
    QCOMPARE(mutationArguments.at(0).toString(), QStringLiteral("Alpha<bold></bold> beta"));

    const QVariantMap focusRequest = mutationArguments.at(1).toMap();
    const int expectedFocusOffset = QStringLiteral("Alpha<bold>").size();
    QCOMPARE(focusRequest.value(QStringLiteral("sourceOffset")).toInt(), expectedFocusOffset);
    QCOMPARE(focusRequest.value(QStringLiteral("targetBlockIndex")).toInt(), 0);
}

void WhatSonCppRegressionTests::qmlStructuredEditors_acceptsPlatformCommandModifierForInlineFormatting()
{
    registerStructuredEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlStructuredEditorsRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonStructuredEditorQmlImportPaths(engine, repositoryRoot);

    QQmlComponent component(
        &engine,
        QUrl::fromLocalFile(
            repositoryRoot
            + QStringLiteral("/src/app/qml/view/content/editor/ContentsInlineFormatEditor.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlStructuredEditorErrorString(component.errors())));
    }

    std::unique_ptr<QObject> inlineEditorObject(component.create());
    if (!inlineEditorObject)
    {
        QFAIL(qPrintable(qmlStructuredEditorErrorString(component.errors())));
    }

    QVariantMap nativeCommandEvent;
    nativeCommandEvent.insert(QStringLiteral("key"), static_cast<int>(Qt::Key_B));
    nativeCommandEvent.insert(
        QStringLiteral("modifiers"),
        static_cast<int>(Qt::ControlModifier | Qt::MetaModifier));
    nativeCommandEvent.insert(QStringLiteral("text"), QStringLiteral("b"));

    QVariant acceptedReturnValue;
    QVERIFY(QMetaObject::invokeMethod(
        inlineEditorObject.get(),
        "eventRequestsInlineFormatShortcut",
        Q_RETURN_ARG(QVariant, acceptedReturnValue),
        Q_ARG(QVariant, QVariant(nativeCommandEvent))));
    QVERIFY(acceptedReturnValue.toBool());

    QVariant tagReturnValue;
    QVERIFY(QMetaObject::invokeMethod(
        inlineEditorObject.get(),
        "inlineFormatShortcutTag",
        Q_RETURN_ARG(QVariant, tagReturnValue),
        Q_ARG(QVariant, QVariant(nativeCommandEvent))));
    QCOMPARE(tagReturnValue.toString(), QStringLiteral("bold"));

    QVariantMap optionCommandEvent = nativeCommandEvent;
    optionCommandEvent.insert(
        QStringLiteral("modifiers"),
        static_cast<int>(Qt::ControlModifier | Qt::AltModifier));

    QVariant rejectedReturnValue;
    QVERIFY(QMetaObject::invokeMethod(
        inlineEditorObject.get(),
        "eventRequestsInlineFormatShortcut",
        Q_RETURN_ARG(QVariant, rejectedReturnValue),
        Q_ARG(QVariant, QVariant(optionCommandEvent))));
    QVERIFY(!rejectedReturnValue.toBool());

    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsInlineFormatEditor.qml"));
    QVERIFY(!displayViewSource.contains(QStringLiteral("metaPressed && controlPressed")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("metaPressed && controlPressed")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_routesInlineFormatShortcutThroughDocumentFlow()
{
    registerStructuredEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlStructuredEditorsRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonStructuredEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportPath = repositoryRoot + QStringLiteral("/src/app/qml/view/content/editor");
    const QString editorImportUrl = QUrl::fromLocalFile(editorImportPath).toString();
    const QString qmlSource = QStringLiteral(R"QML(
import QtQuick
import "%1" as EditorView

EditorView.ContentsStructuredDocumentFlow {
    id: flow
    objectName: "flowUnderTest"
    width: 420
    height: 120
    sourceText: "Alpha beta"
    documentBlocks: [
        {
            "atomicBlock": false,
            "plainText": "Alpha beta",
            "sourceStart": 0,
            "sourceEnd": 10,
            "sourceText": "Alpha beta",
            "textEditable": true,
            "type": "text-group"
        }
    ]
    tagManagementShortcutKeyPressHandler: function (event) {
        return false;
    }
}
)QML").arg(editorImportUrl);

    QQmlComponent component(&engine);
    component.setData(qmlSource.toUtf8(), QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/flow-inline-format.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlStructuredEditorErrorString(component.errors())));
    }

    std::unique_ptr<QObject> flowObject(component.create());
    if (!flowObject)
    {
        QFAIL(qPrintable(qmlStructuredEditorErrorString(component.errors())));
    }

    auto* flowItem = qobject_cast<QQuickItem*>(flowObject.get());
    QVERIFY(flowItem != nullptr);
    flowItem->setWidth(420);
    flowItem->setHeight(120);

    QQuickWindow window;
    window.resize(420, 120);
    flowItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QQuickItem* inlineEditorItem = nullptr;
    QTRY_VERIFY((inlineEditorItem = findStructuredEditorQuickItemByObjectName(
                     flowItem,
                     QStringLiteral("contentsInlineFormatEditor"))) != nullptr);
    QObject* inlineEditor = inlineEditorItem;
    QObject* editorItem = qvariant_cast<QObject*>(inlineEditor->property("editorItem"));
    QVERIFY(editorItem != nullptr);
    QObject* textInput = qvariant_cast<QObject*>(editorItem->property("inputItem"));
    QVERIFY(textInput != nullptr);
    auto* textInputItem = qobject_cast<QQuickItem*>(textInput);
    QVERIFY(textInputItem != nullptr);

    QSignalSpy mutationSpy(
        flowObject.get(),
        SIGNAL(sourceMutationRequested(QString,QVariant)));
    QVERIFY2(mutationSpy.isValid(), "sourceMutationRequested signal must remain observable from C++ tests");

    QVERIFY(QMetaObject::invokeMethod(inlineEditor, "forceActiveFocus"));
    QTRY_VERIFY(textInputItem->hasActiveFocus());
    textInput->setProperty("cursorPosition", 5);
    QTest::keyClick(&window, Qt::Key_B, Qt::ControlModifier);

    QTRY_VERIFY(mutationSpy.count() >= 1);
    const QList<QVariant> mutationArguments = mutationSpy.takeFirst();
    QCOMPARE(mutationArguments.at(0).toString(), QStringLiteral("Alpha<bold></bold> beta"));

    const QString textBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml"));
    const QString documentBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentBlock.qml"));
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));
    QVERIFY(textBlockSource.contains(QStringLiteral("signal inlineFormatRequested(string tagName, var selectionSnapshot)")));
    QVERIFY(textBlockSource.contains(QStringLiteral("blockEditor.inlineFormatShortcutTag(event)")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("signal inlineFormatRequested(int blockIndex, string tagName, var selectionSnapshot)")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("onInlineFormatRequested: function (blockIndex, tagName, selectionSnapshot)")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_focusesDocumentEndFromBottomWhitespace()
{
    registerStructuredEditorRuntimeQmlTypes();

    const QString surfaceHostSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplaySurfaceHost.qml"));
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));
    const QString commandSurfaceSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayInputCommandSurface.qml"));
    QVERIFY(!surfaceHostSource.isEmpty());
    QVERIFY(!structuredFlowSource.isEmpty());
    QVERIFY(!commandSurfaceSource.isEmpty());
    QVERIFY(surfaceHostSource.contains(QStringLiteral("enabled: contentsView.noteDocumentParseMounted")));
    QVERIFY(!surfaceHostSource.contains(QStringLiteral("enabled: contentsView.noteDocumentSurfaceInteractive")));
    QVERIFY(surfaceHostSource.contains(QStringLiteral("pointTargetsDocumentEndEdit(flowTapX, flowTapY)")));
    QVERIFY(surfaceHostSource.contains(QStringLiteral("property bool documentEndEditRequestQueued: false")));
    QVERIFY(surfaceHostSource.contains(QStringLiteral("if (surfaceHost.documentEndEditRequestQueued)")));
    QVERIFY(surfaceHostSource.contains(QStringLiteral("function requestStructuredDocumentEndEditFromSurfacePoint(")));
    QVERIFY(surfaceHostSource.contains(QStringLiteral("structuredDocumentEndWhitespaceTapHandler")));
    QVERIFY(surfaceHostSource.contains(QStringLiteral("contentsView: surfaceHost.contentsView")));
    QVERIFY(surfaceHostSource.contains(QStringLiteral("resourceImportController: surfaceHost.resourceImportController")));
    QVERIFY(!surfaceHostSource.contains(QStringLiteral("contentsView: contentsView")));
    QVERIFY(!surfaceHostSource.contains(QStringLiteral("resourceImportController: resourceImportController")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function pointTargetsDocumentEndEdit(localX, localY)")));
    QVERIFY(structuredFlowSource.indexOf(QStringLiteral("const lastBlockHost = blockRepeater.itemAt(lastBlockIndex)"))
            < structuredFlowSource.indexOf(QStringLiteral("const cachedSummary = documentFlow.cachedBlockLayoutSummaryAt(lastBlockIndex)")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("\"targetBlockIndex\": lastBlockIndex")));
    QVERIFY(commandSurfaceSource.contains(QStringLiteral("target: commandSurface")));
    QVERIFY(commandSurfaceSource.contains(QStringLiteral("acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad")));

    const QString repositoryRoot = qmlStructuredEditorsRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonStructuredEditorQmlImportPaths(engine, repositoryRoot);

    QQmlComponent component(
        &engine,
        QUrl::fromLocalFile(
            repositoryRoot
            + QStringLiteral("/src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlStructuredEditorErrorString(component.errors())));
    }

    const QString resourceSourceText = QStringLiteral("<resource id=\"image\" />");
    QVariantMap resourceBlock;
    resourceBlock.insert(QStringLiteral("atomicBlock"), true);
    resourceBlock.insert(QStringLiteral("gutterCollapsed"), true);
    resourceBlock.insert(QStringLiteral("sourceStart"), 0);
    resourceBlock.insert(QStringLiteral("sourceEnd"), resourceSourceText.size());
    resourceBlock.insert(QStringLiteral("sourceText"), resourceSourceText);
    resourceBlock.insert(QStringLiteral("textEditable"), false);
    resourceBlock.insert(QStringLiteral("type"), QStringLiteral("resource"));

    QVariantMap trailingTextBlock;
    trailingTextBlock.insert(QStringLiteral("atomicBlock"), false);
    trailingTextBlock.insert(QStringLiteral("plainText"), QString());
    trailingTextBlock.insert(QStringLiteral("sourceStart"), resourceSourceText.size());
    trailingTextBlock.insert(QStringLiteral("sourceEnd"), resourceSourceText.size());
    trailingTextBlock.insert(QStringLiteral("sourceText"), QString());
    trailingTextBlock.insert(QStringLiteral("textEditable"), true);
    trailingTextBlock.insert(QStringLiteral("type"), QStringLiteral("text-group"));

    QVariantList documentBlocks;
    documentBlocks.push_back(resourceBlock);
    documentBlocks.push_back(trailingTextBlock);

    std::unique_ptr<QObject> flowObject(component.createWithInitialProperties({
        {QStringLiteral("documentBlocks"), documentBlocks},
        {QStringLiteral("sourceText"), resourceSourceText},
    }));
    if (!flowObject)
    {
        QFAIL(qPrintable(qmlStructuredEditorErrorString(component.errors())));
    }

    QVariant returnValue;
    QVERIFY(QMetaObject::invokeMethod(
        flowObject.get(),
        "requestDocumentEndEdit",
        Q_RETURN_ARG(QVariant, returnValue)));
    QVERIFY(returnValue.toBool());

    const QVariantMap focusRequest = flowObject->property("pendingFocusRequest").toMap();
    QCOMPARE(focusRequest.value(QStringLiteral("reason")).toString(), QStringLiteral("document-end-edit"));
    QCOMPARE(focusRequest.value(QStringLiteral("sourceOffset")).toInt(), resourceSourceText.size());
    QCOMPARE(focusRequest.value(QStringLiteral("targetBlockIndex")).toInt(), 1);
    QCOMPARE(flowObject->property("pendingFocusBlockIndex").toInt(), 1);

    const QString editorImportPath = repositoryRoot + QStringLiteral("/src/app/qml/view/content/editor");
    const QString editorImportUrl = QUrl::fromLocalFile(editorImportPath).toString();
    const QString surfaceHostQmlSource = QStringLiteral(R"QML(
import QtQuick
import "%1" as EditorView

Item {
    id: root
    width: 420
    height: 360

    QtObject {
        id: bodyResourceRenderer
        property var renderedResources: []
    }
    QtObject {
        id: blockRenderer
        property var renderedAgendas: []
        property var renderedCallouts: []
        property var renderedDocumentBlocks: [
            {
                "atomicBlock": false,
                "plainText": "Alpha",
                "sourceStart": 0,
                "sourceEnd": 5,
                "sourceText": "Alpha",
                "textEditable": true,
                "type": "text-group"
            }
        ]
    }
    QtObject {
        id: editorProjection
        property string renderedHtml: ""
    }
    QtObject {
        id: editorTypingController
        function logicalOffsetForSourceOffset(sourceOffset) { return sourceOffset; }
    }
    QtObject {
        id: resourceImportController
        function canAcceptResourceDropUrls(_urls) { return false; }
        function extractResourceDropUrls(_drop) { return []; }
        function importUrlsAsResourcesWithPrompt(_urls) { return false; }
        function releaseResourceDropEditorSurfaceGuard(_accepted) {}
    }
    QtObject { id: agendaBackend }
    QtObject { id: calloutBackend }

    Item {
        id: contentsView
        objectName: "surfaceHostContentsView"
        property int endEditRequestCount: 0
        property int minEditorHeight: 0
        property bool noteDocumentSurfaceInteractive: true
        property bool noteDocumentParseMounted: true
        property bool showPrintEditorLayout: false
        property bool showStructuredDocumentFlow: true
        property bool showDedicatedResourceViewer: false
        property bool showFormattedTextRenderer: false
        property bool showPrintMarginGuides: false
        property bool nativeTextInputPriority: false
        property bool resourceDropActive: false
        property bool noteDocumentContextMenuSurfaceEnabled: false
        property bool contextMenuLongPressEnabled: false
        property bool noteDocumentTagManagementShortcutSurfaceEnabled: false
        property var editorSelectionContextMenuItems: []
        property var resourcesImportViewModel: null
        property string structuredFlowSourceText: "Alpha"
        property int editorBottomInset: 240
        property int editorDocumentStartY: 16
        property int editorFontWeight: Font.Medium
        property int editorHorizontalInset: 24
        property int editorLineHeight: 18
        property int effectiveEditorFontPixelSize: 18
        property color printCanvasColor: "transparent"
        property int printDocumentPageCount: 0
        property int printDocumentSurfaceHeight: 0
        property int printGuideHorizontalInset: 0
        property int printGuideVerticalInset: 0
        property color printPaperBorderColor: "transparent"
        property color printPaperColor: "transparent"
        property int printPaperDocumentHeight: 0
        property color printPaperHighlightColor: "transparent"
        property int printPaperResolvedHeight: 0
        property int printPaperResolvedWidth: 0
        property color printPaperSeparatorColor: "transparent"
        property int printPaperSeparatorThickness: 0
        property color printPaperShadeColor: "transparent"
        property color printPaperShadowColor: "transparent"
        property int printPaperShadowOffsetX: 0
        property int printPaperShadowOffsetY: 0
        property color printPaperTextColor: "transparent"
        property int printPaperTextHeight: 0
        property int printPaperTextWidth: 0
        property int printPaperVerticalMargin: 0
        function applyDocumentSourceMutation(_nextSourceText, _focusRequest) {}
        function documentYForOffset(_logicalOffset) { return 0; }
        function focusStructuredBlockSourceOffset(_sourceOffset) {}
        function handleSelectionContextMenuEvent(_eventName) {}
        function handleTagManagementShortcutKeyPress(_event) { return false; }
        function primeEditorSelectionContextMenuSnapshot() {}
        function queueAgendaShortcutInsertion() { return false; }
        function queueBreakShortcutInsertion() { return false; }
        function queueCalloutShortcutInsertion() { return false; }
        function queueInlineFormatWrap(_tagName) { return false; }
        function requestEditorSelectionContextMenuFromPointer(_x, _y, _reason) {}
        function requestStructuredDocumentEndEdit() {
            endEditRequestCount += 1;
            return true;
        }
        function setAgendaTaskDone(_taskOpenTagStart, _taskOpenTagEnd, _checked) {}
    }

    EditorView.ContentsDisplaySurfaceHost {
        id: surfaceHost
        objectName: "surfaceHostUnderTest"
        anchors.fill: parent
        bodyResourceRenderer: bodyResourceRenderer
        contentsAgendaBackend: agendaBackend
        contentsCalloutBackend: calloutBackend
        contentsView: contentsView
        editorProjection: editorProjection
        editorTypingController: editorTypingController
        resourceImportController: resourceImportController
        structuredBlockRenderer: blockRenderer
    }
}
)QML").arg(editorImportUrl);

    QQmlComponent surfaceHostComponent(&engine);
    surfaceHostComponent.setData(
        surfaceHostQmlSource.toUtf8(),
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/surface-host-end-edit.qml")));
    if (surfaceHostComponent.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlStructuredEditorErrorString(surfaceHostComponent.errors())));
    }

    std::unique_ptr<QObject> rootObject(surfaceHostComponent.create());
    if (!rootObject)
    {
        QFAIL(qPrintable(qmlStructuredEditorErrorString(surfaceHostComponent.errors())));
    }
    auto* rootItem = qobject_cast<QQuickItem*>(rootObject.get());
    QVERIFY(rootItem != nullptr);

    QQuickWindow hostWindow;
    hostWindow.resize(420, 360);
    rootItem->setParentItem(hostWindow.contentItem());
    hostWindow.show();
    QVERIFY(QTest::qWaitForWindowExposed(&hostWindow));

    QObject* hostContentsView = rootObject->findChild<QObject*>(QStringLiteral("surfaceHostContentsView"));
    QVERIFY(hostContentsView != nullptr);
    QObject* surfaceHost = rootObject->findChild<QObject*>(QStringLiteral("surfaceHostUnderTest"));
    QVERIFY(surfaceHost != nullptr);

    QVariant hostReturnValue;
    QVERIFY(QMetaObject::invokeMethod(
        surfaceHost,
        "requestStructuredDocumentEndEditFromSurfacePoint",
        Q_RETURN_ARG(QVariant, hostReturnValue),
        Q_ARG(QVariant, QVariant(220)),
        Q_ARG(QVariant, QVariant(320))));
    QVERIFY(hostReturnValue.toBool());
    QTRY_COMPARE(hostContentsView->property("endEditRequestCount").toInt(), 1);
}

void WhatSonCppRegressionTests::qmlStructuredEditors_deletesEmptyCalloutWithBackspace()
{
    const QString repositoryRoot = qmlStructuredEditorsRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonStructuredEditorQmlImportPaths(engine, repositoryRoot);

    QQmlComponent component(
        &engine,
        QUrl::fromLocalFile(
            repositoryRoot
            + QStringLiteral("/src/app/qml/view/content/editor/ContentsCalloutBlock.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlStructuredEditorErrorString(component.errors())));
    }

    const QString sourceText = QStringLiteral("<callout> </callout>");
    QVariantMap blockData;
    blockData.insert(QStringLiteral("type"), QStringLiteral("callout"));
    blockData.insert(QStringLiteral("sourceStart"), 0);
    blockData.insert(QStringLiteral("sourceEnd"), sourceText.size());
    blockData.insert(QStringLiteral("contentStart"), QStringLiteral("<callout>").size());
    blockData.insert(QStringLiteral("contentEnd"), QStringLiteral("<callout> ").size());
    blockData.insert(QStringLiteral("text"), QString());

    std::unique_ptr<QObject> calloutBlockObject(component.createWithInitialProperties({
        {QStringLiteral("blockData"), blockData},
    }));
    if (!calloutBlockObject)
    {
        QFAIL(qPrintable(qmlStructuredEditorErrorString(component.errors())));
    }

    auto* calloutBlockItem = qobject_cast<QQuickItem*>(calloutBlockObject.get());
    QVERIFY(calloutBlockItem != nullptr);
    calloutBlockItem->setWidth(420);
    calloutBlockItem->setHeight(80);

    QQuickWindow window;
    window.resize(420, 80);
    calloutBlockItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QObject* inlineEditor = calloutBlockObject->findChild<QObject*>(QStringLiteral("contentsInlineFormatEditor"));
    QVERIFY(inlineEditor != nullptr);
    QTRY_COMPARE(inlineEditor->property("text").toString(), QString());

    QObject* editorItem = qvariant_cast<QObject*>(inlineEditor->property("editorItem"));
    QVERIFY(editorItem != nullptr);
    QObject* textInput = qvariant_cast<QObject*>(editorItem->property("inputItem"));
    QVERIFY(textInput != nullptr);
    auto* textInputItem = qobject_cast<QQuickItem*>(textInput);
    QVERIFY(textInputItem != nullptr);

    QSignalSpy deletionSpy(
        calloutBlockObject.get(),
        SIGNAL(blockDeletionRequested(QString)));
    QVERIFY2(deletionSpy.isValid(), "blockDeletionRequested signal must remain observable from C++ tests");

    QVERIFY(QMetaObject::invokeMethod(inlineEditor, "forceActiveFocus"));
    QTRY_VERIFY(textInputItem->hasActiveFocus());
    textInput->setProperty("cursorPosition", 0);
    QTest::keyClick(&window, Qt::Key_Backspace);

    QTRY_COMPARE(deletionSpy.count(), 1);
    const QList<QVariant> deletionArguments = deletionSpy.takeFirst();
    QCOMPARE(deletionArguments.at(0).toString(), QStringLiteral("backward"));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_consumeRendererNormalizedBlocksWithoutLocalFlattening()
{
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));

    QVERIFY(!structuredFlowSource.isEmpty());
    QVERIFY(structuredFlowSource.contains(
        QStringLiteral("documentHost.documentBlocks = documentHost.collectionPolicy.normalizeEntries(documentFlow.documentBlocks)")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("function flattenedInteractiveBlocks()")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("function normalizedParsedBlocks()")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("function buildFlattenedInteractiveTextGroup(")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("function implicitTextBlockInteractiveFlattenCandidate(")));
    QVERIFY(structuredFlowSource.contains(
        QStringLiteral("function snapshotTokenForLogicalLine(blockEntry, logicalLines, lineIndex)")));
    QVERIFY(structuredFlowSource.contains(
        QStringLiteral("\"snapshotToken\": documentFlow.snapshotTokenForLogicalLine(blockEntry, logicalLines, index)")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_refreshesGutterLayoutOnEditorOpen()
{
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString eventPumpSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayEventPump.qml"));

    QVERIFY(!structuredFlowSource.isEmpty());
    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!eventPumpSource.isEmpty());

    QVERIFY(structuredFlowSource.contains(
        QStringLiteral("property int editorOpenLayoutRefreshPassesRemaining: 0")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("readonly property int editorOpenLayoutRefreshPassCount: 3")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function scheduleEditorOpenLayoutCacheRefresh(reason)")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function scheduleEditorOpenLayoutCacheRefreshPass(revision)")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral(
        "documentFlow.editorOpenLayoutRefreshPassesRemaining = Math.max(")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral(
        "documentFlow.scheduleEditorOpenLayoutCacheRefreshPass(")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral(
        "documentFlow.scheduleEditorOpenLayoutCacheRefresh(\"completed\")")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral(
        "documentFlow.scheduleEditorOpenLayoutCacheRefresh(\"visible\")")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("onLoaded: {")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("documentFlow.scheduleLayoutCacheRefresh()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function scheduleStructuredDocumentOpenLayoutRefresh(reason)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("structuredDocumentFlow.scheduleEditorOpenLayoutCacheRefresh(")));
    QVERIFY(displayViewSource.contains(QStringLiteral(
        "contentsView.scheduleStructuredDocumentOpenLayoutRefresh(\"note-mounted\")")));
    QVERIFY(eventPumpSource.contains(QStringLiteral(
        "eventPump.contentsView.scheduleStructuredDocumentOpenLayoutRefresh(\"rendered-blocks\")")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_rejectStaleSourceRangeMutations()
{
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));

    QVERIFY(!structuredFlowSource.isEmpty());
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function sourceRangeMatchesCurrentSnapshot(")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function replaceSourceRangeIfCurrent(")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("sourceRangeMatchesCurrentSnapshot.rejected")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("safeBlock.sourceText")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("\"reason\": \"text-block\"")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("const expectedTaskSourceText = StructuredCursorSupport.replacementSourceText(")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("\"reason\": \"agenda-task\"")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("const expectedCalloutSourceText = StructuredCursorSupport.replacementSourceText(")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("\"reason\": \"callout-text\"")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("\"allowBlankAnchor\": true")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_preserveNativeMobileInputDuringFocusedEdits()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString typingControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsEditorTypingController.qml"));
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsInlineFormatEditor.qml"));
    const QString inlineEditorControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsInlineFormatEditorController.qml"));
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));
    const QString documentBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentBlock.qml"));
    const QString documentBlockControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsDocumentBlockController.qml"));
    const QString textBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml"));
    const QString textBlockControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsDocumentTextBlockController.qml"));
    const QString agendaBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsAgendaBlock.qml"));
    const QString agendaTaskRowControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsAgendaTaskRowController.qml"));
    const QString calloutBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsCalloutBlock.qml"));
    const QString calloutBlockControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsCalloutBlockController.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!typingControllerSource.isEmpty());
    QVERIFY(!inlineEditorSource.isEmpty());
    QVERIFY(!inlineEditorControllerSource.isEmpty());
    QVERIFY(!structuredFlowSource.isEmpty());
    QVERIFY(!documentBlockSource.isEmpty());
    QVERIFY(!documentBlockControllerSource.isEmpty());
    QVERIFY(!textBlockSource.isEmpty());
    QVERIFY(!textBlockControllerSource.isEmpty());
    QVERIFY(!agendaBlockSource.isEmpty());
    QVERIFY(!agendaTaskRowControllerSource.isEmpty());
    QVERIFY(!calloutBlockSource.isEmpty());
    QVERIFY(!calloutBlockControllerSource.isEmpty());

    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property bool nativeTextInputPriority: surfacePolicy.nativeInputPriority")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsDisplaySurfacePolicy")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("contentsView.mobileHost || Qt.platform.os === \"ios\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property bool noteDocumentShortcutSurfaceEnabled")));
    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property bool noteDocumentTagManagementShortcutSurfaceEnabled")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function nativeTextInputSessionOwnsKeyboard()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("structuredDocumentFlow.nativeCompositionActive()")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("context: Qt.WindowShortcut\n                        enabled: contentsView.noteDocumentCommandSurfaceEnabled")));

    QVERIFY(inlineEditorSource.contains(QStringLiteral("EditorInputModel.ContentsInlineFormatEditorController")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property var tagManagementKeyPressHandler: null")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("function shouldRejectFocusedProgrammaticTextSync(nextText)")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("property bool localTextEditSinceFocus: false")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("property var shortcutKeyPressHandler")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("property var modifierVerticalNavigationHandler")));

    QVERIFY(structuredFlowSource.contains(QStringLiteral("property bool nativeTextInputPriority: false")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("if (documentFlow.nativeTextInputPriority)\n            return")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function nativeCompositionActive()")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("ContentsEditorBodyTagInsertionPlanner")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("bodyTagInsertionPlanner.buildStructuredShortcutInsertionPayload(")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("bodyTagInsertionPlanner.buildCalloutRangeWrappingPayload(")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function activeCalloutWrapSourceRange()")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function exitCallout(blockData, sourceOffset)")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("function structuredShortcutSpec(")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function noteActiveBlockCursorInteraction(blockIndex)")));
    QVERIFY(structuredFlowSource.contains(
        QStringLiteral("onCursorInteraction: documentFlow.noteActiveBlockCursorInteraction(blockHost.blockIndex)")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("expectedPreviousSourceText")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("sourceStart + expectedSourceText.length")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("expectedPreviousText")));

    QVERIFY(documentBlockSource.contains(QStringLiteral("property bool nativeTextInputPriority: false")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("readonly property bool inputMethodComposing")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("readonly property string preeditText")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("nativeTextInputPriority: documentBlock.nativeTextInputPriority")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("signal cursorInteraction()")));
    QVERIFY(documentBlockControllerSource.contains(QStringLiteral("function onCursorInteraction()")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("EditorInputModel.ContentsDocumentBlockController")));
    QVERIFY(documentBlockControllerSource.contains(QStringLiteral("function handleAtomicTagManagementKeyPress(event)")));

    QVERIFY(textBlockSource.contains(QStringLiteral("property bool nativeTextInputPriority: false")));
    QVERIFY(textBlockSource.contains(QStringLiteral("EditorInputModel.ContentsDocumentTextBlockController")));
    QVERIFY(textBlockControllerSource.contains(QStringLiteral("property string liveEditSourceText: \"\"")));
    QVERIFY(textBlockControllerSource.contains(QStringLiteral("syncLiveEditSnapshotFromHost")));
    QVERIFY(textBlockControllerSource.contains(QStringLiteral("readonly property bool sourceContainsInlineStyleTags")));
    QVERIFY(textBlockControllerSource.contains(QStringLiteral("function commitPlainTextRawMutation(")));
    QVERIFY(textBlockControllerSource.contains(QStringLiteral("function currentPlainTextCursorSourceOffset(")));
    QVERIFY(textBlockControllerSource.contains(
        QStringLiteral("if (!controller.sourceContainsInlineStyleTags) {\n            controller.commitPlainTextRawMutation(nextPlainText, previousSourceText);\n            return;\n        }")));
    QVERIFY(textBlockControllerSource.contains(
        QStringLiteral("\"sourceOffset\": controller.currentPlainTextCursorSourceOffset(nextSourceText)")));
    const qsizetype plainRawMutationIndex = textBlockControllerSource.indexOf(
        QStringLiteral("function commitPlainTextRawMutation("));
    const qsizetype styledRawMutationIndex = textBlockControllerSource.indexOf(
        QStringLiteral("applyPlainTextReplacementToSource"));
    QVERIFY(plainRawMutationIndex >= 0);
    QVERIFY(styledRawMutationIndex > plainRawMutationIndex);
    QVERIFY(textBlockSource.contains(
        QStringLiteral("sourceText: textBlock.sourceContainsInlineStyleTags ? textBlock.authoritativeSourceText() : \"\"")));
    QVERIFY(!textBlockSource.contains(
        QStringLiteral("sourceText: textBlock.inlineStyleOverlayVisible ? textBlock.authoritativeSourceText() : \"\"")));
    QVERIFY(textBlockSource.contains(QStringLiteral("preferNativeInputHandling: true")));
    QVERIFY(textBlockSource.contains(QStringLiteral("inputMethodHints: Qt.ImhNone")));
    QVERIFY(textBlockSource.contains(QStringLiteral("mouseSelectionMode: TextEdit.SelectCharacters")));
    QVERIFY(textBlockSource.contains(QStringLiteral("overwriteMode: false")));
    QVERIFY(textBlockSource.contains(QStringLiteral("persistentSelection: true")));
    QVERIFY(textBlockSource.contains(QStringLiteral("selectByKeyboard: true")));
    QVERIFY(textBlockSource.contains(QStringLiteral("signal cursorInteraction()")));
    QVERIFY(textBlockControllerSource.contains(QStringLiteral("controller.textBlock.cursorInteraction();")));
    QVERIFY(!textBlockSource.contains(QStringLiteral("onCursorPositionChanged: {\n            if (focused)\n                textBlock.activated()")));
    QVERIFY(!textBlockSource.contains(QStringLiteral("modifierVerticalNavigationHandler")));
    QVERIFY(!textBlockSource.contains(QStringLiteral("shortcutKeyPressHandler: function")));

    QVERIFY(agendaBlockSource.contains(QStringLiteral("property bool nativeTextInputPriority: false")));
    QVERIFY(agendaBlockSource.contains(QStringLiteral("EditorInputModel.ContentsAgendaTaskRowController")));
    QVERIFY(agendaTaskRowControllerSource.contains(QStringLiteral("property string liveTaskText: \"\"")));
    QVERIFY(agendaTaskRowControllerSource.contains(QStringLiteral("syncLiveTaskTextFromHost")));
    QVERIFY(agendaBlockSource.contains(QStringLiteral("signal taskTextChanged(var taskData, string text, int cursorPosition, string expectedPreviousText)")));
    QVERIFY(agendaTaskRowControllerSource.contains(QStringLiteral("controller.agendaBlock.taskTextChanged(")));
    QVERIFY(agendaBlockSource.contains(QStringLiteral("preferNativeInputHandling: true")));
    QVERIFY(agendaBlockSource.contains(QStringLiteral("signal cursorInteraction()")));
    QVERIFY(agendaTaskRowControllerSource.contains(QStringLiteral("controller.agendaBlock.cursorInteraction();")));
    QVERIFY(!agendaBlockSource.contains(QStringLiteral("modifierVerticalNavigationHandler")));
    QVERIFY(!agendaBlockSource.contains(QStringLiteral("shortcutKeyPressHandler: function")));

    QVERIFY(calloutBlockSource.contains(QStringLiteral("property bool nativeTextInputPriority: false")));
    QVERIFY(calloutBlockSource.contains(QStringLiteral("EditorInputModel.ContentsCalloutBlockController")));
    QVERIFY(calloutBlockControllerSource.contains(QStringLiteral("property string liveText: \"\"")));
    QVERIFY(calloutBlockControllerSource.contains(QStringLiteral("syncLiveTextFromHost")));
    QVERIFY(calloutBlockSource.contains(QStringLiteral("signal blockDeletionRequested(string direction)")));
    QVERIFY(calloutBlockSource.contains(QStringLiteral("signal textChanged(string text, int cursorPosition, string expectedPreviousText)")));
    QVERIFY(calloutBlockControllerSource.contains(QStringLiteral("controller.calloutBlock.textChanged(")));
    QVERIFY(calloutBlockSource.contains(QStringLiteral("preferNativeInputHandling: true")));
    QVERIFY(calloutBlockSource.contains(QStringLiteral("signal cursorInteraction()")));
    QVERIFY(calloutBlockControllerSource.contains(QStringLiteral("controller.calloutBlock.cursorInteraction();")));
    QVERIFY(calloutBlockSource.contains(QStringLiteral("tagManagementKeyPressHandler: function (event)")));
    QVERIFY(calloutBlockControllerSource.contains(QStringLiteral("function handleTagManagementKeyPress(event)")));
    QVERIFY(calloutBlockControllerSource.contains(QStringLiteral("key === Qt.Key_Backspace")));
    QVERIFY(calloutBlockControllerSource.contains(QStringLiteral("plainText.length !== 0")));
    QVERIFY(calloutBlockControllerSource.contains(QStringLiteral("controller.calloutBlock.blockDeletionRequested(\"backward\")")));
    QVERIFY(calloutBlockControllerSource.contains(QStringLiteral("textModifiers === Qt.ShiftModifier")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("event.accepted = false;")));
    QVERIFY(calloutBlockControllerSource.contains(QStringLiteral("controller.calloutBlock.enterExitRequested(")));
    QVERIFY(!calloutBlockSource.contains(QStringLiteral("modifierVerticalNavigationHandler")));
    QVERIFY(!calloutBlockSource.contains(QStringLiteral("shortcutKeyPressHandler: function")));

    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsEditorBodyTagInsertionPlanner")));
    QVERIFY(displayViewSource.contains(QStringLiteral("bodyTagInsertionPlanner: contentsBodyTagInsertionPlanner")));
    QVERIFY(typingControllerSource.contains(QStringLiteral("property var bodyTagInsertionPlanner: null")));
    QVERIFY(typingControllerSource.contains(QStringLiteral("bodyTagInsertionPlanner.buildRawSourceInsertionPayload(")));
    QVERIFY(typingControllerSource.contains(QStringLiteral("bodyTagInsertionPlanner.buildCalloutRangeWrappingPayload(")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("function completeStructuredShortcutInsertionSpec(")));
}

void WhatSonCppRegressionTests::qmlEditorInputPolicyAdapter_centralizesNativeInputDecisions()
{
    const QString policyAdapterSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsEditorInputPolicyAdapter.qml"));
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString mutationControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayMutationController.qml"));
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsInlineFormatEditor.qml"));
    const QString inlineEditorControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsInlineFormatEditorController.qml"));
    const QString textBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml"));
    const QString textBlockControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsDocumentTextBlockController.qml"));

    QVERIFY(!policyAdapterSource.isEmpty());
    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!mutationControllerSource.isEmpty());
    QVERIFY(!inlineEditorSource.isEmpty());
    QVERIFY(!inlineEditorControllerSource.isEmpty());
    QVERIFY(!textBlockSource.isEmpty());
    QVERIFY(!textBlockControllerSource.isEmpty());

    QVERIFY(policyAdapterSource.contains(QStringLiteral("readonly property bool nativeCompositionActive")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("readonly property bool nativeTextInputSessionActive")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("|| adapter.editorInputFocused")));
    QVERIFY(!policyAdapterSource.contains(
        QStringLiteral("|| (adapter.nativeTextInputPriority\n                                                             && adapter.editorInputFocused)")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("readonly property bool shortcutSurfaceEnabled")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("readonly property bool tagManagementShortcutSurfaceEnabled")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("&& adapter.noteDocumentCommandSurfaceEnabled")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("&& !adapter.nativeCompositionActive")));
    QVERIFY(!policyAdapterSource.contains(QStringLiteral("&& adapter.shortcutSurfaceEnabled")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("readonly property bool contextMenuLongPressEnabled")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("readonly property bool contextMenuSurfaceEnabled")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("function programmaticTextSyncPolicy(")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("function shouldDeferProgrammaticTextSync(")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("function shouldApplyProgrammaticTextSync(")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("function shouldRestoreFocusForMutation(")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("reason === \"text-edit\"")));

    QVERIFY(displayViewSource.contains(QStringLiteral("EditorInputModel.ContentsEditorInputPolicyAdapter {\n        id: editorInputPolicyAdapter")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool contextMenuLongPressEnabled: editorInputPolicyAdapter.contextMenuLongPressEnabled")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentShortcutSurfaceEnabled: editorInputPolicyAdapter.shortcutSurfaceEnabled")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentTagManagementShortcutSurfaceEnabled: editorInputPolicyAdapter.tagManagementShortcutSurfaceEnabled")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentContextMenuSurfaceEnabled: editorInputPolicyAdapter.contextMenuSurfaceEnabled")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("return editorInputPolicyAdapter.nativeTextInputSessionActive;")));
    QVERIFY(mutationControllerSource.contains(
        QStringLiteral("controller.editorInputPolicyAdapter.shouldRestoreFocusForMutation(")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("readonly property bool contextMenuLongPressEnabled: contentsView.nativeTextInputPriority")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("return contentsView.nativeTextInputPriority && contentsView.editorInputFocused;")));

    QVERIFY(inlineEditorSource.contains(QStringLiteral("EditorInputModel.ContentsInlineFormatEditorController")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("EditorInputModel.ContentsEditorInputPolicyAdapter {\n        id: inlineInputPolicyAdapter")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("ContentsEditorInputPolicyAdapter {\n        id: inlineInputPolicyAdapter")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("function programmaticTextSyncPolicy(nextText)")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("inlineInputPolicyAdapter.programmaticTextSyncPolicy(")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("const syncPolicy = controller.programmaticTextSyncPolicy(normalizedText);")));
    QVERIFY(!inlineEditorControllerSource.contains(QStringLiteral("return control.nativeCompositionActive();")));
    QVERIFY(!inlineEditorControllerSource.contains(QStringLiteral("control._localTextEditSinceFocus = false;\n            return false;")));

    QVERIFY(textBlockControllerSource.contains(QStringLiteral("\"reason\": \"text-edit\"")));
}

void WhatSonCppRegressionTests::qmlEditorViewDirectory_containsOnlyViewSurfaceFiles()
{
    const QDir viewDirectory(QStringLiteral("src/app/qml/view/content/editor"));
    QVERIFY(viewDirectory.exists());

    QStringList actualFiles = viewDirectory.entryList(
        QStringList{QStringLiteral("*.qml"), QStringLiteral("*.js")},
        QDir::Files,
        QDir::Name);
    const QStringList expectedFiles{
        QStringLiteral("ContentsAgendaBlock.qml"),
        QStringLiteral("ContentsAgendaLayer.qml"),
        QStringLiteral("ContentsBreakBlock.qml"),
        QStringLiteral("ContentsCalloutBlock.qml"),
        QStringLiteral("ContentsCalloutLayer.qml"),
        QStringLiteral("ContentsDisplayAuxiliaryRailHost.qml"),
        QStringLiteral("ContentsDisplayExceptionOverlay.qml"),
        QStringLiteral("ContentsDisplayGutterHost.qml"),
        QStringLiteral("ContentsDisplayMinimapRailHost.qml"),
        QStringLiteral("ContentsDisplayOverlayHost.qml"),
        QStringLiteral("ContentsDisplayResourceImportConflictAlert.qml"),
        QStringLiteral("ContentsDisplaySurfaceHost.qml"),
        QStringLiteral("ContentsDisplayView.qml"),
        QStringLiteral("ContentsDocumentBlock.qml"),
        QStringLiteral("ContentsDocumentTextBlock.qml"),
        QStringLiteral("ContentsGutterLayer.qml"),
        QStringLiteral("ContentsImageResourceFrame.qml"),
        QStringLiteral("ContentsInlineFormatEditor.qml"),
        QStringLiteral("ContentsMinimapLayer.qml"),
        QStringLiteral("ContentsResourceBlock.qml"),
        QStringLiteral("ContentsResourceEditorView.qml"),
        QStringLiteral("ContentsResourceLayer.qml"),
        QStringLiteral("ContentsResourceRenderCard.qml"),
        QStringLiteral("ContentsResourceViewer.qml"),
        QStringLiteral("ContentsStructuredDocumentFlow.qml"),
    };
    QCOMPARE(actualFiles, expectedFiles);

    const QString editorCmakeSource = readUtf8SourceFile(QStringLiteral("src/app/models/editor/CMakeLists.txt"));
    QVERIFY(editorCmakeSource.contains(QStringLiteral("whatson_app_register_directory_qml")));

    const QString viewmodelCmakeSource = readUtf8SourceFile(QStringLiteral("src/app/viewmodel/CMakeLists.txt"));
    const QString testCmakeSource = readUtf8SourceFile(QStringLiteral("test/cpp/CMakeLists.txt"));
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString documentBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentBlock.qml"));
    const QString documentTextBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml"));
    const QString calloutBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsCalloutBlock.qml"));
    const QString agendaBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsAgendaBlock.qml"));
    const QString auxiliaryHostSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayAuxiliaryRailHost.qml"));
    const QString surfaceHostSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplaySurfaceHost.qml"));
    const QString overlayHostSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayOverlayHost.qml"));
    const QString eventPumpSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayEventPump.qml"));
    const QString inputCommandSurfaceSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayInputCommandSurface.qml"));
    const QString mutationControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayMutationController.qml"));
    const QString geometryControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayGeometryController.qml"));
    const QString displayControllerBridgeHeader = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsDisplayControllerBridgeViewModel.hpp"));
    const QString mutationViewModelHeader = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsDisplayMutationViewModel.hpp"));
    const QString mutationViewModelCpp = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsDisplayMutationViewModel.cpp"));
    const QString presentationViewModelHeader = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsDisplayPresentationViewModel.hpp"));
    const QString presentationViewModelCpp = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsDisplayPresentationViewModel.cpp"));
    const QString selectionMountViewModelHeader = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsDisplaySelectionMountViewModel.hpp"));
    const QString selectionMountViewModelCpp = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsDisplaySelectionMountViewModel.cpp"));
    const QString geometryViewModelHeader = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsDisplayGeometryViewModel.hpp"));
    const QString geometryViewModelCpp = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsDisplayGeometryViewModel.cpp"));
    const QString activeSurfaceAdapterHeader = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsActiveEditorSurfaceAdapter.hpp"));
    const QString activeSurfaceAdapterCpp = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsActiveEditorSurfaceAdapter.cpp"));
    const QString surfacePolicyHeader = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsDisplaySurfacePolicy.hpp"));
    const QString surfacePolicyCpp = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsDisplaySurfacePolicy.cpp"));
    QVERIFY(!displayControllerBridgeHeader.isEmpty());
    QVERIFY(!mutationViewModelHeader.isEmpty());
    QVERIFY(!mutationViewModelCpp.isEmpty());
    QVERIFY(!presentationViewModelHeader.isEmpty());
    QVERIFY(!presentationViewModelCpp.isEmpty());
    QVERIFY(!selectionMountViewModelHeader.isEmpty());
    QVERIFY(!selectionMountViewModelCpp.isEmpty());
    QVERIFY(!geometryViewModelHeader.isEmpty());
    QVERIFY(!geometryViewModelCpp.isEmpty());
    QVERIFY(!activeSurfaceAdapterHeader.isEmpty());
    QVERIFY(!activeSurfaceAdapterCpp.isEmpty());
    QVERIFY(!surfacePolicyHeader.isEmpty());
    QVERIFY(!surfacePolicyCpp.isEmpty());
    QVERIFY(!documentBlockSource.isEmpty());
    QVERIFY(!documentTextBlockSource.isEmpty());
    QVERIFY(!calloutBlockSource.isEmpty());
    QVERIFY(!agendaBlockSource.isEmpty());
    QVERIFY(!auxiliaryHostSource.isEmpty());
    QVERIFY(!surfaceHostSource.isEmpty());
    QVERIFY(!overlayHostSource.isEmpty());
    QVERIFY(!viewmodelCmakeSource.contains(QStringLiteral("whatson_app_register_directory_qml")));
    QVERIFY(testCmakeSource.contains(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsActiveEditorSurfaceAdapter.cpp")));
    QVERIFY(testCmakeSource.contains(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsDisplaySurfacePolicy.cpp")));
    QVERIFY(displayControllerBridgeHeader.contains(QStringLiteral("Q_PROPERTY(QObject* controller READ controller WRITE setController NOTIFY controllerChanged)")));
    QVERIFY(displayControllerBridgeHeader.contains(QStringLiteral("public slots:")));
    QVERIFY(displayControllerBridgeHeader.contains(QStringLiteral("void controllerChanged();")));
    QVERIFY(mutationViewModelHeader.contains(QStringLiteral("class ContentsDisplayMutationViewModel")));
    QVERIFY(mutationViewModelHeader.contains(QStringLiteral("public slots:")));
    QVERIFY(mutationViewModelCpp.contains(QStringLiteral("invokeControllerBool(\"applyDocumentSourceMutation\"")));
    QVERIFY(presentationViewModelHeader.contains(QStringLiteral("class ContentsDisplayPresentationViewModel")));
    QVERIFY(presentationViewModelHeader.contains(QStringLiteral("public slots:")));
    QVERIFY(presentationViewModelCpp.contains(QStringLiteral("invokeControllerVoid(\"scheduleDocumentPresentationRefresh\"")));
    QVERIFY(selectionMountViewModelHeader.contains(QStringLiteral("class ContentsDisplaySelectionMountViewModel")));
    QVERIFY(selectionMountViewModelHeader.contains(QStringLiteral("public slots:")));
    QVERIFY(selectionMountViewModelCpp.contains(QStringLiteral("invokeControllerVoid(\"scheduleSelectionModelSync\"")));
    QVERIFY(geometryViewModelHeader.contains(QStringLiteral("class ContentsDisplayGeometryViewModel")));
    QVERIFY(geometryViewModelHeader.contains(QStringLiteral("public slots:")));
    QVERIFY(geometryViewModelCpp.contains(QStringLiteral("invokeControllerVoid(\"refreshMinimapSnapshot\")")));
    QVERIFY(activeSurfaceAdapterHeader.contains(QStringLiteral("class ContentsActiveEditorSurfaceAdapter")));
    QVERIFY(activeSurfaceAdapterHeader.contains(QStringLiteral("Q_PROPERTY(QObject* structuredDocumentFlow")));
    QVERIFY(activeSurfaceAdapterHeader.contains(QStringLiteral("bool requestFocus(const QVariant& request)")));
    QVERIFY(activeSurfaceAdapterCpp.contains(QStringLiteral("requestStructuredFocus")));
    QVERIFY(activeSurfaceAdapterCpp.contains(QStringLiteral("requestInlineFocus")));
    QVERIFY(surfacePolicyHeader.contains(QStringLiteral("class ContentsDisplaySurfacePolicy")));
    QVERIFY(surfacePolicyHeader.contains(QStringLiteral("Q_PROPERTY(bool structuredDocumentSurfaceRequested")));
    QVERIFY(surfacePolicyCpp.contains(QStringLiteral("return false;")));
    QVERIFY(displayViewSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplayEventPump")));
    QVERIFY(surfaceHostSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplayInputCommandSurface")));
    QVERIFY(displayViewSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplayPresentationController")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsDisplayPresentationViewModel")));
    QVERIFY(displayViewSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplayMutationController")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsDisplayMutationViewModel")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsActiveEditorSurfaceAdapter")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsDisplaySurfacePolicy")));
    QVERIFY(displayViewSource.contains(QStringLiteral("activeEditorSurface: activeEditorSurfaceAdapter")));
    QVERIFY(displayViewSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplaySelectionMountController")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsDisplaySelectionMountViewModel")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsDisplayAuxiliaryRailHost")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsDisplayOverlayHost")));
    QVERIFY(auxiliaryHostSource.contains(QStringLiteral("ContentsDisplayGutterHost")));
    QVERIFY(auxiliaryHostSource.contains(QStringLiteral("ContentsDisplayMinimapRailHost")));
    QVERIFY(overlayHostSource.contains(QStringLiteral("ContentsDisplayExceptionOverlay")));
    QVERIFY(!surfaceHostSource.contains(QStringLiteral("ContentsDisplayMountLoadingOverlay")));
    QVERIFY(overlayHostSource.contains(QStringLiteral("ContentsDisplayResourceImportConflictAlert")));
    QVERIFY(displayViewSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplayGeometryController")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsDisplayGeometryViewModel")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("Timer {")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("Connections {")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("Shortcut {")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("MouseArea {")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("LV.ContextMenu")));
    QVERIFY(eventPumpSource.contains(QStringLiteral("Timer {")));
    QVERIFY(eventPumpSource.contains(QStringLiteral("Connections {")));
    QVERIFY(inputCommandSurfaceSource.contains(QStringLiteral("Shortcut {")));
    QVERIFY(inputCommandSurfaceSource.contains(QStringLiteral("LV.ContextMenu")));
    QVERIFY(documentBlockSource.contains(
        QStringLiteral("tagManagementShortcutKeyPressHandler: documentBlock.tagManagementShortcutKeyPressHandler")));
    QVERIFY(documentTextBlockSource.contains(QStringLiteral("property var tagManagementShortcutKeyPressHandler")));
    QVERIFY(documentTextBlockSource.contains(QStringLiteral("function invokeTagManagementShortcut(event)")));
    QVERIFY(documentTextBlockSource.contains(QStringLiteral("tagManagementKeyPressHandler: function (event)")));
    QVERIFY(calloutBlockSource.contains(QStringLiteral("property var tagManagementShortcutKeyPressHandler")));
    QVERIFY(calloutBlockSource.contains(QStringLiteral("function invokeDocumentTagManagementShortcut(event)")));
    QVERIFY(calloutBlockSource.contains(QStringLiteral("calloutBlock.invokeDocumentTagManagementShortcut(event)")));
    QVERIFY(agendaBlockSource.contains(QStringLiteral("property var tagManagementShortcutKeyPressHandler")));
    QVERIFY(agendaBlockSource.contains(QStringLiteral("function invokeTagManagementShortcut(event)")));
    QVERIFY(agendaBlockSource.contains(QStringLiteral("agendaBlock.invokeTagManagementShortcut(event)")));
    QVERIFY(mutationControllerSource.contains(QStringLiteral("function applyDocumentSourceMutation(")));
    QVERIFY(geometryControllerSource.contains(QStringLiteral("function refreshMinimapSnapshot()")));

    const QDir repositoryRoot(qmlStructuredEditorsRepositoryRootPath());
    QDirIterator viewmodelQmlFiles(
        repositoryRoot.filePath(QStringLiteral("src/app/viewmodel")),
        QStringList{QStringLiteral("*.qml")},
        QDir::Files | QDir::NoDotAndDotDot,
        QDirIterator::Subdirectories);
    QVERIFY(!viewmodelQmlFiles.hasNext());
}

void WhatSonCppRegressionTests::qmlStructuredEditors_lockCustomInputToTagManagementOnly()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString surfaceHostSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplaySurfaceHost.qml"));
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));
    const QString documentBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentBlock.qml"));
    const QString documentBlockControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsDocumentBlockController.qml"));
    const QString breakBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsBreakBlock.qml"));
    const QString breakBlockControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsBreakBlockController.qml"));
    const QString textBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml"));
    const QString agendaBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsAgendaBlock.qml"));
    const QString calloutBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsCalloutBlock.qml"));
    const QString selectionControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsEditorSelectionController.qml"));
    const QString typingControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsEditorTypingController.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!surfaceHostSource.isEmpty());
    QVERIFY(!structuredFlowSource.isEmpty());
    QVERIFY(!documentBlockSource.isEmpty());
    QVERIFY(!documentBlockControllerSource.isEmpty());
    QVERIFY(!breakBlockSource.isEmpty());
    QVERIFY(!breakBlockControllerSource.isEmpty());
    QVERIFY(!textBlockSource.isEmpty());
    QVERIFY(!agendaBlockSource.isEmpty());
    QVERIFY(!calloutBlockSource.isEmpty());
    QVERIFY(!selectionControllerSource.isEmpty());
    QVERIFY(!typingControllerSource.isEmpty());

    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property bool editorCustomTextInputEnabled: false")));
    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property bool editorTagManagementInputEnabled: true")));
    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property bool noteDocumentTagManagementShortcutSurfaceEnabled")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function handleTagManagementShortcutKeyPress(event)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function inlineFormatShortcutTag(event)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function handleInlineFormatTagShortcut(event)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contentsView.queueInlineFormatWrap(tagName)")));
    QVERIFY(surfaceHostSource.contains(QStringLiteral("tagManagementShortcutKeyPressHandler: function (event)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentTagManagementShortcutSurfaceEnabled: editorInputPolicyAdapter.tagManagementShortcutSurfaceEnabled")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("function handleInlineFormatShortcutKeyPress(event)")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("handlePlainEnterKeyPress(event)")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("handleTagAwareDeleteKeyPress(event)")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("queueMarkdownListMutation")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("sequence: \"Meta+Shift+7\"")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("sequence: \"Alt+Shift+7\"")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("sequence: \"Meta+Shift+8\"")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("sequence: \"Alt+Shift+8\"")));

    QVERIFY(structuredFlowSource.contains(QStringLiteral("property var tagManagementShortcutKeyPressHandler: null")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function blockEntryIsTagManagedAtomicBlock(blockEntry)")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function handleActiveTagManagementKeyPress(event)")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("property var shortcutKeyPressHandler")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("function handleActiveBlockDeleteKeyPress(event)")));

    QVERIFY(documentBlockSource.contains(QStringLiteral("property var tagManagementShortcutKeyPressHandler: null")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("function handleAtomicTagManagementKeyPress(event)")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("function handleAtomicTagDeleteKeyPress(event)")));
    QVERIFY(documentBlockControllerSource.contains(QStringLiteral("modifiers !== Qt.NoModifier")));
    QVERIFY(documentBlockControllerSource.contains(QStringLiteral("modifiers === Qt.MetaModifier")));
    QVERIFY(!documentBlockSource.contains(QStringLiteral("macModifierVerticalNavigation")));
    QVERIFY(!documentBlockControllerSource.contains(QStringLiteral("return !!blockItem.handleDeleteKeyPress(event)")));

    QVERIFY(breakBlockSource.contains(QStringLiteral("property var tagManagementShortcutKeyPressHandler: null")));
    QVERIFY(breakBlockControllerSource.contains(QStringLiteral("modifiers !== Qt.NoModifier")));
    QVERIFY(breakBlockControllerSource.contains(QStringLiteral("modifiers === Qt.MetaModifier")));
    QVERIFY(!breakBlockSource.contains(QStringLiteral("macModifierVerticalNavigation")));
    QVERIFY(!breakBlockSource.contains(QStringLiteral("property var shortcutKeyPressHandler")));

    QVERIFY(!textBlockSource.contains(QStringLiteral("Keys.onPressed")));
    QVERIFY(!textBlockSource.contains(QStringLiteral("function handleDeleteKeyPress(event)")));
    QVERIFY(!textBlockSource.contains(QStringLiteral("function handleAtomicBlockBoundaryKeyPress(event)")));
    QVERIFY(!textBlockSource.contains(QStringLiteral("shortcutKeyPressHandler")));
    QVERIFY(!agendaBlockSource.contains(QStringLiteral("function handleBoundaryKeyPress(event)")));
    QVERIFY(!agendaBlockSource.contains(QStringLiteral("shortcutKeyPressHandler")));
    QVERIFY(!calloutBlockSource.contains(QStringLiteral("function handleBoundaryKeyPress(event)")));
    QVERIFY(!calloutBlockSource.contains(QStringLiteral("shortcutKeyPressHandler")));

    QVERIFY(!selectionControllerSource.contains(QStringLiteral("handleInlineFormatShortcutKeyPress")));
    QVERIFY(!selectionControllerSource.contains(QStringLiteral("queueStructuredShortcutMutation")));
    QVERIFY(!selectionControllerSource.contains(QStringLiteral("queuedStructuredShortcutMutationKeys")));
    QVERIFY(!selectionControllerSource.contains(QStringLiteral("queueMarkdownListMutation")));
    QVERIFY(!selectionControllerSource.contains(QStringLiteral("queuedMarkdownListMutationKeys")));

    QVERIFY(!typingControllerSource.contains(QStringLiteral("handlePlainEnterKeyPress")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("handleTagAwareDeleteKeyPress")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("applyDirectRawSourceReplacement")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("continuedListInsertion")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("markerOnlyListBreakInsertion")));
}
