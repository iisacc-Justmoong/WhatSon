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
        contentViewText.contains(QStringLiteral("readonly property int gutterWidth: 74")),
        "ContentViewLayout.qml must keep the 74px line-number gutter width from the Figma frame.");
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
        contentViewText.contains(QStringLiteral("value: contentsView.editorTopInset")),
        "ContentViewLayout.qml must anchor body text to the 48px top inset.");
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
        contentViewText.contains(QStringLiteral("editorSession.scheduleEditorPersistence();")),
        "ContentsDisplayView.qml must delegate save scheduling through the dedicated editor session.");
    QVERIFY2(
        contentViewText.contains(
            QStringLiteral("readonly property string selectedNoteBodyText: selectionBridge.selectedNoteBodyText")),
        "ContentsDisplayView.qml must source selected note text from the selection adapter.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("onSelectedNoteBodyTextChanged: {")),
        "ContentViewLayout.qml must resync editor text when the selected note body changes.");
    QVERIFY2(
        contentViewText.contains(
            QStringLiteral(
                "editorSession.syncEditorTextFromSelection(contentsView.selectedNoteId, contentsView.selectedNoteBodyText);")),
        "ContentViewLayout.qml must sync incoming note bodies through the guarded editor session.");
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
        contentViewText.contains(QStringLiteral("contentsView.editorTextEdited(text);")),
        "ContentViewLayout.qml must emit editorTextEdited(text) from TextEditor edits.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("anchors.topMargin: contentsView.editorTopInset")),
        "ContentViewLayout.qml placeholder label must also align to the top inset.");
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
                "readonly property int currentHierarchy: hierarchyView.sidebarHierarchyViewModel.resolvedActiveHierarchyIndex")),
        "HierarchySidebarLayout.qml must source the active hierarchy index directly from SidebarHierarchyViewModel.");
    QVERIFY2(
        sidebarLayoutText.contains(
            QStringLiteral(
                "readonly property var resolvedHierarchyViewModel: hierarchyView.sidebarHierarchyViewModel.resolvedHierarchyViewModel")),
        "HierarchySidebarLayout.qml must source the active hierarchy view-model directly from SidebarHierarchyViewModel.");
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
        sidebarLayoutText.contains(QStringLiteral("hierarchyView.setActiveHierarchyIndex(index);")),
        "HierarchySidebarLayout.qml must route toolbar index updates through the normalized setter helper.");

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
        mainQmlText.contains(QStringLiteral("pageInitialPath: workspaceRoutePath")),
        "Main.qml must drive initial shell state through pageInitialPath.");
    QVERIFY2(
        mainQmlText.contains(QStringLiteral("pageRoutes: [workspaceShellRoute]")),
        "Main.qml must expose the workspace shell through pageRoutes.");
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
        mainQmlText.contains(QStringLiteral("sidebarHierarchyViewModel: applicationWindow.sidebarHierarchyVm")),
        "Main.qml must forward sidebarHierarchyViewModel to BodyLayout.");
    QVERIFY2(
        mainQmlText.contains(QStringLiteral("noteDeletionViewModel: applicationWindow.libraryHierarchyVm")),
        "Main.qml must inject the centralized library delete-note command source into BodyLayout.");
    QVERIFY2(
        mainQmlText.contains(QStringLiteral("MainWindowInteractionController {")),
        "Main.qml must delegate root interaction policy to a dedicated MainWindowInteractionController.");

    const QString sidebarViewPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/sidebar/SidebarHierarchyView.qml"));
    QFile sidebarViewFile(sidebarViewPath);
    QVERIFY2(sidebarViewFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(sidebarViewPath));
    const QString sidebarViewText = QString::fromUtf8(sidebarViewFile.readAll());
    const QString sidebarControllerPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/sidebar/SidebarHierarchyInteractionController.qml"));
    QFile sidebarControllerFile(sidebarControllerPath);
    QVERIFY2(sidebarControllerFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(sidebarControllerPath));
    const QString sidebarControllerText = QString::fromUtf8(sidebarControllerFile.readAll());
    const QString hierarchyListCompatPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/sidebar/HierarchyListCompat.qml"));
    QFile hierarchyListCompatFile(hierarchyListCompatPath);
    QVERIFY2(hierarchyListCompatFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(hierarchyListCompatPath));
    const QString hierarchyListCompatText = QString::fromUtf8(hierarchyListCompatFile.readAll());

    QVERIFY2(
        sidebarViewText.contains(QStringLiteral(
            "readonly property var folderModel: hierarchyViewModel ? hierarchyViewModel.itemModel : null")),
        "SidebarHierarchyView.qml must source folder model directly from hierarchyViewModel.itemModel.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("SidebarHierarchyInteractionController {")),
        "SidebarHierarchyView.qml must compose a dedicated interaction controller for rename and drag-drop state.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("autoSelectFirstItem: false")),
        "SidebarHierarchyView.qml must disable LVRS first-item auto-activation so blank-area deselect remains visually cleared.");
    QVERIFY2(
        !sidebarViewText.contains(QStringLiteral("sidebarHierarchyView.activeToolbarIndex = index;")),
        "SidebarHierarchyView.qml must not overwrite activeToolbarIndex locally.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("Keys.onPressed: function (event)")),
        "SidebarHierarchyView.qml must handle Enter/Return rename trigger from keyboard.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("event.key !== Qt.Key_Return && event.key !== Qt.Key_Enter")),
        "SidebarHierarchyView.qml keyboard rename handler must gate Return/Enter keys.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("onDoubleTapped:")),
        "SidebarHierarchyView.qml must expose double-tap rename trigger on hierarchy rows.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function activateSelectedHierarchyItem(focusView)")),
        "SidebarHierarchyView.qml must expose activateSelectedHierarchyItem(focusView) for programmatic focus sync.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function activateHierarchyDelegate(delegate, index)")),
        "SidebarHierarchyView.qml must centralize hierarchy-row activation so drag and selection do not compete through duplicated tap handlers.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function clearHierarchySelection()")),
        "SidebarHierarchyView.qml must expose a dedicated blank-area deselect path instead of leaving hierarchy row activation sticky.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("onActiveChanged: function (item, itemId, index)")),
        "SidebarHierarchyView.qml must sync hierarchy selection from LVRS activeChanged so activation happens after click/release instead of raw press.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("gesturePolicy: TapHandler.DragThreshold")),
        "SidebarHierarchyView.qml row tap handling must keep drag-threshold gesture gating so click activation and folder drag can coexist.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("property bool folderDragActive: false")),
        "SidebarHierarchyView.qml must track whether a hierarchy-folder drag is active so the viewport can stop treating the gesture as scrolling.");
    QVERIFY2(
        sidebarViewText.contains(
            QStringLiteral("interactive: contentHeight > height && !sidebarHierarchyView.folderDragActive")),
        "SidebarHierarchyView.qml must disable Flickable scrolling while a folder drag is active.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("manualActivationOnly: true")),
        "SidebarHierarchyView.qml must suppress LVRS raw-press activation and leave authoritative row selection to the tap-release path.");
    QVERIFY2(
        hierarchyListCompatText.contains(QStringLiteral("property bool manualActivationOnly: false")),
        "HierarchyListCompat.qml must expose manualActivationOnly so SidebarHierarchyView can explicitly suppress raw LVRS press activation without crashing at load time.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral(
            "onTapped: {\n                                    sidebarHierarchyView.activateHierarchyDelegate(hierarchyDelegate, index);")),
        "SidebarHierarchyView.qml must explicitly activate hierarchy rows on tap release so rapid clicks are not lost behind LVRS refresh timing.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("itemKey: hierarchyDelegate.itemKeyValue")),
        "SidebarHierarchyView.qml must forward a stable itemKey into LVRS HierarchyItem instead of relying on transient row indexes.");
    QVERIFY2(
        hierarchyListCompatText.contains(QStringLiteral("return items;")),
        "HierarchyListCompat.qml must return its managed LVRS hierarchy items so activeItem remains single-sourced.");
    QVERIFY2(
        hierarchyListCompatText.contains(QStringLiteral("control.activeItem = item;")),
        "HierarchyListCompat.qml must keep a single activeItem instead of leaving selection state distributed across delegates.");
    QVERIFY2(
        hierarchyListCompatText.contains(QStringLiteral("function activateByKey(itemKey)")),
        "HierarchyListCompat.qml must preserve the LVRS activateByKey lookup path so hierarchy activation can stay keyed to stable identities.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function canMoveFolder(index)")),
        "SidebarHierarchyView.qml must expose canMoveFolder(index) wrapper for folder drag gating.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function canMoveFolderToRoot(sourceIndex)")),
        "SidebarHierarchyView.qml must expose canMoveFolderToRoot(sourceIndex) wrapper for root extraction gating.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function canAcceptFolderDrop(sourceIndex, targetIndex, asChild)")),
        "SidebarHierarchyView.qml must expose canAcceptFolderDrop(...) wrapper for folder drop gating.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function canAcceptFolderDropBefore(sourceIndex, targetIndex)")),
        "SidebarHierarchyView.qml must expose canAcceptFolderDropBefore(...) wrapper for before-insert folder drops.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral(
            "sidebarHierarchyView.noteDropTargetIndex = sidebarHierarchyView.canAcceptNoteDrop(index, noteId) ? index : -1;")),
        "SidebarHierarchyView.qml must derive note-drop highlight state from hierarchyViewModel.canAcceptNoteDrop.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("Drag.keys: [\"whatson.hierarchy.folder\"]")),
        "SidebarHierarchyView.qml hierarchy delegates must advertise whatson.hierarchy.folder drag keys.");
    QVERIFY2(
        sidebarControllerText.contains(QStringLiteral("interactionController.hierarchyList.clearActiveItem()")),
        "SidebarHierarchyInteractionController.qml blank-area deselect must clear the active LVRS hierarchy item explicitly.");
    QVERIFY2(
        sidebarControllerText.contains(QStringLiteral("property int pendingActivationIndex: -1")),
        "SidebarHierarchyInteractionController.qml must track one pending hierarchy activation so the latest user click can survive intermediate refreshes.");
    QVERIFY2(
        sidebarControllerText.contains(QStringLiteral("property int activationRequestRevision: 0")),
        "SidebarHierarchyInteractionController.qml must version hierarchy activation requests so stale replay work is discarded.");
    QVERIFY2(
        sidebarControllerText.contains(QStringLiteral("Qt.callLater(function () {")),
        "SidebarHierarchyInteractionController.qml must replay the latest hierarchy activation after refresh-triggering work settles.");
    QVERIFY2(
        sidebarControllerText.contains(
            QStringLiteral("interactionController.hierarchyViewModel.setSelectedIndex(index)")),
        "SidebarHierarchyInteractionController.qml pending activation replay must be able to realign the hierarchy view-model selection to the latest user tap.");
    QVERIFY2(
        sidebarControllerText.contains(
            QStringLiteral("interactionController.hierarchyList.requestActivate(delegate, true)")),
        "SidebarHierarchyInteractionController.qml must bypass manual-activation suppression when it performs the authoritative hierarchy activation.");
    QVERIFY2(
        !sidebarViewText.contains(QStringLiteral(
            "onPressed: {\n                                sidebarHierarchyView.activateHierarchyDelegate(hierarchyDelegate, index);")),
        "SidebarHierarchyView.qml must not force hierarchy selection on raw press for drag-capable rows.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("hierarchyViewModel.setSelectedIndex(index)")),
        "SidebarHierarchyView.qml must mirror LVRS active item changes into the bound hierarchy view-model.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("contentHeight: Math.max(height, hierarchyList.implicitHeight)")),
        "SidebarHierarchyView.qml must reserve a clickable blank viewport area below short hierarchies.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("anchors.top: hierarchyList.bottom")),
        "SidebarHierarchyView.qml blank-area deselect zone must start immediately below the last hierarchy row.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("grabPermissions: PointerHandler.CanTakeOverFromAnything")),
        "SidebarHierarchyView.qml folder drag handler must be able to take pointer ownership away from row click handlers and Flickable scrolling.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("dragThreshold: 4")),
        "SidebarHierarchyView.qml folder drag handler should lower the drag threshold so folder drags start before viewport scrolling wins.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("dragPreviewActive: folderDragHandler.active")),
        "SidebarHierarchyView.qml must drive LVRS HierarchyItem dragPreviewActive during folder drags so users see a visible grabbed state.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("Drag.supportedActions: Qt.MoveAction")),
        "SidebarHierarchyView.qml folder drag contract must advertise move semantics.");
    QVERIFY2(
        sidebarControllerText.contains(QStringLiteral("interactionController.hierarchyViewModel.canAcceptFolderDrop")),
        "SidebarHierarchyInteractionController.qml must query hierarchyViewModel.canAcceptFolderDrop for folder reparenting.");
    QVERIFY2(
        sidebarControllerText.contains(
            QStringLiteral("interactionController.hierarchyViewModel.canAcceptFolderDropBefore")),
        "SidebarHierarchyInteractionController.qml must query hierarchyViewModel.canAcceptFolderDropBefore for before-insert drops.");
    QVERIFY2(
        sidebarControllerText.contains(
            QStringLiteral("interactionController.hierarchyViewModel.moveFolder(sourceIndex, targetIndex, asChild)")),
        "SidebarHierarchyInteractionController.qml must route folder reparent drops through hierarchyViewModel.moveFolder(...).");
    QVERIFY2(
        sidebarControllerText.contains(
            QStringLiteral("interactionController.hierarchyViewModel.moveFolderBefore(sourceIndex, targetIndex)")),
        "SidebarHierarchyInteractionController.qml must route before-insert drops through hierarchyViewModel.moveFolderBefore(...).");
    QVERIFY2(
        sidebarControllerText.contains(
            QStringLiteral("interactionController.hierarchyViewModel.moveFolderToRoot(sourceIndex)")),
        "SidebarHierarchyInteractionController.qml must expose folder extraction to root through hierarchyViewModel.moveFolderToRoot(...).");
    QVERIFY2(
        sidebarControllerText.contains(
            QStringLiteral("interactionController.hierarchyViewModel.assignNoteToFolder(index, noteId)")),
        "SidebarHierarchyInteractionController.qml must route accepted note drops through hierarchyViewModel.assignNoteToFolder(index, noteId).");
    QVERIFY2(
        sidebarViewText.contains(
            QStringLiteral("property alias noteDropTargetIndex: sidebarController.noteDropTargetIndex")),
        "SidebarHierarchyView.qml must expose note drop state through the dedicated interaction controller.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("folderDropBefore = true")),
        "SidebarHierarchyView.qml must track before-insert drag state separately from child/after drops.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("sidebarController.notifyAcceptedInteraction(\"move-folder-before\")")),
        "SidebarHierarchyView.qml must emit a dedicated hook reason for before-insert folder drops through the interaction controller.");

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
        noteListItemText.contains(QStringLiteral("implicitHeight: 102")),
        "NoteListItem.qml must keep the Figma 102px outer frame height.");
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
        noteListItemText.contains(QStringLiteral("text: noteListItem.resolvedDisplayDate")),
        "NoteListItem.qml date label must render the resolved display date.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("font.pixelSize: noteListItem.primaryTextSize")),
        "NoteListItem.qml primary text must use the dedicated 12px Figma size property.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("font.pixelSize: noteListItem.metadataTextSize")),
        "NoteListItem.qml metadata text must use the dedicated 11px Figma size property.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("property bool image: false")),
        "NoteListItem.qml must expose the image flag from the note-list model.");
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("property url imageSource: \"\"")),
        "NoteListItem.qml must expose the first resource thumbnail source as a url property.");
    QVERIFY2(
        noteListItemText.contains(
            QStringLiteral("Layout.preferredWidth: noteListItem.image ? noteListItem.imagePreviewSize : 0")),
        "NoteListItem.qml imageBox width must only reserve the 24px Figma slot when image=true.");
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
        listBarLayoutText.contains(QStringLiteral("event.key !== Qt.Key_Backspace && event.key !== Qt.Key_Delete")),
        "ListBarLayout.qml must handle both Backspace and Delete when the note list owns keyboard focus.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("noteListView.forceActiveFocus();")),
        "ListBarLayout.qml must return keyboard focus to the note list after tap or delete actions so repeated keyboard deletion keeps working.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral(
            "readonly property var roleModel: noteItemDelegate.model && typeof noteItemDelegate.model === \"object\" ? noteItemDelegate.model : ({})")),
        "ListBarLayout.qml note delegates must normalize the runtime role object once and bind fields directly.");
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
        listBarLayoutText.contains(QStringLiteral("function syncCurrentIndexFromModel()")),
        "ListBarLayout.qml must pull currentIndex from the note model back into ListView state.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("property bool noteDragActive: false")),
        "ListBarLayout.qml must track whether a note-card drag is active so the parent ListView can stop treating the gesture as scrolling.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("property int pressedNoteIndex: -1")),
        "ListBarLayout.qml must track a transient pressed note row separately from the committed currentIndex so drag startup does not mutate selection too early.");
    QVERIFY2(
        listBarLayoutText.contains(
            QStringLiteral("interactive: contentHeight > height && !listBarLayout.noteDragActive")),
        "ListBarLayout.qml must disable ListView scrolling while a note drag is active.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("model: listBarLayout.resolvedNoteListModel")),
        "ListBarLayout.qml must bind ListView only to the resolved note-list model.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("Drag.keys: [\"whatson.library.note\"]")),
        "ListBarLayout.qml note delegates must advertise whatson.library.note drag keys.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("Drag.supportedActions: Qt.CopyAction")),
        "ListBarLayout.qml note delegates must advertise additive note-to-folder drag semantics.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral(
            "image: roleModel.image === undefined ? false : Boolean(roleModel.image)")),
        "ListBarLayout.qml must bind the image flag directly from the normalized role object.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral(
            "imageSource: roleModel.imageSource === undefined || roleModel.imageSource === null ? \"\" : roleModel.imageSource")),
        "ListBarLayout.qml must bind imageSource directly from the normalized role object.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("grabPermissions: PointerHandler.CanTakeOverFromAnything")),
        "ListBarLayout.qml note drag handler must be able to take pointer ownership away from tap selection and list scrolling.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("dragThreshold: 4")),
        "ListBarLayout.qml note drag handler should lower the drag threshold so note-to-folder drags start before vertical scrolling wins.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("gesturePolicy: TapHandler.DragThreshold")),
        "ListBarLayout.qml must keep note-card tap handling passive until drag threshold so note drags can start from the same surface.");
    QVERIFY2(
        !listBarLayoutText.contains(QStringLiteral("TapHandler.ReleaseWithinBounds")),
        "ListBarLayout.qml note tap handler must not take an exclusive press grab that blocks drag startup.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("onPressedChanged: {")),
        "ListBarLayout.qml note tap handling must preserve a transient press preview before the drag threshold resolves.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("onTapped: {")),
        "ListBarLayout.qml note-card selection must be committed on tap release so note drags can begin from the same surface.");
    QVERIFY2(
        listBarLayoutText.contains(
            QStringLiteral("listBarLayout.activateNoteIndex(noteItemDelegate.index, noteItemDelegate.noteId);")),
        "ListBarLayout.qml note-card tap handler must route authoritative activation through activateNoteIndex().");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("function syncFocusedNoteDeletionState()")),
        "ListBarLayout.qml must expose a helper that resyncs focused note deletion state from the visible current card.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("listBarLayout.noteListModel.currentIndex = index;")),
        "ListBarLayout.qml must push tapped note selection into noteModel.currentIndex.");
    QVERIFY2(
        listBarLayoutText.contains(QStringLiteral("property int pendingSelectionIndex: -1")),
        "ListBarLayout.qml must track a pending user note-selection intent while editor save/refresh work settles.");
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
            QStringLiteral("readonly property color inlineFieldBackgroundColor: \"transparent\"")),
        "ListBarHeader.qml inline input must centralize the transparent inline background override.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("shapeStyle: shapeCylinder")),
        "ListBarHeader.qml inline input must use the cylindrical pill shape from Figma.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("readonly property int inlineFieldHeight: 18")),
        "ListBarHeader.qml inline input must keep the 18px height contract from Figma.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("readonly property int inlineFieldTextHeight: 12")),
        "ListBarHeader.qml inline input must keep the 12px text line box from Figma.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("readonly property int inlineFieldHorizontalInset: 7")),
        "ListBarHeader.qml inline input must use the 7px horizontal inset from Figma.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("readonly property int inlineFieldVerticalInset: 3")),
        "ListBarHeader.qml inline input must use the 3px vertical inset from Figma.");
    QVERIFY2(
        listBarHeaderText.contains(QStringLiteral("readonly property int resolvedInputTextHeight: Math.max(")),
        "ListBarHeader.qml must derive the live search text box height from the InputField contentHeight so whitespace edits stay vertically centered.");
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
                "readonly property int activeHierarchyIndex: hStack.sidebarHierarchyViewModel.resolvedActiveHierarchyIndex")),
        "BodyLayout.qml must source the active hierarchy index directly from SidebarHierarchyViewModel.");
    QVERIFY2(
        bodyLayoutText.contains(
            QStringLiteral(
                "readonly property var activeHierarchyViewModel: hStack.sidebarHierarchyViewModel.resolvedHierarchyViewModel")),
        "BodyLayout.qml must source the active hierarchy view-model directly from SidebarHierarchyViewModel.");
    QVERIFY2(
        bodyLayoutText.contains(
            QStringLiteral(
                "readonly property var activeNoteListModel: hStack.sidebarHierarchyViewModel.resolvedNoteListModel")),
        "BodyLayout.qml must source the active note-list model directly from SidebarHierarchyViewModel.");
    QVERIFY2(
        bodyLayoutText.contains(QStringLiteral("property var noteDeletionViewModel: null")),
        "BodyLayout.qml must accept a dedicated delete-note command source instead of hardwiring note deletion inside ListBarLayout.");
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
        bodyLayoutText.contains(QStringLiteral("contentViewModel: hStack.activeHierarchyViewModel")),
        "BodyLayout.qml must pass the resolved active hierarchy view-model into the content surface.");
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

void QmlBindingSyntaxGuardTest::noteListDeleteShortcutWiring_mustStayCentralized()
{
    const QDir testsDir(QStringLiteral(QT_TESTCASE_SOURCEDIR));
    const QString qmlRoot = testsDir.absoluteFilePath(QStringLiteral("../src/app/qml"));

    const QString mainQmlPath = QDir(qmlRoot).absoluteFilePath(QStringLiteral("Main.qml"));
    QFile mainQmlFile(mainQmlPath);
    QVERIFY2(mainQmlFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(mainQmlPath));
    const QString mainQmlText = QString::fromUtf8(mainQmlFile.readAll());
    QVERIFY2(
        mainQmlText.contains(QStringLiteral("noteDeletionViewModel: applicationWindow.libraryHierarchyVm")),
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
            QStringLiteral("listBarLayout.activateNoteIndex(noteItemDelegate.index, noteItemDelegate.noteId);")),
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
