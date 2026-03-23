#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QRegularExpression>
#include <QtTest>

#include <algorithm>

namespace
{
    int countChar(const QString& line, QChar value)
    {
        return static_cast<int>(std::count(line.begin(), line.end(), value));
    }

    QString extractFunctionBody(const QString& text, const QString& signature)
    {
        const qsizetype signatureIndex = text.indexOf(signature);
        if (signatureIndex < 0)
            return {};

        const qsizetype braceStart = text.indexOf(QLatin1Char('{'), signatureIndex);
        if (braceStart < 0)
            return {};

        int braceDepth = 0;
        for (qsizetype index = braceStart; index < text.size(); ++index)
        {
            const QChar ch = text.at(index);
            if (ch == QLatin1Char('{'))
                ++braceDepth;
            else if (ch == QLatin1Char('}'))
            {
                --braceDepth;
                if (braceDepth == 0)
                    return text.mid(braceStart + 1, index - braceStart - 1);
            }
        }

        return {};
    }
} // namespace

class QmlBindingSyntaxGuardTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void bindingBlocks_mustNotContainStandaloneStringLiteral();
    void qmlFiles_mustNotContainStandaloneDottedExpression();
    void criticalViewHelpers_mustReturnExplicitValues();
    void contentView_mustComposeTextEditorGutter();
    void hierarchySidebarWiring_mustBindLoaderAndToolbarTarget();
    void mobileHierarchyPage_mustRouteHierarchyActivationIntoNoteListBody();
    void noteListDeleteShortcutWiring_mustStayCentralized();
};

void QmlBindingSyntaxGuardTest::bindingBlocks_mustNotContainStandaloneStringLiteral()
{
    const QDir testsDir(QStringLiteral(QT_TESTCASE_SOURCEDIR));
    const QString qmlRoot = testsDir.absoluteFilePath(QStringLiteral("../src/app/qml"));

    QDirIterator iterator(
        qmlRoot,
        QStringList{QStringLiteral("*.qml")},
        QDir::Files,
        QDirIterator::Subdirectories);

    const QRegularExpression standaloneStringPattern(QStringLiteral("^\"[^\"]*\"$"));

    QStringList violations;
    while (iterator.hasNext())
    {
        const QString qmlPath = iterator.next();
        QFile file(qmlPath);
        QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(qmlPath));

        const QString content = QString::fromUtf8(file.readAll());
        const QStringList lines = content.split(QLatin1Char('\n'));

        bool inBinding = false;
        int bindingBraceDepth = 0;
        for (qsizetype i = 0; i < lines.size(); ++i)
        {
            const QString line = lines.at(i);
            const QString trimmed = line.trimmed();

            if (!inBinding)
            {
                if (trimmed.startsWith(QStringLiteral("Binding")) && trimmed.contains(QLatin1Char('{')))
                {
                    inBinding = true;
                    bindingBraceDepth = countChar(line, QLatin1Char('{')) - countChar(line, QLatin1Char('}'));
                    if (bindingBraceDepth <= 0)
                    {
                        inBinding = false;
                    }
                }
                continue;
            }

            if (standaloneStringPattern.match(trimmed).hasMatch())
            {
                violations.push_back(
                    QStringLiteral("%1:%2 -> %3").arg(qmlPath, QString::number(i + 1), trimmed));
            }

            bindingBraceDepth += countChar(line, QLatin1Char('{')) - countChar(line, QLatin1Char('}'));
            if (bindingBraceDepth <= 0)
            {
                inBinding = false;
            }
        }
    }

    QVERIFY2(
        violations.isEmpty(),
        qPrintable(QStringLiteral(
                "Invalid standalone string literal found inside Binding block(s):\n%1")
            .arg(violations.join(QLatin1Char('\n')))));
}

void QmlBindingSyntaxGuardTest::qmlFiles_mustNotContainStandaloneDottedExpression()
{
    const QDir testsDir(QStringLiteral(QT_TESTCASE_SOURCEDIR));
    const QString qmlRoot = testsDir.absoluteFilePath(QStringLiteral("../src/app/qml"));

    QDirIterator iterator(
        qmlRoot,
        QStringList{QStringLiteral("*.qml")},
        QDir::Files,
        QDirIterator::Subdirectories);

    const QRegularExpression standaloneExpressionPattern(
        QStringLiteral("^\\s*[A-Za-z_][A-Za-z0-9_]*(?:\\.[A-Za-z_][A-Za-z0-9_]*)+\\s*$"));

    QStringList violations;
    while (iterator.hasNext())
    {
        const QString qmlPath = iterator.next();
        QFile file(qmlPath);
        QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(qmlPath));

        const QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char('\n'));
        for (qsizetype i = 0; i < lines.size(); ++i)
        {
            const QString trimmed = lines.at(i).trimmed();
            if (trimmed.startsWith(QStringLiteral("//")))
                continue;
            if (!standaloneExpressionPattern.match(trimmed).hasMatch())
                continue;
            violations.push_back(
                QStringLiteral("%1:%2 -> %3").arg(qmlPath, QString::number(i + 1), trimmed));
        }
    }

    QVERIFY2(
        violations.isEmpty(),
        qPrintable(QStringLiteral(
                "Invalid standalone dotted-expression line(s) found in QML:\n%1")
            .arg(violations.join(QLatin1Char('\n')))));
}

void QmlBindingSyntaxGuardTest::criticalViewHelpers_mustReturnExplicitValues()
{
    const QDir testsDir(QStringLiteral(QT_TESTCASE_SOURCEDIR));
    const QString qmlRoot = testsDir.absoluteFilePath(QStringLiteral("../src/app/qml"));
    const QString contentViewPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/content/editor/ContentsDisplayView.qml"));
    QFile contentViewFile(contentViewPath);
    QVERIFY2(contentViewFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(contentViewPath));
    const QString contentViewText = QString::fromUtf8(contentViewFile.readAll());
    const QString editorSessionPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/content/editor/ContentsEditorSession.qml"));
    QFile editorSessionFile(editorSessionPath);
    QVERIFY2(editorSessionFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(editorSessionPath));
    const QString editorSessionText = QString::fromUtf8(editorSessionFile.readAll());
    const QString fallbackRowsBody = extractFunctionBody(
        contentViewText, QStringLiteral("function buildFallbackMinimapVisualRows(textStartY)"));
    QVERIFY2(
        !fallbackRowsBody.isEmpty() && fallbackRowsBody.contains(QStringLiteral("return rows;")),
        "ContentsDisplayView.qml buildFallbackMinimapVisualRows() must return the assembled fallback rows.");

    const QString flushPendingBody = extractFunctionBody(
        editorSessionText, QStringLiteral("function flushPendingEditorText()"));
    QVERIFY2(
        !flushPendingBody.isEmpty() &&
        flushPendingBody.contains(QStringLiteral("return true;")) &&
        flushPendingBody.contains(QStringLiteral("return false;")),
        "ContentsEditorSession.qml flushPendingEditorText() must return an explicit success/failure value on every path.");

    const QString markerColorBody = extractFunctionBody(
        contentViewText, QStringLiteral("function markerColorForType(markerType)"));
    QVERIFY2(
        !markerColorBody.isEmpty() &&
        markerColorBody.contains(QStringLiteral("return contentsView.gutterMarkerCurrentColor;")),
        "ContentsDisplayView.qml markerColorForType() must keep an explicit fallback color instead of leaking undefined.");

    const QString resolveFlickableBody = extractFunctionBody(
        contentViewText, QStringLiteral("function resolveEditorFlickable()"));
    QVERIFY2(
        !resolveFlickableBody.isEmpty() &&
        resolveFlickableBody.contains(QStringLiteral("return candidate;")),
        "ContentsDisplayView.qml resolveEditorFlickable() must return the resolved editor flickable on the success path.");

    const QString visibleLinesBody = extractFunctionBody(
        contentViewText, QStringLiteral("function visibleLineNumbers()"));
    QVERIFY2(
        !visibleLinesBody.isEmpty() &&
        visibleLinesBody.contains(QStringLiteral("return visibleLines;")),
        "ContentsDisplayView.qml visibleLineNumbers() must return the visible logical-line model for the gutter.");
}

void QmlBindingSyntaxGuardTest::contentView_mustComposeTextEditorGutter()
{
    const QDir testsDir(QStringLiteral(QT_TESTCASE_SOURCEDIR));
    const QString qmlRoot = testsDir.absoluteFilePath(QStringLiteral("../src/app/qml"));

    const QString contentViewLayoutPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/ContentViewLayout.qml"));
    QFile contentViewLayoutFile(contentViewLayoutPath);
    QVERIFY2(contentViewLayoutFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(contentViewLayoutPath));
    const QString contentViewLayoutText = QString::fromUtf8(contentViewLayoutFile.readAll());
    QVERIFY2(
        contentViewLayoutText.contains(QStringLiteral("ContentsDisplayView {")),
        "ContentViewLayout.qml must remain a panel-level wrapper that composes the dedicated contents surface.");
    QVERIFY2(
        contentViewLayoutText.contains(
            QStringLiteral(
                "panelViewModelRegistry ? panelViewModelRegistry.panelViewModel(\"ContentViewLayout\") : null")),
        "ContentViewLayout.qml must keep panel-view-model wiring at the panel wrapper layer.");

    const QString contentViewPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/content/editor/ContentsDisplayView.qml"));
    QFile contentViewFile(contentViewPath);
    QVERIFY2(contentViewFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(contentViewPath));
    const QString contentViewText = QString::fromUtf8(contentViewFile.readAll());
    const QString contentViewSessionPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/content/editor/ContentsEditorSession.qml"));
    QFile contentViewSessionFile(contentViewSessionPath);
    QVERIFY2(contentViewSessionFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(contentViewSessionPath));
    const QString contentViewSessionText = QString::fromUtf8(contentViewSessionFile.readAll());

    QVERIFY2(
        contentViewText.contains(QStringLiteral("property color displayColor: \"transparent\"")),
        "ContentsDisplayView.qml must keep the editor surface transparent so the ApplicationWindow canvas shows through.");
    QVERIFY2(
        !contentViewText.contains(QStringLiteral("property color panelColor:")),
        "ContentsDisplayView.qml must not reintroduce a second panel-color wrapper contract inside the editor surface.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("readonly property int gutterWidth: 74")),
        "ContentViewLayout.qml must keep the 74px line-number gutter width from the Figma frame.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("property color gutterColor: \"transparent\"")),
        "ContentsDisplayView.qml must keep the desktop gutter fill transparent while still allowing route-specific overrides.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("readonly property int frameHorizontalInset: 2")),
        "ContentViewLayout.qml must preserve the Figma 2px horizontal frame inset around the gutter/editor stack.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("property var noteListModel: null")),
        "ContentViewLayout.qml must accept the active note list model for body-text projection.");
    QVERIFY2(
        contentViewLayoutText.contains(
            QStringLiteral("readonly property var resolvedContentViewModel: contentViewLayout.contentViewModel")),
        "ContentViewLayout.qml must normalize the incoming content view-model through a resolved contract property.");
    QVERIFY2(
        contentViewLayoutText.contains(
            QStringLiteral("readonly property var resolvedNoteListModel: contentViewLayout.noteListModel")),
        "ContentViewLayout.qml must normalize the incoming note-list model through a resolved contract property.");
    QVERIFY2(
        contentViewLayoutText.contains(QStringLiteral("contentViewModel: contentViewLayout.resolvedContentViewModel")),
        "ContentViewLayout.qml must pass the resolved content view-model into ContentsDisplayView.");
    QVERIFY2(
        contentViewLayoutText.contains(QStringLiteral("property color gutterColor: \"transparent\"")) &&
            contentViewLayoutText.contains(QStringLiteral("gutterColor: contentViewLayout.gutterColor")),
        "ContentViewLayout.qml must expose the shared gutter-color contract so desktop can stay transparent and mobile routes can still override editor gutter fill without forking the surface.");
    QVERIFY2(
        contentViewLayoutText.contains(QStringLiteral("noteListModel: contentViewLayout.resolvedNoteListModel")),
        "ContentViewLayout.qml must pass the resolved note-list model into ContentsDisplayView.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("import WhatSon.App.Internal 1.0")),
        "ContentsDisplayView.qml must import the internal backend bridge module for editor-side state delegation.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("ContentsEditorSelectionBridge {")),
        "ContentsDisplayView.qml must compose a dedicated selection/persistence backend adapter.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("ContentsLogicalTextBridge {")),
        "ContentsDisplayView.qml must compose a dedicated logical-text backend adapter.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("ContentsGutterMarkerBridge {")),
        "ContentsDisplayView.qml must compose a dedicated gutter-marker backend adapter.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("ContentsEditorSession {")),
        "ContentsDisplayView.qml must compose a dedicated editor-session object for debounce and selection sync.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("contentViewModel: contentsView.contentViewModel")),
        "ContentsDisplayView.qml must bind the active hierarchy view-model into the selection adapter.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("noteListModel: contentsView.noteListModel")),
        "ContentsDisplayView.qml must bind the active note-list model into the selection adapter.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("text: contentsView.editorText")),
        "ContentsDisplayView.qml must bind editor text into the logical-text adapter.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("gutterMarkers: contentsView.gutterMarkers")),
        "ContentsDisplayView.qml must bind external gutter markers into the gutter-marker adapter.");
    QVERIFY2(
        contentViewText.contains(
            QStringLiteral("readonly property int visibleNoteCount: selectionBridge.visibleNoteCount")),
        "ContentsDisplayView.qml must derive the visible note count from the selection adapter instead of probing the note-list model inline.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("visible: contentsView.hasSelectedNote")),
        "ContentsDisplayView.qml must only expose the editor stack when a real note is selected.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("property var contentViewModel: null")),
        "ContentViewLayout.qml must accept the active hierarchy view-model for body persistence.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral(
            "readonly property bool noteSelectionContractAvailable: selectionBridge.noteSelectionContractAvailable")),
        "ContentsDisplayView.qml must expose note-selection availability through the selection adapter.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral(
            "readonly property bool noteCountContractAvailable: selectionBridge.noteCountContractAvailable")),
        "ContentsDisplayView.qml must expose note-count availability through the selection adapter.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral(
            "readonly property bool contentPersistenceContractAvailable: selectionBridge.contentPersistenceContractAvailable")),
        "ContentsDisplayView.qml must expose the persistence contract through the selection adapter.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("property alias pendingBodySave: editorSession.pendingBodySave")),
        "ContentViewLayout.qml must route debounced body persistence state through the dedicated session object.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("property int gutterRefreshRevision: 0")),
        "ContentViewLayout.qml must expose an explicit gutter refresh revision so late TextEditor layout can be re-sampled.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function scheduleGutterRefresh(passCount)")),
        "ContentViewLayout.qml must expose a helper that schedules multi-pass gutter refresh after note/view transitions.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function commitGutterRefresh()")),
        "ContentViewLayout.qml must separate the actual gutter refresh pulse from the scheduling path.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function visibleLineNumbers()")),
        "ContentViewLayout.qml must derive visible line numbers for the gutter viewport.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("const refreshRevision = contentsView.gutterRefreshRevision;")),
        "ContentViewLayout.qml gutter calculations must depend on an explicit refresh revision so visibility refresh pulses invalidate stale layout samples.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("const firstVisibleLine = contentsView.firstVisibleLogicalLine();")),
        "ContentViewLayout.qml must choose the initial visible gutter line from viewport-space line positions.");
    QVERIFY2(
        contentViewText.contains(
            QStringLiteral("readonly property var logicalLineStartOffsets: textMetricsBridge.logicalLineStartOffsets")),
        "ContentsDisplayView.qml must track logical-line offsets through the logical-text adapter.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("return visibleLines;")),
        "ContentViewLayout.qml must return the visible gutter line model instead of leaving the gutter empty.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("textMetricsBridge.logicalLineNumberForOffset(")),
        "ContentsDisplayView.qml must resolve cursor and minimap line numbers through the logical-text adapter.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function logicalLineNumberForDocumentY(documentY)")),
        "ContentViewLayout.qml must derive the first visible logical line from document Y rather than rescanning from line 1 each repaint.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function firstVisibleLogicalLine()")),
        "ContentViewLayout.qml must derive the first rendered gutter row from viewport-space line positions.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("positionToRectangle(safeOffset)")),
        "ContentViewLayout.qml must map gutter positions through editorItem.positionToRectangle for wrapped text.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function lineVisualHeight(startLine, lineSpan)")),
        "ContentViewLayout.qml must expand gutter markers to the visual height of wrapped logical lines.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function currentCursorVisualLineHeight()")),
        "ContentViewLayout.qml must expose the active cursor-row height so the current-line gutter marker can track the visual row being edited.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function markerHeight(markerSpec)")),
        "ContentViewLayout.qml must route gutter marker height through a dedicated helper so current-line and range markers can use different height rules.");
    const QString gutterLayerPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/content/editor/ContentsGutterLayer.qml"));
    QFile gutterLayerFile(gutterLayerPath);
    QVERIFY2(gutterLayerFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(gutterLayerPath));
    const QString gutterLayerText = QString::fromUtf8(gutterLayerFile.readAll());
    const QString minimapLayerPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/content/editor/ContentsMinimapLayer.qml"));
    QFile minimapLayerFile(minimapLayerPath);
    QVERIFY2(minimapLayerFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(minimapLayerPath));
    const QString minimapLayerText = QString::fromUtf8(minimapLayerFile.readAll());
    QVERIFY2(
        contentViewText.contains(QStringLiteral("ContentsGutterLayer {")),
        "ContentViewLayout.qml must compose the dedicated ContentsGutterLayer module for SRP-aligned gutter rendering.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("ContentsMinimapLayer {")),
        "ContentViewLayout.qml must compose the dedicated ContentsMinimapLayer module for SRP-aligned minimap rendering.");
    QVERIFY2(
        gutterLayerText.contains(
            QStringLiteral(
                "height: gutterLayer.markerHeightResolver ? Number(gutterLayer.markerHeightResolver(markerSpec)) || 0 : 0")),
        "ContentsGutterLayer.qml gutter marker delegate must resolve marker height via the injected markerHeight resolver.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("readonly property color lineNumberColor: \"#4E5157\"")),
        "ContentViewLayout.qml must keep the gutter caption token color from Figma.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("readonly property color activeLineNumberColor: \"#9DA0A8\"")),
        "ContentViewLayout.qml must keep the focused line-number tint from Figma.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("readonly property color decorativeMarkerYellow: \"#FFF567\"")),
        "ContentViewLayout.qml must keep the changed-line gutter marker color contract.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("readonly property color gutterMarkerCurrentColor: LV.Theme.primary")),
        "ContentViewLayout.qml must expose the current-line gutter marker color contract.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("readonly property color gutterMarkerConflictColor: LV.Theme.danger")),
        "ContentViewLayout.qml must expose the conflict gutter marker color contract.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("property var gutterMarkers: []")),
        "ContentViewLayout.qml must accept external gutter markers for changed/conflict ranges.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function markerColorForType(markerType)")),
        "ContentViewLayout.qml must normalize gutter marker types into stable colors.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral(
            "readonly property var normalizedExternalGutterMarkers: gutterMarkerBridge.normalizedExternalGutterMarkers")),
        "ContentsDisplayView.qml must consume backend-normalized gutter marker specs from the gutter-marker adapter.");
    QVERIFY2(
        contentViewText.contains(
            QStringLiteral("readonly property int lineNumberRightInset: contentsView.editorHorizontalInset")),
        "ContentViewLayout.qml must right-align gutter numbers against the editor inset.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral(
            "readonly property int lineNumberColumnTextWidth: contentsView.gutterWidth - contentsView.lineNumberColumnLeft - contentsView.lineNumberRightInset")),
        "ContentViewLayout.qml must widen the gutter text column up to the mirrored editor inset.");
    QVERIFY2(
        gutterLayerText.contains(QStringLiteral("x: gutterLayer.lineNumberColumnLeft")),
        "ContentsGutterLayer.qml must keep the gutter number column anchored from the left rail origin.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("model: contentsView.effectiveGutterMarkers")) ||
        gutterLayerText.contains(QStringLiteral("model: gutterLayer.effectiveGutterMarkers")),
        "ContentsDisplayView.qml or ContentsGutterLayer.qml must render the effective gutter marker model instead of decorative placeholders.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("LV.TextEditor {")),
        "ContentViewLayout.qml must compose LV.TextEditor for the contents editing surface.");
    QVERIFY2(
        !contentViewText.contains(QStringLiteral("text: \"No files in this folder\"")),
        "ContentsDisplayView.qml must not fabricate an empty-folder message when nothing is selected; the center surface should stay blank.");
    QVERIFY2(
        !contentViewText.contains(QStringLiteral("text: \"Select a note to view its body text\"")),
        "ContentsDisplayView.qml must not keep a synthetic no-selection editor prompt; no selected note must leave the editor surface empty.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("showRenderedOutput: false")),
        "ContentViewLayout.qml must disable TextEditor preview output inside the contents display surface.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("enforceModeDefaults: false")),
        "ContentViewLayout.qml must opt out of TextEditor forced defaults so wrapped logical-line gutter mapping can be set.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("wrapMode: TextEdit.Wrap")),
        "ContentViewLayout.qml must soft-wrap the editor while keeping gutter numbers on logical lines.");
    QVERIFY2(
        contentViewSessionText.contains(QStringLiteral("function scheduleEditorPersistence()")),
        "ContentsEditorSession.qml must debounce body persistence instead of rewriting files on every keystroke.");
    QVERIFY2(
        contentViewSessionText.contains(QStringLiteral("selectionBridge.persistEditorTextForNote(noteId,")),
        "ContentsEditorSession.qml must persist body edits against an explicit note id so selection changes do not misroute pending saves.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("Component.onDestruction: editorSession.flushPendingEditorText()")),
        "ContentViewLayout.qml must flush pending body edits through the dedicated editor session when the surface is torn down.");
    QVERIFY2(
        contentViewSessionText.contains(QStringLiteral("Timer {")),
        "ContentsEditorSession.qml must use a Timer-backed debounce for body persistence.");
    QVERIFY2(
        contentViewSessionText.contains(QStringLiteral("if (!bodySaveTimer.running)")) &&
        contentViewSessionText.contains(QStringLiteral("bodySaveTimer.start();")),
        "ContentsEditorSession.qml must keep a running save timer so continuous typing still flushes pending edits to disk.");
    QVERIFY2(
        contentViewSessionText.contains(QStringLiteral("repeat: true")),
        "ContentsEditorSession.qml save timer must repeat so long typing sessions persist through the active edit stream.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("id: gutterRefreshTimer")),
        "ContentViewLayout.qml must use a dedicated timer to resample gutter layout after note changes and resurfacing.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("contentsView.scheduleGutterRefresh(4);")),
        "ContentViewLayout.qml must trigger extra gutter refresh passes when a note body is rebound or the surface is shown.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function onContentHeightChanged()")),
        "ContentViewLayout.qml must resample the gutter when the internal TextEditor content height changes after layout settles.");
    QVERIFY2(
        contentViewText.contains(
            QStringLiteral("readonly property var editorFlickable: contentsView.resolveEditorFlickable()")),
        "ContentViewLayout.qml must resolve the internal TextEditor flickable for minimap scrolling.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function resolveEditorFlickable()")),
        "ContentViewLayout.qml must expose a helper that resolves the internal editor flickable.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("return candidate;")),
        "ContentViewLayout.qml must return the resolved internal editor flickable so viewport-derived gutter lookups stay live.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function scrollEditorViewportToMinimapPosition(localY)")),
        "ContentViewLayout.qml must expose a minimap scroll helper that repositions the editor viewport.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("id: minimapCanvas")) ||
        minimapLayerText.contains(QStringLiteral("id: minimapCanvas")),
        "ContentsDisplayView.qml or ContentsMinimapLayer.qml must render a right-side minimap canvas for the editor document overview.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function minimapContentYForLine(lineNumber)")),
        "ContentViewLayout.qml minimap must project each bar from the editor text start offset rather than gutter state.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function minimapTrackYForContentY(contentY)")),
        "ContentViewLayout.qml minimap must map document content Y into track Y so short notes stay top-aligned instead of filling the whole rail.");
    QVERIFY2(
        contentViewText.contains(
            QStringLiteral("readonly property var minimapVisualRows: contentsView.buildMinimapVisualRows(")),
        "ContentViewLayout.qml minimap must cache visual-row segments from the live editor layout instead of painting one carved block per logical line.");
    QVERIFY2(
        contentViewText.contains(
            QStringLiteral("function buildMinimapVisualRows(text, editorWidth, editorContentHeight)")),
        "ContentViewLayout.qml minimap must derive rows from the editor's actual wrapped visual layout.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function minimapVisualRowPaintHeight(rowSpec)")),
        "ContentViewLayout.qml minimap must thin each painted row below the full slot height so the text silhouette stays crisp.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("readonly property int minimapRowGap: 1")),
        "ContentViewLayout.qml minimap must keep a fixed 1px gap between packed silhouette rows.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function minimapSilhouetteHeight()")),
        "ContentViewLayout.qml minimap must expose a packed silhouette height instead of relying on the whole editor rail height.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral(
            "return visualIndex * (contentsView.minimapRowGap + contentsView.minimapVisualRowPaintHeight(rowSpec));")),
        "ContentViewLayout.qml minimap row Y positions must be packed from visual-row index plus a fixed 1px gap.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("Layout.preferredWidth: contentsView.minimapOuterWidth")),
        "ContentViewLayout.qml minimap rail must reserve its own right-side layout width.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("anchors.rightMargin: contentsView.minimapTrackInset")) ||
        minimapLayerText.contains(QStringLiteral("anchors.rightMargin: minimapLayer.minimapTrackInset")),
        "ContentsDisplayView.qml or ContentsMinimapLayer.qml minimap track must sit inline against the right edge instead of using a framed centered rail.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("border.width: 0")) ||
        minimapLayerText.contains(QStringLiteral("border.width: 0")),
        "ContentsDisplayView.qml or ContentsMinimapLayer.qml minimap viewport must remain borderless.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("color: contentsView.minimapViewportFillColor")) ||
        minimapLayerText.contains(QStringLiteral("color: minimapLayer.minimapViewportFillColor")),
        "ContentsDisplayView.qml or ContentsMinimapLayer.qml minimap viewport must use a subtle inline fill instead of a framed outline.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("width: contentsView.minimapCurrentLineWidth()")) ||
        minimapLayerText.contains(QStringLiteral(
            "width: minimapLayer.minimapCurrentLineWidthResolver ? Number(minimapLayer.minimapCurrentLineWidthResolver()) || 0 : 0")),
        "ContentsDisplayView.qml or ContentsMinimapLayer.qml current-line minimap indicator must follow the active visual-row silhouette width instead of spanning the whole rail.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("LV.WheelScrollGuard {")) ||
        minimapLayerText.contains(QStringLiteral("LV.WheelScrollGuard {")),
        "ContentsDisplayView.qml or ContentsMinimapLayer.qml minimap must route wheel scroll into the editor flickable.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("targetFlickable: contentsView.editorFlickable")) ||
        minimapLayerText.contains(QStringLiteral("targetFlickable: minimapLayer.editorFlickable")),
        "ContentsDisplayView.qml or ContentsMinimapLayer.qml minimap wheel routing must target the resolved editor flickable.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("fontFamily: LV.Theme.fontBody")),
        "ContentViewLayout.qml must bind TextEditor typography to the LVRS body font family.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("fontLetterSpacing: 0")),
        "ContentViewLayout.qml must keep the Figma body token's zero letter spacing.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("selectionColor: LV.Theme.accent")),
        "ContentViewLayout.qml must keep the editor selection highlight aligned with the LVRS input accent color.");
    QVERIFY2(
        contentViewText.contains(
            QStringLiteral("fieldMinHeight: Math.max(contentsView.minEditorHeight, editorViewport.height)")),
        "ContentViewLayout.qml must keep the editor surface on Fill height even when the body text is empty.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("property: \"y\"")),
        "ContentViewLayout.qml must override the internal TextEditor y-position for top-left body alignment.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("readonly property real editorDocumentStartY: {")),
        "ContentsDisplayView.qml must centralize the document origin so desktop and mobile share the same editor geometry contract.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("readonly property int editorTopInset: LV.Theme.gap4")),
        "ContentsDisplayView.qml must keep a shared 16px top inset through the LVRS gap4 token for every platform.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("value: contentsView.editorDocumentStartY")),
        "ContentViewLayout.qml must drive body origin from the shared editorDocumentStartY resolver.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("readonly property int editorDocumentTopPadding: 0")),
        "ContentsDisplayView.qml must neutralize LVRS internal top padding through a shared editorDocumentTopPadding contract.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("property: \"topPadding\"")) &&
            contentViewText.contains(QStringLiteral("value: contentsView.editorDocumentTopPadding")),
        "ContentViewLayout.qml must clear TextEditor topPadding for both desktop and mobile instead of keeping a mobile-only special case.");
    QVERIFY2(
        !contentViewText.contains(QStringLiteral("when: contentsView.effectiveEditorTopInset === 0")),
        "ContentsDisplayView.qml must not keep a mobile-only topPadding branch after the shared editor geometry merge.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("return contentsView.effectiveEditorTopInset;")),
        "ContentsDisplayView.qml shared editor origin must still resolve from the shared top inset unless a route override replaces it.");
    QVERIFY2(
        !contentViewText.contains(QStringLiteral("Number(contentEditor.editorItem.y)")),
        "ContentsDisplayView.qml must not derive the shared editor origin from TextEditor.editorItem.y, because that turns the top inset into a self-referential binding loop and collapses the intended 16px padding.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("text: contentsView.editorText")),
        "ContentViewLayout.qml must bind the editor and gutter to the same document text source.");
    QVERIFY2(
        contentViewText.contains(
            QStringLiteral("property alias syncingEditorTextFromModel: editorSession.syncingEditorTextFromModel")),
        "ContentViewLayout.qml must guard model-driven body sync through the dedicated editor session object.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("selectionBridge: selectionBridge")),
        "ContentsDisplayView.qml must wire the dedicated editor session to the selection/persistence adapter.");
    QVERIFY2(
        contentViewSessionText.contains(QStringLiteral("function syncEditorTextFromSelection(noteId, text)")),
        "ContentsEditorSession.qml must resync selection changes through an explicit editor-sync helper.");
    QVERIFY2(
        contentViewSessionText.contains(QStringLiteral("property bool localEditorAuthority: false")),
        "ContentsEditorSession.qml must track whether the current note is locally authoritative.");
    QVERIFY2(
        contentViewSessionText.contains(QStringLiteral("function shouldAcceptModelBodyText(noteId, text)")),
        "ContentsEditorSession.qml must gate incoming body syncs through a local-authority helper.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("editorSession.scheduleEditorPersistence();")),
        "ContentsDisplayView.qml must delegate save scheduling through the dedicated editor session.");
    QVERIFY2(
        contentViewText.contains(
            QStringLiteral("readonly property string selectedNoteBodyText: selectionBridge.selectedNoteBodyText")),
        "ContentsDisplayView.qml must source selected note text from the selection adapter.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("onSelectedNoteBodyTextChanged: {")),
        "ContentViewLayout.qml must resync editor text when the selected note body changes.");
    const QString selectedNoteBodyChangedBody = extractFunctionBody(
        contentViewText, QStringLiteral("onSelectedNoteBodyTextChanged:"));
    QVERIFY2(
        selectedNoteBodyChangedBody.contains(
            QStringLiteral("editorSession.shouldAcceptModelBodyText(contentsView.selectedNoteId, contentsView.selectedNoteBodyText)")),
        "ContentsDisplayView.qml must gate same-note model reloads through the local-authority session helper.");
    QVERIFY2(
        selectedNoteBodyChangedBody.contains(
            QStringLiteral(
                "editorSession.syncEditorTextFromSelection(contentsView.selectedNoteId, contentsView.selectedNoteBodyText);")),
        "ContentViewLayout.qml must sync incoming note bodies through the guarded editor session.");
    QVERIFY2(
        selectedNoteBodyChangedBody.contains(QStringLiteral("editorSession.scheduleEditorPersistence();")),
        "ContentsDisplayView.qml must reassert the local editor body when storage pushes a divergent same-note payload.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("onSelectedNoteIdChanged: {")),
        "ContentViewLayout.qml must react explicitly when the selected note id changes.");
    const QString selectedNoteIdChangedBody = extractFunctionBody(
        contentViewText, QStringLiteral("onSelectedNoteIdChanged:"));
    QVERIFY2(
        selectedNoteIdChangedBody.contains(QStringLiteral(
            "editorSession.syncEditorTextFromSelection(contentsView.selectedNoteId, contentsView.selectedNoteBodyText);")),
        "ContentViewLayout.qml must resync editor text on note-id changes so switching between empty notes does not keep stale editor content.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("if (contentsView.syncingEditorTextFromModel)")),
        "ContentViewLayout.qml must skip file persistence while replaying model-driven editor text.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("editorSession.scheduleEditorPersistence();")),
        "ContentViewLayout.qml must debounce persistence for user edits through the dedicated editor session.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("editorSession.markLocalEditorAuthority();")),
        "ContentsDisplayView.qml must mark user edits as locally authoritative before persisting them.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("contentsView.editorTextEdited(text);")),
        "ContentViewLayout.qml must emit editorTextEdited(text) from TextEditor edits.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("anchors.topMargin: contentsView.editorDocumentStartY")),
        "ContentViewLayout.qml placeholder label must align to the shared document origin.");
}

void QmlBindingSyntaxGuardTest::hierarchySidebarWiring_mustBindLoaderAndToolbarTarget()
{
    const QDir testsDir(QStringLiteral(QT_TESTCASE_SOURCEDIR));
    const QString qmlRoot = testsDir.absoluteFilePath(QStringLiteral("../src/app/qml"));

    const QString sidebarLayoutPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/HierarchySidebarLayout.qml"));
    QFile sidebarLayoutFile(sidebarLayoutPath);
    QVERIFY2(sidebarLayoutFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(sidebarLayoutPath));
    const QString sidebarLayoutText = QString::fromUtf8(sidebarLayoutFile.readAll());

    QVERIFY2(
        sidebarLayoutText.contains(QStringLiteral("required property var sidebarHierarchyViewModel")),
        "HierarchySidebarLayout.qml must require the shared sidebar hierarchy state manager.");
    QVERIFY2(
        sidebarLayoutText.contains(
            QStringLiteral(
                "readonly property int currentHierarchy: hierarchyView.sidebarHierarchyViewModel ? hierarchyView.sidebarHierarchyViewModel.resolvedActiveHierarchyIndex : 0")),
        "HierarchySidebarLayout.qml must source the active hierarchy index directly from SidebarHierarchyViewModel.");
    QVERIFY2(
        sidebarLayoutText.contains(
            QStringLiteral(
                "readonly property var resolvedHierarchyViewModel: hierarchyView.sidebarHierarchyViewModel ? hierarchyView.sidebarHierarchyViewModel.resolvedHierarchyViewModel : null")),
        "HierarchySidebarLayout.qml must source the active hierarchy view-model directly from SidebarHierarchyViewModel.");
    QVERIFY2(
        sidebarLayoutText.contains(QStringLiteral("readonly property var noteDropTargetView: sidebarView")),
        "HierarchySidebarLayout.qml must expose the composed SidebarHierarchyView as the canonical cross-panel note-drop target.");
    QVERIFY2(
        sidebarLayoutText.contains(QStringLiteral("function setActiveHierarchyIndex(index)")),
        "HierarchySidebarLayout.qml must centralize toolbar-driven hierarchy activation through one setter helper.");
    QVERIFY2(
        sidebarLayoutText.contains(
            QStringLiteral("hierarchyView.sidebarHierarchyViewModel.setActiveHierarchyIndex(index);")),
        "HierarchySidebarLayout.qml must delegate hierarchy activation directly to SidebarHierarchyViewModel.");
    QVERIFY2(
        sidebarLayoutText.contains(QStringLiteral("activeToolbarIndex: hierarchyView.currentHierarchy")),
        "HierarchySidebarLayout.qml must bind sidebar activeToolbarIndex from currentHierarchy.");
    QVERIFY2(
        sidebarLayoutText.contains(QStringLiteral("hierarchyViewModel: hierarchyView.resolvedHierarchyViewModel")),
        "HierarchySidebarLayout.qml must pass only the resolved hierarchy view-model into SidebarHierarchyView.");
    QVERIFY2(
        sidebarLayoutText.contains(
            QStringLiteral("bookmarkPaletteVisualsEnabled: hierarchyView.currentHierarchy === hierarchyEnum.bookmarks")),
        "HierarchySidebarLayout.qml must enable the bookmark-specific visual palette only for the bookmarks domain.");
    QVERIFY2(
        sidebarLayoutText.contains(QStringLiteral("hierarchyView.setActiveHierarchyIndex(index);")),
        "HierarchySidebarLayout.qml must route toolbar index updates through the normalized setter helper.");
    QVERIFY2(
        sidebarLayoutText.contains(QStringLiteral("signal hierarchyItemActivated(var item, int itemId, int index)")),
        "HierarchySidebarLayout.qml must expose a dedicated hierarchyItemActivated signal for routed mobile page transitions.");

    const QString mainQmlPath = QDir(qmlRoot).absoluteFilePath(QStringLiteral("Main.qml"));
    QFile mainQmlFile(mainQmlPath);
    QVERIFY2(mainQmlFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(mainQmlPath));
    const QString mainQmlText = QString::fromUtf8(mainQmlFile.readAll());

    QVERIFY2(
        mainQmlText.contains(QStringLiteral("readonly property string workspaceRoutePath: \"/\"")),
        "Main.qml must declare an explicit workspace route path for the LVRS page stack.");
    QVERIFY2(
        mainQmlText.contains(QStringLiteral("useInternalPageStack: true")),
        "Main.qml must enable the LVRS internal page stack instead of bypassing ApplicationWindow routing.");
    QVERIFY2(
        mainQmlText.contains(QStringLiteral("internalRouterRegisterAsGlobalNavigator: true")),
        "Main.qml must register its internal LVRS router as the active global navigator.");
    QVERIFY2(
        mainQmlText.contains(QStringLiteral("initialRoutePath: startupRoutePath")),
        "Main.qml must drive initial shell state through LVRS initialRoutePath.");
    QVERIFY2(
        mainQmlText.contains(QStringLiteral("pageInitialPath: startupRoutePath")),
        "Main.qml must mirror the first-frame route seed through pageInitialPath for deterministic mobile bootstrap.");
    QVERIFY2(
        mainQmlText.contains(QStringLiteral("pageRoutes: [onboardingRoute, workspaceShellRoute]")),
        "Main.qml must expose both onboarding and workspace shell routes through pageRoutes.");
    QVERIFY2(
        mainQmlText.contains(QStringLiteral(
            "sourceComponent: applicationWindow.adaptiveMobileLayout ? mobileMainLayoutComponent : desktopMainLayoutComponent")),
        "Main.qml must choose the routed workspace layout from LVRS adaptiveMobileLayout instead of a root Loader-only branch.");
    QVERIFY2(
        !mainQmlText.contains(QStringLiteral("readonly property string activeMainLayout")),
        "Main.qml must not keep the legacy activeMainLayout branch contract once LVRS page-stack routing is enabled.");
    QVERIFY2(
        !mainQmlText.contains(QStringLiteral("id: mainLayoutLoader")),
        "Main.qml must not bypass the routed shell through a root-level mainLayoutLoader.");
    QVERIFY2(
        mainQmlText.contains(QStringLiteral("readonly property var rootSidebarHierarchyViewModel: sidebarHierarchyViewModel")) &&
            mainQmlText.contains(QStringLiteral("sidebarHierarchyViewModel: applicationWindow.rootSidebarHierarchyViewModel")),
        "Main.qml must forward sidebarHierarchyViewModel to BodyLayout through a distinct root alias.");
    QVERIFY2(
        mainQmlText.contains(QStringLiteral("readonly property var rootLibraryHierarchyViewModel: libraryHierarchyViewModel")) &&
            mainQmlText.contains(QStringLiteral("noteDeletionViewModel: applicationWindow.rootLibraryHierarchyViewModel")),
        "Main.qml must inject the centralized library delete-note command source into BodyLayout through a distinct root alias.");
    QVERIFY2(
        mainQmlText.contains(QStringLiteral("MainWindowInteractionController {")),
        "Main.qml must delegate root interaction policy to a dedicated MainWindowInteractionController.");
    QVERIFY2(
        mainQmlText.contains(QStringLiteral("function showOnboardingWindow()")),
        "Main.qml must expose a dedicated helper that opens and foregrounds the onboarding subwindow from shell-level actions.");
    QVERIFY2(
        mainQmlText.contains(QStringLiteral("onboardingSubWindow.raise();")) &&
        mainQmlText.contains(QStringLiteral("onboardingSubWindow.requestActivate();")),
        "Main.qml onboarding helper must foreground the existing onboarding subwindow when the user reopens it from the global menu bar.");

    const QString nativeMenuBarPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("window/MacNativeMenuBar.qml"));
    QFile nativeMenuBarFile(nativeMenuBarPath);
    QVERIFY2(nativeMenuBarFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(nativeMenuBarPath));
    const QString nativeMenuBarText = QString::fromUtf8(nativeMenuBarFile.readAll());
    QVERIFY2(
        mainQmlText.contains(QStringLiteral("Qt.resolvedUrl(\"window/MacNativeMenuBar.qml\")")) &&
        mainQmlText.contains(QStringLiteral("item.hostWindow = applicationWindow")) &&
        nativeMenuBarText.contains(QStringLiteral("title: qsTr(\"Window\")")) &&
        nativeMenuBarText.contains(QStringLiteral("text: qsTr(\"Onboarding\")")) &&
        nativeMenuBarText.contains(QStringLiteral("root.hostWindow.showOnboardingWindow();")),
        "MacNativeMenuBar.qml must stay lazily loaded from Main.qml and expose an Onboarding Window-menu action wired to the root onboarding helper.");

    const QString sidebarViewPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/sidebar/SidebarHierarchyView.qml"));
    QFile sidebarViewFile(sidebarViewPath);
    QVERIFY2(sidebarViewFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(sidebarViewPath));
    const QString sidebarViewText = QString::fromUtf8(sidebarViewFile.readAll());
    const QString hierarchySidebarLayoutPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/HierarchySidebarLayout.qml"));
    QFile hierarchySidebarLayoutFile(hierarchySidebarLayoutPath);
    QVERIFY2(
        hierarchySidebarLayoutFile.open(QIODevice::ReadOnly | QIODevice::Text),
        qPrintable(hierarchySidebarLayoutPath));
    const QString hierarchySidebarLayoutText = QString::fromUtf8(hierarchySidebarLayoutFile.readAll());
    QVERIFY2(
        !sidebarViewText.contains(QStringLiteral("import WhatSon.App.Internal 1.0")),
        "SidebarHierarchyView.qml must no longer depend on an internal hierarchy adapter module.");
    QVERIFY2(
        !sidebarViewText.contains(QStringLiteral("sidebarHierarchyView.activeToolbarIndex = index;")),
        "SidebarHierarchyView.qml must not overwrite activeToolbarIndex locally.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function syncSelectedHierarchyItem(focusView)")),
        "SidebarHierarchyView.qml must expose syncSelectedHierarchyItem(focusView) for programmatic focus sync.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("hierarchyTree.activateListItemById(selectedFolderIndex);")),
        "SidebarHierarchyView.qml must drive programmatic hierarchy activation through LVRS activateListItemById(...).");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("LV.Hierarchy {")),
        "SidebarHierarchyView.qml must render folders through the LVRS Hierarchy surface.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("model: sidebarHierarchyView.standardHierarchyModel")),
        "SidebarHierarchyView.qml must feed LVRS Hierarchy from the active hierarchy view-model's standard model property.");
    QVERIFY2(
        sidebarViewText.contains(
            QStringLiteral("readonly property bool activeToolbar: index === sidebarHierarchyView.activeToolbarIndex")),
        "SidebarHierarchyView.qml custom toolbar row must keep active state sourced from the shared sidebar selection model.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("onClicked: {")),
        "SidebarHierarchyView.qml must switch hierarchy domains from the fixed toolbar row button callback.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("sidebarHierarchyView.toolbarIndexChangeRequested(index);")),
        "SidebarHierarchyView.qml toolbar activation must flow back through the shared domain-switch signal.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("onListItemActivated: function (item, itemId, index)")),
        "SidebarHierarchyView.qml must sync hierarchy selection from the LVRS listItemActivated callback.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("sidebarHierarchyView.hierarchyViewModel.setSelectedIndex(itemId);")),
        "SidebarHierarchyView.qml must mirror LVRS item activation into the bound hierarchy view-model by stable itemId.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("signal hierarchyItemActivated(var item, int itemId, int index)")) &&
            sidebarViewText.contains(QStringLiteral("sidebarHierarchyView.hierarchyItemActivated(item, itemId, index);")),
        "SidebarHierarchyView.qml must re-emit hierarchy row activation after syncing selection so mobile routing can subscribe without bypassing the bound view-model.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function normalizeHierarchyModel(modelValue)")),
        "SidebarHierarchyView.qml must normalize C++ hierarchy QVariantList payloads into a real JS array before handing them to LVRS editable drag logic.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function projectedHierarchyModel(modelValue)")) &&
        sidebarViewText.contains(QStringLiteral("projectedItem.label = \"\";")) &&
        sidebarViewText.contains(QStringLiteral(
            "readonly property var standardHierarchyModel: sidebarHierarchyView.projectedHierarchyModel(hierarchyViewModel && hierarchyViewModel.hierarchyModel !== undefined ? hierarchyViewModel.hierarchyModel : [])")),
        "SidebarHierarchyView.qml must project the direct hierarchy model through a rename-safe view that blanks the edited row label while inline rename is active.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("property var hierarchyDragDropBridge: null")),
        "SidebarHierarchyView.qml must accept a dedicated hierarchy drag/drop bridge instead of embedding local reorder state.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("property bool hierarchyEditable: false")),
        "SidebarHierarchyView.qml must expose an explicit editability contract for the LVRS hierarchy surface.");
    QVERIFY2(
        sidebarViewText.contains(
            QStringLiteral(
                "readonly property bool createFolderContractAvailable: hierarchyViewModel && hierarchyViewModel.createFolder !== undefined")),
        "SidebarHierarchyView.qml must detect create-folder support from the bound hierarchy view-model.");
    QVERIFY2(
        sidebarViewText.contains(
            QStringLiteral(
                "readonly property bool setItemExpandedContractAvailable: hierarchyViewModel && hierarchyViewModel.setItemExpanded !== undefined")),
        "SidebarHierarchyView.qml must detect the optional targeted expansion-state contract before mutating any hierarchy expansion data.");
    QVERIFY2(
        sidebarViewText.contains(
            QStringLiteral(
                "readonly property bool deleteFolderContractAvailable: hierarchyViewModel && hierarchyViewModel.deleteSelectedFolder !== undefined")),
        "SidebarHierarchyView.qml must detect delete-folder support from the bound hierarchy view-model.");
    QVERIFY2(
        sidebarViewText.contains(
            QStringLiteral(
                "readonly property bool renameContractAvailable: hierarchyViewModel && hierarchyViewModel.canRenameItem !== undefined && hierarchyViewModel.renameItem !== undefined")),
        "SidebarHierarchyView.qml must detect rename support from the bound hierarchy view-model before exposing inline edit actions.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("property int editingHierarchyIndex: -1")),
        "SidebarHierarchyView.qml must keep explicit inline-rename state for the currently edited hierarchy row.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("property string editingHierarchyLabel: \"\"")),
        "SidebarHierarchyView.qml must store the transient rename text separately from the model snapshot.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function beginRenameSelectedHierarchyItem()")),
        "SidebarHierarchyView.qml must expose a keyboard-driven entrypoint for inline hierarchy rename.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function commitHierarchyRename()")),
        "SidebarHierarchyView.qml must centralize hierarchy rename commits through a dedicated helper.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function cancelHierarchyRename()")),
        "SidebarHierarchyView.qml must centralize inline rename cancellation so selection and toolbar actions can unwind the editor cleanly.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("Keys.onEnterPressed: function (event)")) &&
        sidebarViewText.contains(QStringLiteral("Keys.onReturnPressed: function (event)")),
        "SidebarHierarchyView.qml must enter inline hierarchy rename when the active sidebar surface receives Enter or Return.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("LV.InputField {")) &&
        sidebarViewText.contains(QStringLiteral("id: hierarchyRenameField")),
        "SidebarHierarchyView.qml must render inline rename through an LVRS InputField overlay instead of a raw TextInput.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("readonly property color hierarchyRenameFieldBackgroundColor: {")) &&
        sidebarViewText.contains(
            QStringLiteral("backgroundColor: sidebarHierarchyView.hierarchyRenameFieldBackgroundColor")) &&
        sidebarViewText.contains(
            QStringLiteral("backgroundColorFocused: sidebarHierarchyView.hierarchyRenameFieldBackgroundColor")),
        "SidebarHierarchyView.qml inline rename field must inherit the active row background so the original label never bleeds through the overlay.");
    QVERIFY2(
        sidebarViewText.contains(
            QStringLiteral("sidebarHierarchyView.hierarchyViewModel.renameItem(renameIndex, nextLabel)")),
        "SidebarHierarchyView.qml must commit inline rename through the bound hierarchy view-model renameItem contract.");
    QVERIFY2(
        sidebarViewText.contains(
            QStringLiteral(
                "readonly property bool viewOptionsEnabled: hierarchyViewModel && hierarchyViewModel.viewOptionsEnabled !== undefined ? Boolean(hierarchyViewModel.viewOptionsEnabled) : true")),
        "SidebarHierarchyView.qml must expose a view-options capability contract for the footer menu button.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("property int toolbarFrameWidth: 200")),
        "SidebarHierarchyView.qml hierarchy toolbar must keep the 200px Figma track width.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral(
            "readonly property real toolbarButtonSpacing: sidebarHierarchyView.toolbarItems.length > 1 ? (sidebarHierarchyView.toolbarFrameWidth - sidebarHierarchyView.toolbarButtonSize * sidebarHierarchyView.toolbarItems.length) / (sidebarHierarchyView.toolbarItems.length - 1) : 0")),
        "SidebarHierarchyView.qml hierarchy toolbar must derive a fixed left-anchored spacing from the 200px Figma track.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("editable: sidebarHierarchyView.hierarchyEditable")),
        "SidebarHierarchyView.qml must source LVRS editable drag behavior from the resolved hierarchy drag/drop contract.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("toolbarItems: []")),
        "SidebarHierarchyView.qml must disable the built-in LVRS toolbar items when mounting the fixed Figma toolbar row.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("searchFieldShapeStyle: hierarchySearchHeader.shapeRoundRect")),
        "SidebarHierarchyView.qml must override the shared search header to LVRS shapeRoundRect so hierarchy search stays rounded instead of pill-shaped.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("outerHorizontalInset: LV.Theme.gapNone")),
        "SidebarHierarchyView.qml must source hierarchy-search horizontal padding from the list anchor inset instead of keeping a second wrapper inset.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("Row {")),
        "SidebarHierarchyView.qml must compose a dedicated fixed toolbar row for the Figma header icons.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("spacing: sidebarHierarchyView.toolbarButtonSpacing")),
        "SidebarHierarchyView.qml custom hierarchy toolbar must keep the fixed left-anchored Figma spacing.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("width: sidebarHierarchyView.toolbarFrameWidth")),
        "SidebarHierarchyView.qml custom hierarchy toolbar must keep the Figma 200px track width.");
    QVERIFY2(
        sidebarViewText.contains(
            QStringLiteral("tone: activeToolbar ? LV.AbstractButton.Default : LV.AbstractButton.Borderless")),
        "SidebarHierarchyView.qml custom hierarchy toolbar must preserve LVRS active/default button tones.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("sidebarHierarchyView.toolbarIndexChangeRequested(index);")),
        "SidebarHierarchyView.qml custom hierarchy toolbar must emit toolbar index changes directly from the fixed icon row.");
    QVERIFY2(
        sidebarViewText.contains(
            QStringLiteral(
                "anchors.bottomMargin: sidebarHierarchyView.verticalInset + (sidebarHierarchyView.footerVisible ? hierarchyFooter.implicitHeight : 0)")),
        "SidebarHierarchyView.qml must reserve explicit space above the bottom footer instance.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("LV.ListFooter {")),
        "SidebarHierarchyView.qml must mount the Figma footer as a direct LVRS ListFooter instance.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("id: hierarchyFooter")),
        "SidebarHierarchyView.qml must keep a stable footer instance for bottom inset coordination.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("anchors.leftMargin: sidebarHierarchyView.horizontalInset")),
        "SidebarHierarchyView.qml footer must align to the shared sidebar inset.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("anchors.bottomMargin: sidebarHierarchyView.verticalInset")),
        "SidebarHierarchyView.qml footer must keep the Figma 2px bottom inset.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("width: 78")),
        "SidebarHierarchyView.qml footer must keep the Figma 78px footer width.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("height: 24")),
        "SidebarHierarchyView.qml footer must keep the Figma 24px footer height.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("iconName: \"generaladd\"")),
        "SidebarHierarchyView.qml footer must restore the add button defined by the Figma footer node.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("iconName: \"generaldelete\"")),
        "SidebarHierarchyView.qml footer must restore the delete button defined by the Figma footer node.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("iconName: \"generalsettings\"")),
        "SidebarHierarchyView.qml footer must restore the settings menu button defined by the Figma footer node.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("type: \"menu\"")),
        "SidebarHierarchyView.qml footer must keep the third button as a menu-style action.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("backgroundColor: \"transparent\"")),
        "SidebarHierarchyView.qml footer buttons must not introduce non-Figma background fills.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("leftPadding: 2")),
        "SidebarHierarchyView.qml footer menu button must keep the Figma 2px left padding.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("rightPadding: 4")),
        "SidebarHierarchyView.qml footer menu button must keep the Figma 4px right padding.");
    QVERIFY2(
        !sidebarViewText.contains(QStringLiteral("spacing: -4")),
        "SidebarHierarchyView.qml footer menu button must not override the LVRS icon-chevron spacing contract.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("sidebarHierarchyView.requestCreateFolder();")),
        "SidebarHierarchyView.qml footer add button must route into the shared create-folder helper.");
    QVERIFY2(
        sidebarViewText.contains(
            QStringLiteral("const activeHierarchyItemId = Number(hierarchyTree.activeListItemId);")) &&
        sidebarViewText.contains(QStringLiteral(
            "sidebarHierarchyView.hierarchyViewModel.setSelectedIndex(Math.floor(activeHierarchyItemId));")),
        "SidebarHierarchyView.qml create-folder flow must snapshot the visually active LVRS row before mutating the hierarchy so new folders always attach to the active parent.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("sidebarHierarchyView.requestDeleteFolder();")),
        "SidebarHierarchyView.qml footer delete button must route into the shared delete-folder helper.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("onListItemExpanded: function (item, itemId, index, expanded)")) &&
        sidebarViewText.contains(
            QStringLiteral("sidebarHierarchyView.hierarchyViewModel.setItemExpanded(itemId, expanded);")),
        "SidebarHierarchyView.qml must sync user-triggered expansion changes through the targeted setItemExpanded contract instead of rebuilding unrelated hierarchy state.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("sequence: \"Escape\"")) &&
        sidebarViewText.contains(QStringLiteral("sidebarHierarchyView.cancelHierarchyRename();")),
        "SidebarHierarchyView.qml must provide an Escape path that exits inline rename without mutating unrelated sidebar state.");
    QVERIFY2(
        sidebarViewText.contains(
            QStringLiteral("onListItemMoved: function (item, itemId, itemKey, fromIndex, toIndex, depth)")),
        "SidebarHierarchyView.qml must listen to LVRS listItemMoved so drag reorder events can be persisted.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral(
            "sidebarHierarchyView.hierarchyDragDropBridge.applyHierarchyReorder(hierarchyTree.model, itemKey)")),
        "SidebarHierarchyView.qml must persist LVRS drag reorder results through the dedicated hierarchy drag/drop bridge.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("DropArea {")),
        "SidebarHierarchyView.qml must expose a note-drop target surface over the LVRS hierarchy.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function updateAcceptance(drag)")),
        "SidebarHierarchyView.qml note-drop surface must gate acceptance through a dedicated payload-validation helper instead of static drag-key filtering.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("property int noteDropHoverIndex: -1")) &&
            sidebarViewText.contains(QStringLiteral("readonly property var noteDropHoverItem: sidebarHierarchyView.hierarchyItemForResolvedIndex(sidebarHierarchyView.noteDropHoverIndex)")),
        "SidebarHierarchyView.qml must track a hovered note-drop row so folder assignment can preview the active destination before release.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("property bool bookmarkPaletteVisualsEnabled: false")) &&
            sidebarViewText.contains(QStringLiteral("function bookmarkPaletteColorTokenForLabel(label)")) &&
            sidebarViewText.contains(QStringLiteral("function applyBookmarkPaletteVisuals()")),
        "SidebarHierarchyView.qml must expose a bookmark-specific color/icon override path for the full bookmarks hierarchy.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("id: bookmarkPaletteIconOverlay")) &&
            sidebarViewText.contains(QStringLiteral("function drawBookmarkGlyph(context, x, y, size, color)")) &&
            sidebarViewText.contains(QStringLiteral("item.iconGlyph = \" \"")) &&
            sidebarViewText.contains(QStringLiteral("item.iconPlaceholderColor = \"transparent\"")),
        "SidebarHierarchyView.qml must draw bookmark icons through the overlay canvas and clear the default placeholder when the bookmarks palette is active.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function clearNoteDropPreview()")) &&
            sidebarViewText.contains(QStringLiteral("function updateNoteDropPreviewAtPosition(x, y, noteId, referenceItem)")),
        "SidebarHierarchyView.qml must expose explicit note-drop preview helpers so DropArea traffic and desktop internal drags share the same hover-highlight path.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("id: noteDropHoverOverlay")) &&
            sidebarViewText.contains(QStringLiteral("SequentialAnimation on opacity {")),
        "SidebarHierarchyView.qml must render a pulsing hover overlay over the target hierarchy row while a note card is dragged across folders.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function canAcceptNoteDropAtPosition(x, y, noteId, referenceItem)")),
        "SidebarHierarchyView.qml must expose a position-based note-drop acceptance helper so both DropArea traffic and desktop internal drags share one validation path.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function commitNoteDropAtPosition(x, y, noteId, referenceItem)")),
        "SidebarHierarchyView.qml must expose a position-based note-drop commit helper so desktop internal drags can persist folder assignment without relying on native drop delivery.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function noteDropTargetAtPosition(x, y, referenceItem)")),
        "SidebarHierarchyView.qml must normalize hovered note-drop targets through a shared descriptor helper so preview and commit stay on the same hierarchy row.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function hierarchyItemContainsPoint(item, x, y)")),
        "SidebarHierarchyView.qml note-drop hit testing must use explicit hierarchy-row bounds instead of relying on LVRS childAt traversal through overlay guards.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("const children = item.children;")),
        "SidebarHierarchyView.qml note-drop hit testing must traverse LVRS hierarchy descendants directly so overlay helpers do not swallow folder hit tests.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("hierarchyItem.resolvedItemId")),
        "SidebarHierarchyView.qml note-drop hit testing must fall back to LVRS resolvedItemId when the raw itemId is absent on the hovered hierarchy row.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("hierarchyTree.mapFromItem(referenceItem, localX, localY)")),
        "SidebarHierarchyView.qml note-drop hit testing must normalize foreign-panel pointer coordinates into the LVRS hierarchy coordinate space.");
    QVERIFY2(
        sidebarViewText.contains(
            QStringLiteral("sidebarHierarchyView.hierarchyDragDropBridge.canAcceptNoteDrop(target.index, normalizedNoteId)")),
        "SidebarHierarchyView.qml must validate note drops through the shared hierarchy drag/drop bridge.");
    QVERIFY2(
        sidebarViewText.contains(
            QStringLiteral("sidebarHierarchyView.hierarchyDragDropBridge.assignNoteToFolder(target.index, normalizedNoteId)")),
        "SidebarHierarchyView.qml must persist accepted note drops through the shared hierarchy drag/drop bridge.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("onExited: {")) &&
            sidebarViewText.contains(QStringLiteral("sidebarHierarchyView.clearNoteDropPreview();")),
        "SidebarHierarchyView.qml must clear the hovered note-drop target preview when the drag leaves the hierarchy surface.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function onHierarchyModelChanged()")),
        "SidebarHierarchyView.qml must resync LVRS activation when the active domain view-model publishes a new hierarchy model.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("ignoreUnknownSignals: true")),
        "SidebarHierarchyView.qml must tolerate domains that do not expose extra legacy hierarchy signals.");
    QVERIFY2(
        !sidebarViewText.contains(QStringLiteral("SidebarHierarchyInteractionController {")),
        "SidebarHierarchyView.qml must no longer rely on a local interaction-controller hierarchy engine.");
    QVERIFY2(
        !sidebarViewText.contains(QStringLiteral("HierarchyListCompat")),
        "SidebarHierarchyView.qml must no longer route hierarchy rendering through the local compat list wrapper.");
    QVERIFY2(
        !sidebarViewText.contains(QStringLiteral("SidebarHierarchyLvrsAdapter {")),
        "SidebarHierarchyView.qml must not reintroduce an app-specific LVRS adapter bridge.");
    QVERIFY2(
        !sidebarViewText.contains(QStringLiteral("LV.HierarchyList {")),
        "SidebarHierarchyView.qml must not bypass the higher-level LVRS Hierarchy surface.");
    QVERIFY2(
        !sidebarViewText.contains(QStringLiteral("footer: hierarchyFooter")),
        "SidebarHierarchyView.qml footer must stay implemented through direct LVRS ListFooter wiring rather than the Hierarchy footer alias.");
    QVERIFY2(
        !sidebarViewText.contains(QStringLiteral("property string searchQuery: \"\"")),
        "SidebarHierarchyView.qml standard-contract pass must not keep a local hierarchy search filter bridge.");
    QVERIFY2(
        !sidebarViewText.contains(QStringLiteral("property string noteDropTargetKey: \"\"")),
        "SidebarHierarchyView.qml standard-contract pass must not keep custom note-drop overlay state.");
    QVERIFY2(
        hierarchySidebarLayoutText.contains(QStringLiteral("HierarchyDragDropBridge {")),
        "HierarchySidebarLayout.qml must compose the dedicated system-level hierarchy drag/drop bridge.");
    QVERIFY2(
        hierarchySidebarLayoutText.contains(
            QStringLiteral("hierarchyViewModel: hierarchyView.resolvedHierarchyViewModel")),
        "HierarchySidebarLayout.qml drag/drop bridge must bind to the resolved active hierarchy view-model.");
    QVERIFY2(
        hierarchySidebarLayoutText.contains(
            QStringLiteral("hierarchyEditable: hierarchyDragDropBridge.reorderContractAvailable")),
        "HierarchySidebarLayout.qml must forward editability into SidebarHierarchyView through the bridge capability contract.");
    QVERIFY2(
        hierarchySidebarLayoutText.contains(QStringLiteral("hierarchyDragDropBridge: hierarchyDragDropBridge")),
        "HierarchySidebarLayout.qml must forward the dedicated hierarchy drag/drop bridge into SidebarHierarchyView.");
    QVERIFY2(
        hierarchySidebarLayoutText.contains(QStringLiteral("onHierarchyItemActivated: function (item, itemId, index)")) &&
            hierarchySidebarLayoutText.contains(QStringLiteral("hierarchyView.hierarchyItemActivated(item, itemId, index);")),
        "HierarchySidebarLayout.qml must forward hierarchy row activation directly from SidebarHierarchyView.");

    const QString noteListItemPath = QDir(qmlRoot).absoluteFilePath(QStringLiteral("view/panels/NoteListItem.qml"));
    QFile noteListItemFile(noteListItemPath);
    QVERIFY2(noteListItemFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(noteListItemPath));
    const QString noteListItemText = QString::fromUtf8(noteListItemFile.readAll());
    QVERIFY2(
        noteListItemText.contains(
            QStringLiteral(
                "readonly property var calendarStore: typeof systemCalendarStore !== \"undefined\" ? systemCalendarStore : null")),
        "NoteListItem.qml must resolve the app-level system calendar store when available.");
    QVERIFY2(
        noteListItemText.contains(
            QStringLiteral(
                "readonly property string displayDatePlaceholder: calendarStore && calendarStore.shortDatePlaceholderText !== undefined ? String(calendarStore.shortDatePlaceholderText) : qsTr(\"Date\")")),
        "NoteListItem.qml date placeholder must defer to the system calendar store instead of hardcoding a single region format.");
    QVERIFY2(
        !noteListItemText.contains(QStringLiteral("readonly property string displayDatePlaceholder: \"YYYY-MM-dd\"")),
        "NoteListItem.qml must not hardcode YYYY-MM-dd as the only placeholder date format.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("implicitHeight: noteListItem.image ? 126 : 102")),
        "NoteListItem.qml must expand to the Figma 126px frame in image mode while preserving the 102px text-only contract.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("implicitWidth: 194")),
        "NoteListItem.qml must keep the Figma 194px width.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("anchors.leftMargin: noteListItem.horizontalPadding")),
        "NoteListItem.qml must apply horizontal content padding from the Figma frame.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("anchors.topMargin: noteListItem.verticalPadding")),
        "NoteListItem.qml must apply vertical content padding from the Figma frame.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("readonly property string resolvedDisplayDate: {")),
        "NoteListItem.qml must resolve a visible display-date string.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("property bool active: false")),
        "NoteListItem.qml must expose a dedicated persistent active-state contract separate from transient pointer press.");
    QVERIFY2(
        noteListItemText.contains(
            QStringLiteral("readonly property bool activeState: noteListItem.active")),
        "NoteListItem.qml must treat active state as the committed selection only, so note rows do not look selected before release commits the tap.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("readonly property color pressedCardColor: noteListItem.hoverCardColor")),
        "NoteListItem.qml must keep transient press feedback on a non-active surface so pointer press does not masquerade as committed selection.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral(": noteListItem.pressed")),
        "NoteListItem.qml background resolution must still keep a dedicated pressed branch after the active branch.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("text: noteListItem.resolvedDisplayDate")),
        "NoteListItem.qml date label must render the resolved display date.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("font.pixelSize: noteListItem.primaryTextSize")),
        "NoteListItem.qml primary text must use the dedicated 12px Figma size property.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("font.pixelSize: noteListItem.metadataTextSize")),
        "NoteListItem.qml metadata text must use the dedicated 11px Figma size property.");
    QVERIFY2(
        noteListItemText.contains(
            QStringLiteral("readonly property var visibleFolders: metadataPreview(noteListItem.folders, true)")),
        "NoteListItem.qml folder metadata preview must collapse hierarchy paths to their final child labels.");
    QVERIFY2(
        noteListItemText.contains(
            QStringLiteral("readonly property var visibleTags: metadataPreview(noteListItem.tags, false)")),
        "NoteListItem.qml tag metadata preview must keep the non-folder metadata path untouched.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("function leafFolderLabel(value) {")),
        "NoteListItem.qml must normalize folder labels through a dedicated leaf-folder helper.");
    QVERIFY2(
        noteListItemText.contains(
            QStringLiteral(
                "var entry = leafOnly ? leafFolderLabel(sourceValues[i]) : String(sourceValues[i]).trim();")),
        "NoteListItem.qml metadata preview must only strip hierarchy depth for folder labels.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("property bool image: false")),
        "NoteListItem.qml must expose the image flag from the note-list model.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("property url imageSource: \"\"")),
        "NoteListItem.qml must expose the first resource thumbnail source as a url property.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("readonly property int imagePreviewSize: 48")),
        "NoteListItem.qml image mode must use the Figma 48px preview frame.");
    QVERIFY2(
        noteListItemText.contains(
            QStringLiteral("Layout.preferredWidth: noteListItem.image ? noteListItem.imagePreviewSize : 0")),
        "NoteListItem.qml imageBox width must only reserve the 48px Figma slot when image=true.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("Layout.alignment: Qt.AlignLeft | Qt.AlignTop")),
        "NoteListItem.qml image-mode title block must stay anchored to the top-left of the row.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("horizontalAlignment: Text.AlignLeft")),
        "NoteListItem.qml title text must keep explicit left alignment in the image layout variant.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("visible: noteListItem.image")),
        "NoteListItem.qml must only show the imageBox when the image flag is true.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("source: noteListItem.imageSource")),
        "NoteListItem.qml imageBox preview must bind through source: noteListItem.imageSource.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("source: noteListItem.folderIconSource")),
        "NoteListItem.qml folder icon must bind through source: noteListItem.folderIconSource.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("source: noteListItem.tagIconSource")),
        "NoteListItem.qml tag icon must bind through source: noteListItem.tagIconSource.");
    QVERIFY2(
        !noteListItemText.contains(QStringLiteral("noteListItem: folderIconSource")),
        "NoteListItem.qml must not use the invalid noteListItem: folderIconSource binding.");
    QVERIFY2(
        !noteListItemText.contains(QStringLiteral("noteListItem: tagIconSource")),
        "NoteListItem.qml must not use the invalid noteListItem: tagIconSource binding.");
    QVERIFY2(
        !noteListItemText.contains(QStringLiteral("visible: noteListItem.displayDate.length > 0")),
        "NoteListItem.qml must not collapse the Figma date row when displayDate is empty.");

    const QString listBarLayoutPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/ListBarLayout.qml"));
    QFile listBarLayoutFile(listBarLayoutPath);
    QVERIFY2(listBarLayoutFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(listBarLayoutPath));
    const QString listBarLayoutText = QString::fromUtf8(listBarLayoutFile.readAll());
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("ListBarHeader {")),
        "ListBarLayout.qml must compose the dedicated ListBarHeader Figma frame.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("function applySearchTextToModel()")),
        "ListBarLayout.qml must centralize note-list search propagation through applySearchTextToModel().");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral(
            "readonly property bool hasNoteListModel: listBarLayout.noteListModel !== null && listBarLayout.noteListModel !== undefined")),
        "ListBarLayout.qml must expose an explicit note-list model presence contract.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral(
            "readonly property bool noteListSearchContractAvailable: listBarLayout.hasNoteListModel && (listBarLayout.noteListModel.searchText !== undefined || listBarLayout.noteListModel.setSearchText !== undefined)")),
        "ListBarLayout.qml must expose a dedicated search-contract capability flag instead of scattering dynamic checks.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral(
            "readonly property bool noteListCurrentIndexContractAvailable: listBarLayout.hasNoteListModel && (listBarLayout.noteListModel.currentIndex !== undefined || listBarLayout.noteListModel.setCurrentIndex !== undefined)")),
        "ListBarLayout.qml must expose a dedicated selection-contract capability flag instead of scattering dynamic checks.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral(
            "readonly property bool noteDeletionContractAvailable: noteDeletionBridge.deleteContractAvailable && noteDeletionBridge.focusedNoteAvailable")),
        "ListBarLayout.qml must expose an explicit delete-note capability contract through FocusedNoteDeletionBridge instead of probing the destructive command inline.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("property string contextMenuNoteId: \"\"")),
        "ListBarLayout.qml must track the note id targeted by the note-card context menu independently from committed selection state.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral(
            "readonly property bool noteFolderClearContractAvailable: listBarLayout.noteDeletionViewModel !== null")),
        "ListBarLayout.qml must expose an explicit clear-folder capability contract before invoking note folder removal from the shared context menu.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("readonly property var noteContextMenuItems: [")),
        "ListBarLayout.qml must centralize note-card context menu entries at the root so delegates only forward the hovered note id and anchor position.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("FocusedNoteDeletionBridge {")),
        "ListBarLayout.qml must compose FocusedNoteDeletionBridge so delete-key handling tracks the visually focused note directly.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("deletionTarget: listBarLayout.noteDeletionViewModel")),
        "ListBarLayout.qml delete bridge must bind to the injected delete-note command source.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("noteListModel: listBarLayout.resolvedNoteListModel")),
        "ListBarLayout.qml delete bridge must retain the resolved note-list model as a fallback focus source.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral(
            "readonly property var resolvedNoteListModel: listBarLayout.noteListMode ? listBarLayout.noteListModel : null")),
        "ListBarLayout.qml must normalize the active note-list model through a resolved contract property.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("function deleteCurrentNote()")),
        "ListBarLayout.qml must centralize current-note deletion through an explicit helper.");
    const QString deleteCurrentNoteBody = extractFunctionBody(
        listBarLayoutText, QStringLiteral("function deleteCurrentNote()"));
    QVERIFY2(
        deleteCurrentNoteBody.contains(QStringLiteral("return deleted;")),
        "ListBarLayout.qml deleteCurrentNote() must return the delete result so keyboard handlers can stop only on successful deletion.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("const deleted = noteDeletionBridge.deleteFocusedNote();")),
        "ListBarLayout.qml must route destructive note deletion through FocusedNoteDeletionBridge.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("function clearContextMenuNoteFolders()")),
        "ListBarLayout.qml must expose a helper that clears all folder assignments for the note targeted by the context menu.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("listBarLayout.noteDeletionViewModel.clearNoteFoldersById(normalizedNoteId)")),
        "ListBarLayout.qml note context-menu folder clearing must call the explicit clearNoteFoldersById() contract on the injected library mutation view-model.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("function deleteContextMenuNote()")),
        "ListBarLayout.qml must expose a helper that deletes the note targeted by the context menu without mutating selection first.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("function openNoteContextMenu(delegateItem, localX, localY)")),
        "ListBarLayout.qml must centralize note-card context-menu opening through a root helper so right-click delegates do not duplicate routing logic.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("noteContextMenu.openFor(delegateItem, Number(localX) || 0, Number(localY) || 0);")),
        "ListBarLayout.qml note-card context menu must open from the clicked delegate's local pointer position.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("if (normalizedNoteId.length > 0 && noteDeletionBridge.focusedNoteId === normalizedNoteId)")),
        "ListBarLayout.qml must release the temporary context-menu focus id when the note-card menu closes so later keyboard deletion still tracks the committed note selection.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("event.key !== Qt.Key_Backspace && event.key !== Qt.Key_Delete")),
        "ListBarLayout.qml must handle both Backspace and Delete when the note list owns keyboard focus.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("noteListView.forceActiveFocus();")),
        "ListBarLayout.qml must return keyboard focus to the note list after tap or delete actions so repeated keyboard deletion keeps working.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("required property string noteId")),
        "ListBarLayout.qml note delegates must declare the note id as an explicit required role contract.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("required property string primaryText")),
        "ListBarLayout.qml note delegates must declare the visible preview text as an explicit required role contract.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("height: noteCard.implicitHeight")),
        "ListBarLayout.qml note delegates must expose the card implicit height on the delegate root so rows stay visible.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("width: ListView.view ? ListView.view.width : listBarLayout.width")),
        "ListBarLayout.qml note delegates must stretch to the active ListView width so the bound NoteListItem can render.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("NoteListItem {")),
        "ListBarLayout.qml note delegates must wrap the visual card in a dedicated NoteListItem composition surface.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("readonly property int committedNoteIndex: listBarLayout.normalizeCurrentIndex(")),
        "ListBarLayout.qml must expose the committed note selection separately from transient ListView currentIndex changes.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("active: listBarLayout.committedNoteIndex === noteItemDelegate.index")),
        "ListBarLayout.qml note delegates must bind persistent card activation only to the committed note-model selection.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral(
            "pressed: listBarLayout.pressedNoteIndex === noteItemDelegate.index || noteItemDelegate.pointerDragActive")),
        "ListBarLayout.qml note delegates must keep transient pressed styling separate from persistent active selection.");
    QVERIFY2(
        listBarLayoutText.
        contains(QStringLiteral("listBarLayout.noteListModel.searchText = listBarLayout.searchText;")),
        "ListBarLayout.qml must push the active search text into the bound note list model.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("function pushCurrentIndexToModel(index)")),
        "ListBarLayout.qml must expose an explicit helper that pushes ListView selection into the note model.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("function activateNoteIndex(index, noteId)")),
        "ListBarLayout.qml must centralize immediate note activation through an explicit helper that also captures the visually focused note id.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("signal noteActivated(int index, string noteId)")),
        "ListBarLayout.qml must expose an explicit noteActivated signal so routed mobile shells can react to note taps without forking the shared delegate.");
    const QString activateNoteIndexBody = extractFunctionBody(
        listBarLayoutText, QStringLiteral("function activateNoteIndex(index, noteId)"));
    QVERIFY2(
        activateNoteIndexBody.contains(QStringLiteral("Qt.callLater(function () {")),
        "ListBarLayout.qml must reapply the latest note selection after the current event turn so editor save/refresh work cannot drop the user's last click.");
    QVERIFY2(
        activateNoteIndexBody.contains(QStringLiteral("listBarLayout.pushCurrentIndexToModel(targetIndex);")),
        "ListBarLayout.qml immediate note activation helper must push the selected row into the bound note-list model.");
    QVERIFY2(
        activateNoteIndexBody.contains(QStringLiteral("noteDeletionBridge.focusedNoteId = normalizedNoteId;")),
        "ListBarLayout.qml immediate note activation helper must capture the tapped note id for low-latency keyboard deletion.");
    QVERIFY2(
        activateNoteIndexBody.contains(QStringLiteral("listBarLayout.noteActivated(targetIndex, normalizedNoteId);")),
        "ListBarLayout.qml immediate note activation helper must emit the shared noteActivated signal after the authoritative selection update so mobile routing can follow the tapped note.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("function syncCurrentIndexFromModel()")),
        "ListBarLayout.qml must pull currentIndex from the note model back into ListView state.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("property bool syncingCurrentIndexFromModel: false")),
        "ListBarLayout.qml must track when ListView currentIndex is being synchronized from the note model so reset-time defaults cannot leak back into app state.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("function currentIndexChangeIsAuthoritative(index)")),
        "ListBarLayout.qml must centralize the authority check for view-driven currentIndex changes.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("return listBarLayout.currentIndexFromModel() === normalizedIndex;")),
        "ListBarLayout.qml currentIndex authority helper must fall back to the bound note-model selection instead of trusting ListView defaults.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("if (!listBarLayout.currentIndexChangeIsAuthoritative(noteListView.currentIndex))")),
        "ListBarLayout.qml must reject unsolicited ListView currentIndex changes and resync from the note model.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("listBarLayout.syncingCurrentIndexFromModel = true;")) &&
            listBarLayoutText.contains(QStringLiteral("listBarLayout.syncingCurrentIndexFromModel = false;")),
        "ListBarLayout.qml must wrap model-driven currentIndex synchronization with an explicit guard flag.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("property bool noteDragActive: false")),
        "ListBarLayout.qml must track whether a note-card drag is active so the parent ListView can stop treating the gesture as scrolling.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("property bool noteDragCanceled: false")),
        "ListBarLayout.qml must track canceled note drags separately so desktop drop commits do not fire during teardown paths.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("id: noteDragPreviewState")) &&
            listBarLayoutText.contains(QStringLiteral("property var delegateItem: null")),
        "ListBarLayout.qml must group drag-preview state in a dedicated local object so the root property surface stays smaller while the actively dragged delegate still drives the floating preview.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("property var noteDropTarget: null")),
        "ListBarLayout.qml must accept an explicit hierarchy drop target so desktop internal note drags can resolve folder assignment across panels.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("function clearInternalNoteDropPreview()")) &&
            listBarLayoutText.contains(QStringLiteral("function updateInternalNoteDropPreview(delegateItem, localX, localY)")),
        "ListBarLayout.qml must drive desktop note-drop hover previews through explicit shared-target helpers.");
    QVERIFY2(
        !listBarLayoutText.contains(QStringLiteral("property bool notePointerPressed: false")),
        "ListBarLayout.qml must not keep a redundant notePointerPressed root property once pressed-row state is already tracked through the delegate index and local selection state.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("property int pressedNoteIndex: -1")),
        "ListBarLayout.qml must track a transient pressed note row separately from the committed currentIndex so drag startup does not mutate selection too early.");
    QVERIFY2(
        listBarLayoutText.contains(
            QStringLiteral(
                "interactive: contentHeight > height && !listBarLayout.noteDragActive")),
        "ListBarLayout.qml must keep ListView scrolling available during note-row press handling and only suspend viewport dragging while an actual note drag is active.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("grabPermissions: PointerHandler.ApprovesTakeOverByAnything")),
        "ListBarLayout.qml tap handlers must approve drag takeover so note-card drags can start from the same pointer press as row selection.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("model: listBarLayout.resolvedNoteListModel")),
        "ListBarLayout.qml must bind ListView only to the resolved note-list model.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("readonly property bool useInternalNoteDrag: !LV.Theme.mobileTarget")),
        "ListBarLayout.qml must resolve note drag dispatch per runtime profile so desktop keeps the in-scene card preview while mobile can still publish mime data.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("readonly property int mobileNoteDragHoldInterval: 1000")),
        "ListBarLayout.qml must expose an explicit 1000ms mobile long-press interval before note dragging begins.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("Drag.dragType: listBarLayout.useInternalNoteDrag ? Drag.Internal : Drag.Automatic")),
        "ListBarLayout.qml note delegates must use internal drag on desktop and automatic mime-backed drag only on mobile targets.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("LV.ContextMenu {")) &&
            listBarLayoutText.contains(QStringLiteral("id: noteContextMenu")),
        "ListBarLayout.qml must mount an LVRS context menu dedicated to note-card actions.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("onItemEventTriggered: function(eventName, payload, index, item) {")),
        "ListBarLayout.qml note-card context menu must handle normalized LVRS menu events instead of binding delegate-local callbacks.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("eventName === \"note.delete\"")) &&
            listBarLayoutText.contains(QStringLiteral("eventName === \"note.clearAllFolders\"")),
        "ListBarLayout.qml note-card context menu must expose both delete-note and clear-all-folders actions.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("Drag.keys: [\"whatson.library.note\"]")),
        "ListBarLayout.qml note delegates must advertise whatson.library.note drag keys.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("\"application/x-whatson-note-id\": noteItemDelegate.noteId")),
        "ListBarLayout.qml note delegates must publish noteId through drag mime data so the Qt drag operation has a concrete payload.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function noteIdFromDragPayload(drag)")),
        "SidebarHierarchyView.qml must normalize note ids from the drag event itself rather than depending only on drag.source.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("drag.getDataAsString(\"application/x-whatson-note-id\")")),
        "SidebarHierarchyView.qml must read note ids from the custom mime payload when DropArea source metadata is unavailable.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("drag.getDataAsString(\"text/plain\")")),
        "SidebarHierarchyView.qml must fall back to text/plain note ids so note drops stay recoverable across Qt drag backends.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("Drag.supportedActions: Qt.CopyAction")),
        "ListBarLayout.qml note delegates must advertise additive note-to-folder drag semantics.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("parent: Controls.Overlay.overlay ? Controls.Overlay.overlay : listBarLayout")),
        "ListBarLayout.qml drag preview must be reparented into the overlay layer so the grabbed note card can cross panel boundaries.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("active: true")),
        "ListBarLayout.qml drag preview note card must force the active visual state while dragging.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("readonly property real grabbedNoteOpacity: 0.25")),
        "ListBarLayout.qml must centralize the grabbed-note translucency so drag hover cues stay visible across the sidebar.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("opacity: listBarLayout.grabbedNoteOpacity")),
        "ListBarLayout.qml drag preview note card must stay translucent enough for hovered-folder feedback to remain visible underneath.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("opacity: noteItemDelegate.pointerDragActive ? listBarLayout.grabbedNoteOpacity : 1")),
        "ListBarLayout.qml grabbed source row must also dim to the shared dragged-note opacity while the note card is being carried, regardless of desktop immediate drag or mobile long-press drag.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("function beginNoteDragPreview(delegateItem, hotSpotX, hotSpotY)")),
        "ListBarLayout.qml must expose an explicit helper that captures the dragged note-card preview state.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("function updateNoteDragPreviewPosition(delegateItem, localX, localY)")),
        "ListBarLayout.qml must expose an explicit helper that moves the dragged note-card preview with the pointer.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("function commitInternalNoteDrop(delegateItem, localX, localY)")),
        "ListBarLayout.qml must expose an explicit helper that commits desktop internal note drags into the shared hierarchy drop target on release.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("delegateItem.mapToItem(listBarLayout.noteDropTarget, Number(localX) || 0, Number(localY) || 0)")),
        "ListBarLayout.qml desktop note-drop commit must map the release point into the shared sidebar hierarchy coordinate space.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral(
            "image: noteItemDelegate.image === undefined ? false : Boolean(noteItemDelegate.image)")),
        "ListBarLayout.qml must bind the image flag directly from the required note delegate role contract.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral(
            "imageSource: noteItemDelegate.imageSource === undefined || noteItemDelegate.imageSource === null ? \"\" : noteItemDelegate.imageSource")),
        "ListBarLayout.qml must bind imageSource directly from the required note delegate role contract.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("readonly property bool pointerDragRequiresLongPress: LV.Theme.mobileTarget")) &&
            listBarLayoutText.contains(QStringLiteral("readonly property bool immediatePointerDragEnabled: !noteItemDelegate.pointerDragRequiresLongPress")) &&
            listBarLayoutText.contains(QStringLiteral("readonly property bool pointerDragActive: noteDragHandler.active || noteItemDelegate.mobilePointerDragging")),
        "ListBarLayout.qml note delegates must split desktop immediate drag and mobile long-press drag through explicit pointer-drag state contracts.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("property bool mobileLongPressPendingContextMenu: false")) &&
            listBarLayoutText.contains(QStringLiteral("property bool mobileSuppressNextClick: false")),
        "ListBarLayout.qml mobile note delegates must track whether a long press should open the context menu and suppress the follow-up tap activation.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("grabPermissions: PointerHandler.CanTakeOverFromAnything")),
        "ListBarLayout.qml note drag handler must be able to take pointer ownership away from tap selection and list scrolling.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("dragThreshold: 4")),
        "ListBarLayout.qml desktop note drag handler should keep a low drag threshold so pointer drags begin promptly on non-mobile targets.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("enabled: noteItemDelegate.immediatePointerDragEnabled")),
        "ListBarLayout.qml must disable the immediate drag/tap handlers on mobile so note rows can wait for long-press before drag pickup.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("onCentroidChanged: {")),
        "ListBarLayout.qml note drag handler must update the floating note-card preview while the pointer moves.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("listBarLayout.updateInternalNoteDropPreview(")),
        "ListBarLayout.qml note drag handler must refresh the hovered hierarchy drop preview while the pointer moves across folders.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("if (!listBarLayout.noteDragCanceled)")),
        "ListBarLayout.qml note drag handler must only commit desktop folder drops on normal release, not on canceled drag teardown.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("listBarLayout.commitInternalNoteDrop(")),
        "ListBarLayout.qml note drag handler must attempt the shared desktop folder-drop commit when an internal drag ends.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("listBarLayout.clearInternalNoteDropPreview();")),
        "ListBarLayout.qml note drag teardown must clear any hovered hierarchy drop preview after desktop internal drags finish or cancel.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("gesturePolicy: TapHandler.DragThreshold")),
        "ListBarLayout.qml must keep the desktop note-card tap handler passive until drag threshold so taps and immediate pointer drags can share the same surface.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("enabled: !noteItemDelegate.pointerDragRequiresLongPress")) &&
            listBarLayoutText.contains(QStringLiteral("acceptedButtons: Qt.RightButton")) &&
            listBarLayoutText.contains(QStringLiteral("listBarLayout.openNoteContextMenu(")),
        "ListBarLayout.qml must keep the immediate context-menu TapHandler desktop-only so mobile touch taps do not synthesize the note-card context menu.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("acceptedButtons: Qt.RightButton")) &&
            listBarLayoutText.contains(QStringLiteral("listBarLayout.openNoteContextMenu(")),
        "ListBarLayout.qml note delegates must route right-click gestures into the shared note-card context menu helper.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("onPressAndHold: function(mouse) {")) &&
            listBarLayoutText.contains(QStringLiteral("noteItemDelegate.mobileLongPressPendingContextMenu = true;")) &&
            listBarLayoutText.contains(QStringLiteral("noteItemDelegate.mobileSuppressNextClick = true;")) &&
            listBarLayoutText.contains(QStringLiteral("const openContextMenu = noteItemDelegate.mobileLongPressPendingContextMenu && !dragging;")) &&
            listBarLayoutText.contains(QStringLiteral("listBarLayout.openNoteContextMenu(")),
        "ListBarLayout.qml mobile note cards must require the shared 1000ms hold before arming the context menu, then open it on release when the pointer never escalates into a drag.");
    QVERIFY2(
        !listBarLayoutText.contains(QStringLiteral("TapHandler.ReleaseWithinBounds")),
        "ListBarLayout.qml note tap handler must not take an exclusive press grab that blocks drag startup.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("onPressedChanged: {")),
        "ListBarLayout.qml note tap handling must preserve a transient press preview before the drag threshold resolves.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("onTapped: {")),
        "ListBarLayout.qml note-card selection must be committed on tap release so note drags can coexist with scroll and press feedback.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("id: mobileLongPressDragArea")) &&
            listBarLayoutText.contains(QStringLiteral("enabled: noteItemDelegate.pointerDragRequiresLongPress")) &&
            listBarLayoutText.contains(QStringLiteral("pressAndHoldInterval: listBarLayout.mobileNoteDragHoldInterval")) &&
            listBarLayoutText.contains(QStringLiteral("noteItemDelegate.mobilePointerDragging = true;")),
        "ListBarLayout.qml mobile note cards must defer drag pickup to a dedicated long-press area with a 1000ms hold interval.");
    QVERIFY2(
        listBarLayoutText.contains(
            QStringLiteral("listBarLayout.activateNoteIndex(noteItemDelegate.index, noteCard.noteId);")),
        "ListBarLayout.qml note-card tap handler must route authoritative activation through activateNoteIndex().");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("function syncFocusedNoteDeletionState()")),
        "ListBarLayout.qml must expose a helper that resyncs focused note deletion state from the visible current card.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("listBarLayout.noteListModel.currentIndex = index;")),
        "ListBarLayout.qml must push tapped note selection into noteModel.currentIndex.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("id: noteSelectionState")) &&
            listBarLayoutText.contains(QStringLiteral("property int pendingIndex: -1")),
        "ListBarLayout.qml must keep pending user note-selection intent inside a dedicated local state object while editor save/refresh work settles.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("onNoteListModelChanged: {")),
        "ListBarLayout.qml must resync the visible note selection whenever the active note model changes.");

    const QString listBarHeaderPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/ListBarHeader.qml"));
    QFile listBarHeaderFile(listBarHeaderPath);
    QVERIFY2(listBarHeaderFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(listBarHeaderPath));
    const QString listBarHeaderText = QString::fromUtf8(listBarHeaderFile.readAll());
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("mode: searchMode")),
        "ListBarHeader.qml search field must use LV.InputField searchMode so the built-in search icon is shown.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("clearButtonVisible: false")),
        "ListBarHeader.qml search field must suppress the default clear affordance for the Figma inline style.");
    QVERIFY2(
        listBarHeaderText.contains(
            QStringLiteral("property color inlineFieldBackgroundColor: \"transparent\"")),
        "ListBarHeader.qml inline input must centralize the transparent inline background override.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("readonly property int actionButtonSize: LV.Theme.gap20")),
        "ListBarHeader.qml trailing action buttons must resolve through the LVRS gap20 token.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("property int frameMinHeight: LV.Theme.gap24")),
        "ListBarHeader.qml must keep the desktop header height contract on the LVRS gap24 token.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("readonly property int shapeCylinder: searchField.shapeCylinder")) &&
            listBarHeaderText.contains(QStringLiteral("readonly property int shapeRoundRect: searchField.shapeRoundRect")) &&
            listBarHeaderText.contains(QStringLiteral("property int searchFieldShapeStyle: listBarHeader.shapeCylinder")) &&
            listBarHeaderText.contains(QStringLiteral("shapeStyle: listBarHeader.searchFieldShapeStyle")),
        "ListBarHeader.qml inline input must expose both LVRS search-field shapes while defaulting the list header to the cylindrical pill shape from Figma.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("readonly property int inlineFieldHeight: LV.Theme.gap18")),
        "ListBarHeader.qml inline input must keep the 18px height contract from the LVRS gap18 token.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("readonly property int inlineFieldTextHeight: LV.Theme.gap12")),
        "ListBarHeader.qml inline input must keep the 12px text line box from the LVRS gap12 token.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("readonly property int inlineFieldHorizontalInset: LV.Theme.gap7")),
        "ListBarHeader.qml inline input must use the LVRS gap7 token for the horizontal inset.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("readonly property int inlineFieldVerticalInset: LV.Theme.gap3")),
        "ListBarHeader.qml inline input must use the LVRS gap3 token for the vertical inset.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("property int outerHorizontalInset: LV.Theme.gap2")),
        "ListBarHeader.qml must expose tokenized outer horizontal chrome so mobile can collapse the search wrapper.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("property int outerVerticalInset: LV.Theme.gap2")),
        "ListBarHeader.qml must expose tokenized outer vertical chrome so mobile can collapse the search wrapper.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("readonly property int resolvedInputTextHeight: Math.max(")),
        "ListBarHeader.qml must derive the live search text box height from the InputField contentHeight so whitespace edits stay vertically centered.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("implicitHeight: Math.max(listBarHeader.frameMinHeight, headerRow.implicitHeight + listBarHeader.outerVerticalInset * 2)")),
        "ListBarHeader.qml must derive its wrapper height from the tokenized outer chrome so mobile can reduce the search/header gap.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("target: searchField.inputItem")),
        "ListBarHeader.qml must override the underlying LVRS input item directly for stable inline search alignment.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("property: \"height\"")),
        "ListBarHeader.qml must bind the underlying input height for stable search text centering.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("property: \"y\"")),
        "ListBarHeader.qml must bind the underlying input y-position for stable search text centering.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral(
            "value: Math.max(0, Math.floor((searchField.height - searchField.resolvedInputTextHeight) / 2))")),
        "ListBarHeader.qml must recenter the live input item from its resolved text height.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("iconName: \"cwmPermissionView\"")),
        "ListBarHeader.qml visibility button must use the Figma eye icon asset.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("iconName: \"sortByType\"")),
        "ListBarHeader.qml sort button must keep the Figma sort icon asset.");

    const QString bodyLayoutPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/BodyLayout.qml"));
    QFile bodyLayoutFile(bodyLayoutPath);
    QVERIFY2(bodyLayoutFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(bodyLayoutPath));
    const QString bodyLayoutText = QString::fromUtf8(bodyLayoutFile.readAll());

    QVERIFY2(
        bodyLayoutText.contains(QStringLiteral("function totalSplitterWidth()")),
        "BodyLayout.qml must keep totalSplitterWidth helper.");
    QVERIFY2(
        bodyLayoutText.contains(QStringLiteral("return isFinite(width) ? width : 0;")),
        "BodyLayout.qml totalSplitterWidth must sanitize non-finite width values.");
    QVERIFY2(
        bodyLayoutText.contains(QStringLiteral("required property var sidebarHierarchyViewModel")),
        "BodyLayout.qml must require the shared sidebar hierarchy state manager.");
    QVERIFY2(
        bodyLayoutText.contains(
            QStringLiteral(
                "readonly property int activeHierarchyIndex: hStack.sidebarHierarchyViewModel ? hStack.sidebarHierarchyViewModel.resolvedActiveHierarchyIndex : 0")),
        "BodyLayout.qml must source the active hierarchy index directly from SidebarHierarchyViewModel.");
    QVERIFY2(
        bodyLayoutText.contains(
            QStringLiteral(
                "readonly property var activeHierarchyViewModel: hStack.sidebarHierarchyViewModel ? hStack.sidebarHierarchyViewModel.resolvedHierarchyViewModel : null")),
        "BodyLayout.qml must source the active hierarchy view-model directly from SidebarHierarchyViewModel.");
    QVERIFY2(
        bodyLayoutText.contains(
            QStringLiteral(
                "readonly property var activeNoteListModel: hStack.sidebarHierarchyViewModel ? hStack.sidebarHierarchyViewModel.resolvedNoteListModel : null")),
        "BodyLayout.qml must source the active note-list model directly from SidebarHierarchyViewModel.");
    QVERIFY2(
        bodyLayoutText.contains(QStringLiteral("property var noteDeletionViewModel: null")),
        "BodyLayout.qml must accept a dedicated delete-note command source instead of hardwiring note deletion inside ListBarLayout.");
    QVERIFY2(
        bodyLayoutText.contains(QStringLiteral("property var libraryHierarchyViewModel: null")),
        "BodyLayout.qml must accept the shared library hierarchy view-model explicitly instead of relying on a self-referential child binding.");
    QVERIFY2(
        bodyLayoutText.contains(QStringLiteral("activeToolbarIndex: hStack.activeHierarchyIndex")),
        "BodyLayout.qml must feed child hierarchy/list views from the resolved active hierarchy index.");
    QVERIFY2(
        bodyLayoutText.contains(QStringLiteral("noteListModel: hStack.activeNoteListModel")),
        "BodyLayout.qml must pass the resolved active note-list model into child views.");
    QVERIFY2(
        bodyLayoutText.contains(QStringLiteral("noteDeletionViewModel: hStack.noteDeletionViewModel")),
        "BodyLayout.qml must forward the centralized delete-note command source into ListBarLayout.");
    QVERIFY2(
        bodyLayoutText.contains(QStringLiteral("noteDropTarget: sideBar.noteDropTargetView")),
        "BodyLayout.qml must forward the shared sidebar hierarchy view into ListBarLayout so desktop internal note drags can commit folder drops.");
    QVERIFY2(
        bodyLayoutText.contains(QStringLiteral("contentViewModel: hStack.activeHierarchyViewModel")),
        "BodyLayout.qml must pass the resolved active hierarchy view-model into the content surface.");
    QVERIFY2(
        bodyLayoutText.contains(QStringLiteral("libraryHierarchyViewModel: hStack.libraryHierarchyViewModel")),
        "BodyLayout.qml must forward the explicit library hierarchy view-model into the content surface without a self-binding loop.");
    QVERIFY2(
        bodyLayoutText.contains(QStringLiteral("searchFieldVisible: true")),
        "BodyLayout.qml desktop hierarchy route must keep the shared hierarchy search field visible.");
    QVERIFY2(
        bodyLayoutText.contains(QStringLiteral("height: Math.max(1, hStack.splitterThickness)")) &&
            bodyLayoutText.contains(QStringLiteral("color: hStack.splitterColor")),
        "BodyLayout.qml must render a desktop top border using the shared splitter token so the navigation bar and content HStack stay visually separated.");
    QVERIFY2(
        bodyLayoutText.contains(QStringLiteral("PanelEdgeSplitter {")),
        "BodyLayout.qml must delegate splitter drag interactions to the reusable PanelEdgeSplitter component.");

    const QString detailPanelPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/detail/DetailPanel.qml"));
    QFile detailPanelFile(detailPanelPath);
    QVERIFY2(detailPanelFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(detailPanelPath));
    const QString detailPanelText = QString::fromUtf8(detailPanelFile.readAll());
    QVERIFY2(
        detailPanelText.contains(QStringLiteral("readonly property var detailPanelVm: detailPanelViewModel")),
        "DetailPanel.qml must bind detailPanelVm from detailPanelViewModel context.");
    QVERIFY2(
        detailPanelText.contains(QStringLiteral(
            "readonly property var resolvedActiveContentViewModel: detailPanel.resolveActiveContentViewModel()")),
        "DetailPanel.qml must resolve the active content view-model through an explicit contract property.");
    QVERIFY2(
        detailPanelText.contains(
            QStringLiteral("readonly property string resolvedActiveStateName: detailPanel.resolveActiveStateName()")),
        "DetailPanel.qml must resolve the active state name through an explicit contract property.");
    QVERIFY2(
        detailPanelText.contains(
            QStringLiteral("readonly property var resolvedToolbarItems: detailPanel.resolveToolbarItems()")),
        "DetailPanel.qml must resolve toolbar specs through an explicit contract property.");
    QVERIFY2(
        detailPanelText.contains(QStringLiteral("function resolveActiveContentViewModel()")),
        "DetailPanel.qml must expose a helper that resolves the active content view-model from C++ state.");
    QVERIFY2(
        detailPanelText.contains(QStringLiteral("function resolveActiveStateName()")),
        "DetailPanel.qml must expose a helper that resolves the active detail state name from C++ state.");
    QVERIFY2(
        detailPanelText.contains(QStringLiteral("function resolveToolbarItems()")),
        "DetailPanel.qml must expose a helper that resolves toolbar specs from C++ state.");
    QVERIFY2(
        detailPanelText.contains(QStringLiteral("toolbarButtonSpecs: detailPanel.resolvedToolbarItems")),
        "DetailPanel.qml must source toolbar specs from the resolved detail-panel contract.");
    QVERIFY2(
        detailPanelText.contains(QStringLiteral("detailPanel.detailPanelVm.requestStateChange(stateValue);")),
        "DetailPanel.qml must forward toolbar state changes to C++ detailPanelViewModel.");
    QVERIFY2(
        detailPanelText.contains(QStringLiteral("activeContentViewModel: detailPanel.resolvedActiveContentViewModel")),
        "DetailPanel.qml must forward the resolved content view-model into DetailContents.");
    QVERIFY2(
        detailPanelText.contains(QStringLiteral("activeStateName: detailPanel.resolvedActiveStateName")),
        "DetailPanel.qml must forward the resolved state name into DetailContents.");

    const QString detailToolbarPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/detail/DetailPanelHeaderToolbar.qml"));
    QFile detailToolbarFile(detailToolbarPath);
    QVERIFY2(detailToolbarFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(detailToolbarPath));
    const QString detailToolbarText = QString::fromUtf8(detailToolbarFile.readAll());
    QVERIFY2(
        detailToolbarText.contains(QStringLiteral("signal detailStateChangeRequested(int stateValue)")),
        "DetailPanelHeaderToolbar.qml must expose detailStateChangeRequested(int stateValue).");
    QVERIFY2(
        detailToolbarText.contains(QStringLiteral(
            "readonly property var resolvedToolbarButtonSpecs: detailPanelHeaderToolbar.normalizeToolbarButtonSpecs(detailPanelHeaderToolbar.toolbarButtonSpecs)")),
        "DetailPanelHeaderToolbar.qml must normalize incoming toolbar specs through an explicit contract property.");
    QVERIFY2(
        detailToolbarText.contains(QStringLiteral("function normalizeToolbarButtonSpecs(value)")),
        "DetailPanelHeaderToolbar.qml must expose a helper that normalizes toolbar specs into an array.");
    QVERIFY2(
        detailToolbarText.contains(QStringLiteral("implicitHeight: 20")),
        "DetailPanelHeaderToolbar.qml must keep the Figma 20px frame height.");
    QVERIFY2(
        detailToolbarText.contains(QStringLiteral("implicitWidth: 145")),
        "DetailPanelHeaderToolbar.qml must keep the Figma 145px frame width.");
    QVERIFY2(
        detailToolbarText.contains(QStringLiteral("spacing: detailPanelHeaderToolbar.buttonSpacing")),
        "DetailPanelHeaderToolbar.qml must keep the Figma 5px inter-button spacing contract.");
    QVERIFY2(
        detailToolbarText.contains(QStringLiteral("DetailPanelHeaderToolbarButton")),
        "DetailPanelHeaderToolbar.qml must compose DetailPanelHeaderToolbarButton delegates.");
    QVERIFY2(
        detailToolbarText.contains(QStringLiteral("model: detailPanelHeaderToolbar.resolvedToolbarButtonSpecs.length")),
        "DetailPanelHeaderToolbar.qml must drive its repeater from resolved toolbar specs.");
    QVERIFY2(
        detailToolbarText.contains(
            QStringLiteral("buttonSpec: detailPanelHeaderToolbar.resolvedToolbarButtonSpecs[index]")),
        "DetailPanelHeaderToolbar.qml must pass resolved toolbar specs into each delegate.");
    QVERIFY2(
        detailToolbarText.contains(QStringLiteral(
            "detailPanelHeaderToolbar.detailStateChangeRequested(stateValue);")),
        "DetailPanelHeaderToolbar.qml must emit detailStateChangeRequested from delegate clicks.");

    const QString detailToolbarButtonPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/detail/DetailPanelHeaderToolbarButton.qml"));
    QFile detailToolbarButtonFile(detailToolbarButtonPath);
    QVERIFY2(
        detailToolbarButtonFile.open(QIODevice::ReadOnly | QIODevice::Text),
        qPrintable(detailToolbarButtonPath));
    const QString detailToolbarButtonText = QString::fromUtf8(detailToolbarButtonFile.readAll());
    QVERIFY2(
        detailToolbarButtonText.contains(
            QStringLiteral("property bool selected: buttonSpec && buttonSpec.selected === true")),
        "DetailPanelHeaderToolbarButton.qml must use C++ selected field from toolbar specs.");
    QVERIFY2(
        detailToolbarButtonText.contains(QStringLiteral("iconSize: 16")),
        "DetailPanelHeaderToolbarButton.qml must keep the Figma 16px icon size.");
    QVERIFY2(
        detailToolbarButtonText.contains(
            QStringLiteral("tone: selected ? LV.AbstractButton.Default : LV.AbstractButton.Borderless")),
        "DetailPanelHeaderToolbarButton.qml must map selected state to filled/default tone and others to borderless.");
    QVERIFY2(
        detailToolbarButtonText.contains(QStringLiteral("stateClickRequested(nextState);")),
        "DetailPanelHeaderToolbarButton.qml must emit stateClickRequested from icon button clicks.");

    const QString detailContentsPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/detail/DetailContents.qml"));
    QFile detailContentsFile(detailContentsPath);
    QVERIFY2(detailContentsFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(detailContentsPath));
    const QString detailContentsText = QString::fromUtf8(detailContentsFile.readAll());
    QVERIFY2(
        detailContentsText.contains(QStringLiteral("property var activeContentViewModel: null")),
        "DetailContents.qml must expose activeContentViewModel for dedicated detail section view-model wiring.");
    QVERIFY2(
        detailContentsText.contains(QStringLiteral("property string activeStateName: \"fileInfo\"")),
        "DetailContents.qml must expose activeStateName default.");
    QVERIFY2(
        detailContentsText.contains(QStringLiteral(
            "readonly property string resolvedActiveStateName: detailContents.normalizeStateName(detailContents.activeStateName)")),
        "DetailContents.qml must normalize incoming state names through an explicit contract property.");
    QVERIFY2(
        detailContentsText.contains(QStringLiteral("function normalizeStateName(value)")),
        "DetailContents.qml must expose a helper that clamps incoming state names to known values.");
    QVERIFY2(
        detailContentsText.contains(QStringLiteral("state: detailContents.resolvedActiveStateName")),
        "DetailContents.qml must bind QML state to the resolved active state name.");
}

void QmlBindingSyntaxGuardTest::mobileHierarchyPage_mustRouteHierarchyActivationIntoNoteListBody()
{
    const QDir testsDir(QStringLiteral(QT_TESTCASE_SOURCEDIR));
    const QString qmlRoot = testsDir.absoluteFilePath(QStringLiteral("../src/app/qml"));

    const QString navigationBarPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/NavigationBarLayout.qml"));
    QFile navigationBarFile(navigationBarPath);
    QVERIFY2(navigationBarFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(navigationBarPath));
    const QString navigationBarText = QString::fromUtf8(navigationBarFile.readAll());
    QVERIFY2(
        navigationBarText.contains(QStringLiteral("signal compactLeadingActionRequested")) &&
            navigationBarText.contains(QStringLiteral("property bool compactLeadingActionVisible: false")) &&
            navigationBarText.contains(QStringLiteral("property bool compactNoteListControlsVisible: false")) &&
            navigationBarText.contains(QStringLiteral("property bool compactSettingsVisible: true")) &&
            navigationBarText.contains(
                QStringLiteral("property string compactLeadingActionIconName: \"generalchevronLeft\"")),
        "NavigationBarLayout.qml compact mode must expose a dedicated leading-action slot so mobile pages can mount a back affordance without forking the shared bar.");
    QVERIFY2(
        navigationBarText.contains(QStringLiteral("iconName: navigationBar.compactLeadingActionIconName")) &&
            navigationBarText.contains(QStringLiteral("visible: navigationBar.compactSettingsVisible")) &&
            navigationBarText.contains(QStringLiteral("navigationBar.compactLeadingActionRequested()")),
        "NavigationBarLayout.qml compact leading action must bind its icon and click flow through the dedicated compactLeadingAction contract while letting mobile routes suppress the shared settings affordance.");

    const QString mobileScaffoldPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/mobile/MobilePageScaffold.qml"));
    QFile mobileScaffoldFile(mobileScaffoldPath);
    QVERIFY2(mobileScaffoldFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(mobileScaffoldPath));
    const QString mobileScaffoldText = QString::fromUtf8(mobileScaffoldFile.readAll());
    QVERIFY2(
        mobileScaffoldText.contains(QStringLiteral("property bool compactAddFolderVisible: true")) &&
            mobileScaffoldText.contains(QStringLiteral("property bool compactLeadingActionVisible: false")) &&
            mobileScaffoldText.contains(QStringLiteral("property bool compactNoteListControlsVisible: false")) &&
            mobileScaffoldText.contains(QStringLiteral("property bool compactSettingsVisible: true")) &&
            mobileScaffoldText.contains(QStringLiteral("signal compactLeadingActionRequested")) &&
            mobileScaffoldText.contains(QStringLiteral("property string bodyInitialPath: \"/\"")) &&
            mobileScaffoldText.contains(QStringLiteral("property var bodyRoutes: []")),
        "MobilePageScaffold.qml must expose compact-bar visibility knobs so routed mobile pages can switch chrome without cloning the shared scaffold.");
    QVERIFY2(
        mobileScaffoldText.contains(
            QStringLiteral("compactAddFolderVisible: mobilePageScaffold.compactAddFolderVisible")) &&
            mobileScaffoldText.contains(
                QStringLiteral("compactNoteListControlsVisible: mobilePageScaffold.compactNoteListControlsVisible")) &&
            mobileScaffoldText.contains(
                QStringLiteral("compactSettingsVisible: mobilePageScaffold.compactSettingsVisible")) &&
            mobileScaffoldText.contains(
                QStringLiteral("compactLeadingActionVisible: mobilePageScaffold.compactLeadingActionVisible")) &&
            mobileScaffoldText.contains(
                QStringLiteral("onCompactLeadingActionRequested: mobilePageScaffold.compactLeadingActionRequested()")) &&
            mobileScaffoldText.contains(QStringLiteral("readonly property var activePageRouter: bodyRouter")) &&
            mobileScaffoldText.contains(QStringLiteral("readonly property var bodyItem: bodyRouter.currentPageItem")) &&
            mobileScaffoldText.contains(QStringLiteral("LV.PageRouter {")) &&
            mobileScaffoldText.contains(QStringLiteral("routes: mobilePageScaffold.bodyRoutes")) &&
            !mobileScaffoldText.contains(QStringLiteral("Loader {")),
        "MobilePageScaffold.qml must forward compact chrome through the shared bars while routing mobile body pages through LV.PageRouter instead of a private loader.");

    const QString statusBarPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/StatusBarLayout.qml"));
    QFile statusBarFile(statusBarPath);
    QVERIFY2(statusBarFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(statusBarPath));
    const QString statusBarText = QString::fromUtf8(statusBarFile.readAll());
    QVERIFY2(
        statusBarText.contains(QStringLiteral("id: compactSearchInput")) &&
            statusBarText.contains(QStringLiteral("fieldMinHeight: statusBar.compactFieldHeight")) &&
            statusBarText.contains(QStringLiteral("backgroundColor: statusBar.compactFieldColor")) &&
            statusBarText.contains(QStringLiteral("placeholderText: statusBar.compactToolbarText")) &&
            statusBarText.contains(QStringLiteral("shapeStyle: shapeCylinder")) &&
            !statusBarText.contains(QStringLiteral("id: compactSearchTextField")),
        "StatusBarLayout.qml compact search input must let LVRS InputField render the cylindrical frame directly so the mobile bottom bar keeps the shared pill-shaped search affordance instead of a wrapped round-rect shell.");

    const QString mobilePagePath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/mobile/pages/MobileHierarchyPage.qml"));
    QFile mobilePageFile(mobilePagePath);
    QVERIFY2(mobilePageFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(mobilePagePath));
    const QString mobilePageText = QString::fromUtf8(mobilePageFile.readAll());
    QVERIFY2(
        mobilePageText.contains(
            QStringLiteral("readonly property var activeContentViewModel: mobileHierarchyPage.sidebarHierarchyViewModel")) &&
            mobilePageText.contains(QStringLiteral("resolvedHierarchyViewModel")),
        "MobileHierarchyPage.qml must source the active editor content view-model directly from SidebarHierarchyViewModel so mobile editing follows the same hierarchy-backed model contract as desktop.");
    QVERIFY2(
        mobilePageText.contains(
            QStringLiteral("readonly property var activeNoteListModel: mobileHierarchyPage.sidebarHierarchyViewModel")) &&
            mobilePageText.contains(QStringLiteral("resolvedNoteListModel")),
        "MobileHierarchyPage.qml must source the active note-list model directly from SidebarHierarchyViewModel instead of resolving a separate mobile-only list pipeline.");
    QVERIFY2(
        mobilePageText.contains(QStringLiteral("readonly property string editorRoutePath: \"/mobile/editor\"")) &&
        mobilePageText.contains(QStringLiteral("readonly property string hierarchyRoutePath: \"/mobile/hierarchy\"")) &&
            mobilePageText.contains(QStringLiteral("readonly property string noteListRoutePath: \"/mobile/note-list\"")) &&
            mobilePageText.contains(QStringLiteral("readonly property var mobileBodyRoutes: [")) &&
            mobilePageText.contains(QStringLiteral("\"component\": hierarchyBodyComponent")) &&
            mobilePageText.contains(QStringLiteral("\"component\": noteListBodyComponent")) &&
            mobilePageText.contains(QStringLiteral("\"component\": editorBodyComponent")),
        "MobileHierarchyPage.qml must declare explicit LV.PageRouter route entries for the hierarchy body, note-list body, and editor body.");
    QVERIFY2(
        mobilePageText.contains(QStringLiteral("property int backSwipeConsumedSessionId: -1")) &&
        mobilePageText.contains(QStringLiteral("property int backSwipeSessionId: -1")) &&
            mobilePageText.contains(QStringLiteral("property int preservedNoteListSelectionIndex: -1")) &&
            mobilePageText.contains(QStringLiteral("property bool routeSelectionSyncSuppressed: false")) &&
            mobilePageText.contains(
                QStringLiteral("readonly property bool backNavigationAvailable: mobileScaffold.activePageRouter")) &&
            mobilePageText.contains(QStringLiteral("readonly property int backSwipeEdgeWidth: LV.Theme.gap24")) &&
            mobilePageText.contains(
                QStringLiteral("readonly property string resolvedBodyRoutePath: mobileHierarchyPage.displayedBodyRoutePath()")) &&
            mobilePageText.contains(
                QStringLiteral("readonly property bool editorPageActive: mobileHierarchyPage.resolvedBodyRoutePath === mobileHierarchyPage.editorRoutePath")) &&
            mobilePageText.contains(
                QStringLiteral("readonly property bool noteListPageActive: mobileHierarchyPage.resolvedBodyRoutePath === mobileHierarchyPage.noteListRoutePath")),
        "MobileHierarchyPage.qml must derive mobile note-list and editor state from the routed scaffold body contract instead of a private route-memory stack or a stale currentPath-only check.");
    QVERIFY2(
        mobilePageText.contains(
            QStringLiteral("bodyInitialPath: mobileHierarchyPage.hierarchyRoutePath")) &&
            mobilePageText.contains(QStringLiteral("bodyRoutes: mobileHierarchyPage.mobileBodyRoutes")) &&
            mobilePageText.contains(
                QStringLiteral("compactAddFolderVisible: !mobileHierarchyPage.noteListPageActive && !mobileHierarchyPage.editorPageActive")) &&
            mobilePageText.contains(QStringLiteral("compactNoteListControlsVisible: mobileHierarchyPage.noteListPageActive")) &&
            mobilePageText.contains(QStringLiteral("compactSettingsVisible: !mobileHierarchyPage.noteListPageActive")) &&
            mobilePageText.contains(QStringLiteral("compactLeadingActionVisible: false")),
        "MobileHierarchyPage.qml must drive scaffold body routing from the shared mobile PageRouter state while suppressing the compact folder affordance on routed note-list and editor views and swapping the compact control cluster to the note-list variant when the routed note-list page is active.");
    QVERIFY2(
        mobilePageText.contains(QStringLiteral("function requestBackToHierarchy()")) &&
            mobilePageText.contains(QStringLiteral("function currentHierarchySelectionIndex()")) &&
            mobilePageText.contains(QStringLiteral("function rememberNoteListSelection(selectionIndex)")) &&
            mobilePageText.contains(QStringLiteral("function requestOpenEditor(noteId, index)")) &&
            mobilePageText.contains(QStringLiteral("function cancelPendingEditorPopRepair()")) &&
            mobilePageText.contains(QStringLiteral("function restoreNoteListSelection(selectionIndex)")) &&
            mobilePageText.contains(QStringLiteral("function routePendingCreatedNoteToEditor()")) &&
            mobilePageText.contains(QStringLiteral("function scheduleCreatedNoteEditorRoute(noteId)")) &&
            mobilePageText.contains(QStringLiteral("function verifyCommittedEditorPopState(requestId, attemptsRemaining)")) &&
            mobilePageText.contains(QStringLiteral("function requestOpenNoteList(item, itemId, index)")) &&
            mobilePageText.contains(QStringLiteral("function routeToHierarchyRoot()")) &&
            mobilePageText.contains(QStringLiteral("function clearActiveHierarchySelection()")) &&
            mobilePageText.contains(QStringLiteral("function syncRouteSelectionState()")) &&
            mobilePageText.contains(QStringLiteral("onCompactLeadingActionRequested: mobileHierarchyPage.requestBackToHierarchy()")),
        "MobileHierarchyPage.qml must expose explicit hierarchy, note-list, editor, created-note, and editor-pop repair helpers while keeping the compact leading action wired to the shared back path and a dedicated hierarchy-selection reset helper.");
    QVERIFY2(
        mobilePageText.contains(QStringLiteral("LV.PageTransitionController {")) &&
            mobilePageText.contains(QStringLiteral("router: mobileScaffold.activePageRouter")) &&
            mobilePageText.contains(QStringLiteral("function beginBackSwipeGesture(eventData)")) &&
            mobilePageText.contains(QStringLiteral("function updateBackSwipeGesture(eventData)")) &&
            mobilePageText.contains(QStringLiteral("function finishBackSwipeGesture(eventData, cancelled)")) &&
            mobilePageText.contains(QStringLiteral("mobileHierarchyPage.rememberNoteListSelection();")) &&
            mobilePageText.contains(QStringLiteral("mobileHierarchyPage.verifyCommittedEditorPopState(repairRequestId, 2);")) &&
            mobilePageText.contains(QStringLiteral("const depth = mobileHierarchyPage.routeStackDepth();")) &&
            mobilePageText.contains(QStringLiteral("if (currentPath === mobileHierarchyPage.noteListRoutePath && depth >= 2)")) &&
            mobilePageText.contains(QStringLiteral("sessionId === mobileHierarchyPage.backSwipeConsumedSessionId")) &&
            mobilePageText.contains(QStringLiteral("mobileHierarchyPage.routeSelectionSyncSuppressed = true;")) &&
            mobilePageText.contains(QStringLiteral("mobileHierarchyPage.restoreNoteListSelection(preservedSelectionIndex);")) &&
            !mobilePageText.contains(QStringLiteral("mobileHierarchyPage.backSwipeSessionId < 0 && !mobileHierarchyPage.beginBackSwipeGesture(eventData)")) &&
            mobilePageText.contains(QStringLiteral("pageTransitionController.shouldCommit(")),
        "MobileHierarchyPage.qml must centralize mobile back-swipe state through LV.PageTransitionController while preventing the same touch session from reopening a second back-swipe after a committed or cancelled pop, and any canonical note-list/editor rebuild must preserve the active folder selection instead of falling back to All Library.");
    QVERIFY2(
        mobilePageText.contains(QStringLiteral("onHierarchyItemActivated: function (item, itemId, index)")) &&
            mobilePageText.contains(QStringLiteral("mobileHierarchyPage.requestOpenNoteList(item, itemId, index);")) &&
            mobilePageText.contains(QStringLiteral("mobileScaffold.activePageRouter.push(mobileHierarchyPage.noteListRoutePath);")),
        "MobileHierarchyPage.qml must transition into the mobile note-list body by pushing the note-list route onto the LV.PageRouter stack.");
    QVERIFY2(
        mobilePageText.contains(QStringLiteral("id: noteListBodyComponent")) &&
            mobilePageText.contains(QStringLiteral("PanelView.ListBarLayout {")) &&
            mobilePageText.contains(QStringLiteral("activeToolbarIndex: mobileHierarchyPage.activeToolbarIndex")) &&
            mobilePageText.contains(QStringLiteral("headerVisible: false")) &&
            mobilePageText.contains(QStringLiteral("noteListModel: mobileHierarchyPage.activeNoteListModel")) &&
            mobilePageText.contains(QStringLiteral("searchText: mobileHierarchyPage.statusSearchText")),
        "MobileHierarchyPage.qml must reuse ListBarLayout with the shared note-list model while hiding the desktop-only list header so the compact status bar owns mobile search.");
    QVERIFY2(
        mobilePageText.contains(QStringLiteral("onNoteActivated: function (index, noteId) {")) &&
            mobilePageText.contains(QStringLiteral("mobileHierarchyPage.requestOpenEditor(noteId, index);")) &&
            mobilePageText.contains(QStringLiteral("mobileScaffold.activePageRouter.push(mobileHierarchyPage.editorRoutePath);")),
        "MobileHierarchyPage.qml must transition from the mobile note-list body into the mobile editor body when a shared note card is activated.");
    QVERIFY2(
        mobilePageText.contains(QStringLiteral("readonly property var libraryNoteCreationViewModel: mobileHierarchyPage.windowInteractions")) &&
            mobilePageText.contains(QStringLiteral("property string pendingCreatedNoteId: \"\"")) &&
            mobilePageText.contains(QStringLiteral("target: mobileHierarchyPage.libraryNoteCreationViewModel")) &&
            mobilePageText.contains(QStringLiteral("function onEmptyNoteCreated(noteId) {")) &&
            mobilePageText.contains(QStringLiteral("mobileHierarchyPage.scheduleCreatedNoteEditorRoute(noteId);")) &&
            mobilePageText.contains(QStringLiteral("onActiveContentViewModelChanged: mobileHierarchyPage.routePendingCreatedNoteToEditor()")) &&
            mobilePageText.contains(QStringLiteral("mobileHierarchyPage.routePendingCreatedNoteToEditor();")) &&
            mobilePageText.contains(QStringLiteral("mobileHierarchyPage.requestOpenEditor(pendingNoteId, -1);")),
        "MobileHierarchyPage.qml must capture LibraryHierarchyViewModel.emptyNoteCreated(noteId) and promote the created note into the mobile editor route once the shared library models resolve.");
    QVERIFY2(
        mobilePageText.contains(QStringLiteral("id: editorBodyComponent")) &&
            mobilePageText.contains(QStringLiteral("PanelView.ContentViewLayout {")) &&
            mobilePageText.contains(QStringLiteral("contentViewModel: mobileHierarchyPage.activeContentViewModel")) &&
            mobilePageText.contains(QStringLiteral("noteListModel: mobileHierarchyPage.activeNoteListModel")) &&
            mobilePageText.contains(QStringLiteral("drawerVisible: false")) &&
            mobilePageText.contains(QStringLiteral("minimapVisible: false")) &&
            mobilePageText.contains(QStringLiteral("gutterColor: \"transparent\"")) &&
            mobilePageText.contains(QStringLiteral("gutterWidthOverride: LV.Theme.gap20 * 2")) &&
            !mobilePageText.contains(QStringLiteral("editorTopInsetOverride:")),
        "MobileHierarchyPage.qml must reuse ContentViewLayout for mobile editing while inheriting the shared 16px editor top inset instead of overriding it away.");
    QVERIFY2(
        mobilePageText.contains(QStringLiteral("const preservedSelectionIndex = mobileHierarchyPage.rememberNoteListSelection(itemId);")) &&
            mobilePageText.contains(QStringLiteral("const preservedSelectionIndex = mobileHierarchyPage.rememberNoteListSelection();")) &&
            mobilePageText.contains(QStringLiteral("if (mobileHierarchyPage.routeSelectionSyncSuppressed)")) &&
            mobilePageText.contains(QStringLiteral("mobileHierarchyPage.routeToCanonicalNoteList(preservedSelectionIndex);")) &&
            mobilePageText.contains(QStringLiteral("mobileHierarchyPage.routeToCanonicalEditor(preservedSelectionIndex);")),
        "MobileHierarchyPage.qml must snapshot the active hierarchy selection before opening or repairing note-list/editor routes, and it must suppress the temporary hierarchy-route deselection pass while canonical stack rebuilds run.");
    QVERIFY2(
            mobilePageText.contains(QStringLiteral("id: backSwipeEdgeZone")) &&
            mobilePageText.contains(QStringLiteral("visible: mobileHierarchyPage.backNavigationAvailable")) &&
            mobilePageText.contains(QStringLiteral("width: visible ? mobileHierarchyPage.backSwipeEdgeWidth : 0")) &&
            mobilePageText.contains(QStringLiteral("function backSwipeGestureEventData(localX, localY, totalDeltaX, totalDeltaY, sessionId)")) &&
            mobilePageText.contains(QStringLiteral("id: backSwipeDragHandler")) &&
            mobilePageText.contains(QStringLiteral("DragHandler {")) &&
            mobilePageText.contains(QStringLiteral("acceptedDevices: PointerDevice.TouchScreen")) &&
            mobilePageText.contains(QStringLiteral("grabPermissions: PointerHandler.CanTakeOverFromAnything")) &&
            !mobilePageText.contains(QStringLiteral("trigger: \"touchStarted\"")) &&
            !mobilePageText.contains(QStringLiteral("trigger: \"touchUpdated\"")) &&
            !mobilePageText.contains(QStringLiteral("trigger: \"touchEnded\"")) &&
            !mobilePageText.contains(QStringLiteral("trigger: \"touchCancelled\"")) &&
            !mobilePageText.contains(QStringLiteral("LV.EventListener {")),
        "MobileHierarchyPage.qml must keep mobile back swipe edge-local through a touch DragHandler so the shell does not subscribe to global LVRS gesture runtime events that would pre-empt native editor text-selection gestures.");
    QVERIFY2(
        mobilePageText.contains(QStringLiteral("onActiveNoteListModelChanged: {")) &&
            mobilePageText.contains(QStringLiteral("mobileHierarchyPage.routeToHierarchyRoot();")),
        "MobileHierarchyPage.qml must fall back to the hierarchy route when the resolved note-list model disappears.");
    QVERIFY2(
        mobilePageText.contains(QStringLiteral("property string lastObservedRoutePath: hierarchyRoutePath")) &&
            mobilePageText.contains(QStringLiteral("target: mobileScaffold.activePageRouter")) &&
            mobilePageText.contains(QStringLiteral("function onCurrentPathChanged() {")) &&
            mobilePageText.contains(QStringLiteral("mobileHierarchyPage.syncRouteSelectionState();")) &&
            mobilePageText.contains(QStringLiteral("mobileHierarchyPage.activeContentViewModel.setSelectedIndex(-1);")),
        "MobileHierarchyPage.qml must clear the active hierarchy selection whenever routing returns to the hierarchy root so tapping the same folder can reopen the note-list route after a mobile back gesture.");

    const QString listBarLayoutPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/ListBarLayout.qml"));
    QFile listBarLayoutFile(listBarLayoutPath);
    QVERIFY2(listBarLayoutFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(listBarLayoutPath));
    const QString listBarLayoutText = QString::fromUtf8(listBarLayoutFile.readAll());
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("property bool headerVisible: true")) &&
            listBarLayoutText.contains(
                QStringLiteral("Layout.preferredHeight: listBarLayout.headerVisible ? 24 : 0")) &&
            listBarLayoutText.contains(QStringLiteral("visible: listBarLayout.headerVisible")),
        "ListBarLayout.qml must expose a headerVisible switch so mobile note-list pages can reuse the shared note-model wiring without mounting the desktop header frame.");
}

void QmlBindingSyntaxGuardTest::noteListDeleteShortcutWiring_mustStayCentralized()
{
    const QDir testsDir(QStringLiteral(QT_TESTCASE_SOURCEDIR));
    const QString qmlRoot = testsDir.absoluteFilePath(QStringLiteral("../src/app/qml"));

    const QString mainQmlPath = QDir(qmlRoot).absoluteFilePath(QStringLiteral("Main.qml"));
    QFile mainQmlFile(mainQmlPath);
    QVERIFY2(mainQmlFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(mainQmlPath));
    const QString mainQmlText = QString::fromUtf8(mainQmlFile.readAll());
    QVERIFY2(
        mainQmlText.contains(QStringLiteral("noteDeletionViewModel: applicationWindow.rootLibraryHierarchyViewModel")),
        "Main.qml must inject the centralized library delete-note command source into BodyLayout.");

    const QString bodyLayoutPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/BodyLayout.qml"));
    QFile bodyLayoutFile(bodyLayoutPath);
    QVERIFY2(bodyLayoutFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(bodyLayoutPath));
    const QString bodyLayoutText = QString::fromUtf8(bodyLayoutFile.readAll());
    QVERIFY2(
        bodyLayoutText.contains(QStringLiteral("property var noteDeletionViewModel: null")),
        "BodyLayout.qml must accept a dedicated delete-note command source instead of hardwiring note deletion inside ListBarLayout.");
    QVERIFY2(
        bodyLayoutText.contains(QStringLiteral("noteDeletionViewModel: hStack.noteDeletionViewModel")),
        "BodyLayout.qml must forward the centralized delete-note command source into ListBarLayout.");

    const QString listBarLayoutPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/ListBarLayout.qml"));
    QFile listBarLayoutFile(listBarLayoutPath);
    QVERIFY2(listBarLayoutFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(listBarLayoutPath));
    const QString listBarLayoutText = QString::fromUtf8(listBarLayoutFile.readAll());
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral(
            "readonly property bool noteDeletionContractAvailable: noteDeletionBridge.deleteContractAvailable && noteDeletionBridge.focusedNoteAvailable")),
        "ListBarLayout.qml must expose an explicit delete-note capability contract through FocusedNoteDeletionBridge instead of probing the destructive command inline.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("function deleteCurrentNote()")),
        "ListBarLayout.qml must centralize current-note deletion through an explicit helper.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("FocusedNoteDeletionBridge {")),
        "ListBarLayout.qml must compose FocusedNoteDeletionBridge so delete-key handling tracks the visually focused note directly.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("const deleted = noteDeletionBridge.deleteFocusedNote();")),
        "ListBarLayout.qml must route destructive note deletion through FocusedNoteDeletionBridge.");
    QVERIFY2(
        listBarLayoutText.contains(
            QStringLiteral("listBarLayout.activateNoteIndex(noteItemDelegate.index, noteCard.noteId);")),
        "ListBarLayout.qml must capture the tapped note id immediately so keyboard deletion follows the visual note focus without latency.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("event.key !== Qt.Key_Backspace && event.key !== Qt.Key_Delete")),
        "ListBarLayout.qml must handle both Backspace and Delete when the note list owns keyboard focus.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("noteListView.forceActiveFocus();")),
        "ListBarLayout.qml must return keyboard focus to the note list after tap or delete actions so repeated keyboard deletion keeps working.");
}

QTEST_APPLESS_MAIN(QmlBindingSyntaxGuardTest)

#include "test_qml_binding_syntax_guard.moc"
