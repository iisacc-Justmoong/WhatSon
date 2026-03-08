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
} // namespace

class QmlBindingSyntaxGuardTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void bindingBlocks_mustNotContainStandaloneStringLiteral();
    void contentView_mustComposeTextEditorGutter();
    void hierarchySidebarWiring_mustBindLoaderAndToolbarTarget();
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

void QmlBindingSyntaxGuardTest::contentView_mustComposeTextEditorGutter()
{
    const QDir testsDir(QStringLiteral(QT_TESTCASE_SOURCEDIR));
    const QString qmlRoot = testsDir.absoluteFilePath(QStringLiteral("../src/app/qml"));

    const QString contentViewPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/ContentViewLayout.qml"));
    QFile contentViewFile(contentViewPath);
    QVERIFY2(contentViewFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(contentViewPath));
    const QString contentViewText = QString::fromUtf8(contentViewFile.readAll());

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
        contentViewText.contains(QStringLiteral("property var contentViewModel: null")),
        "ContentViewLayout.qml must accept the active hierarchy view-model for body persistence.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("property bool pendingBodySave: false")),
        "ContentViewLayout.qml must track debounced body persistence state.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function visibleLineNumbers()")),
        "ContentViewLayout.qml must derive visible line numbers for the gutter viewport.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("const firstVisibleLine = contentsView.firstVisibleLogicalLine();")),
        "ContentViewLayout.qml must choose the initial visible gutter line from viewport-space line positions.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral(
            "readonly property var logicalLineStartOffsets: contentsView.normalizeLogicalLineOffsets(")),
        "ContentViewLayout.qml must track logical-line offsets independently from wrapped visual rows.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function buildLogicalLineStartOffsets(text)")),
        "ContentViewLayout.qml must derive logical line starts for wrap-safe gutter numbering.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("return offsets;")),
        "ContentViewLayout.qml must return the computed logical-line offsets so gutter numbers remain visible.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function logicalLineNumberForOffset(offset)")),
        "ContentViewLayout.qml must resolve cursor line numbers from the logical-line offset table instead of reparsing the whole document every keypress.");
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
        contentViewText.contains(QStringLiteral("function normalizeGutterMarker(markerSpec)")),
        "ContentViewLayout.qml must normalize externally supplied gutter marker specs.");
    QVERIFY2(
        contentViewText.contains(
            QStringLiteral("readonly property int lineNumberRightInset: contentsView.editorHorizontalInset")),
        "ContentViewLayout.qml must right-align gutter numbers against the editor inset.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral(
            "readonly property int lineNumberColumnTextWidth: contentsView.gutterWidth - contentsView.lineNumberColumnLeft - contentsView.lineNumberRightInset")),
        "ContentViewLayout.qml must widen the gutter text column up to the mirrored editor inset.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("x: contentsView.lineNumberColumnLeft")),
        "ContentViewLayout.qml must keep the gutter number column anchored from the left rail origin.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("model: contentsView.effectiveGutterMarkers")),
        "ContentViewLayout.qml must render the effective gutter marker model instead of decorative placeholders.");
    QVERIFY2(
        contentViewText.contains(
            QStringLiteral("markerType !== \"changed\" && markerType !== \"conflict\" && markerType !== \"current\"")),
        "ContentViewLayout.qml gutter marker contract must explicitly validate current/changed/conflict marker types.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("LV.TextEditor {")),
        "ContentViewLayout.qml must compose LV.TextEditor for the contents editing surface.");
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
        contentViewText.contains(QStringLiteral("function scheduleEditorPersistence()")),
        "ContentViewLayout.qml must debounce body persistence instead of rewriting files on every keystroke.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("saveBodyTextForNote(noteId, text)")),
        "ContentViewLayout.qml must persist body edits against an explicit note id so selection changes do not misroute pending saves.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("Component.onDestruction: contentsView.flushPendingEditorText()")),
        "ContentViewLayout.qml must flush pending body edits when the surface is torn down.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("Timer {")),
        "ContentViewLayout.qml must use a Timer-backed debounce for body persistence.");
    QVERIFY2(
        contentViewText.contains(
            QStringLiteral("readonly property var editorFlickable: contentsView.resolveEditorFlickable()")),
        "ContentViewLayout.qml must resolve the internal TextEditor flickable for minimap scrolling.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function resolveEditorFlickable()")),
        "ContentViewLayout.qml must expose a helper that resolves the internal editor flickable.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function scrollEditorViewportToMinimapPosition(localY)")),
        "ContentViewLayout.qml must expose a minimap scroll helper that repositions the editor viewport.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("id: minimapCanvas")),
        "ContentViewLayout.qml must render a right-side minimap canvas for the editor document overview.");
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
        contentViewText.contains(QStringLiteral("Layout.preferredWidth: contentsView.minimapOuterWidth")),
        "ContentViewLayout.qml minimap rail must reserve its own right-side layout width.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("anchors.rightMargin: contentsView.minimapTrackInset")),
        "ContentViewLayout.qml minimap track must sit inline against the right edge instead of using a framed centered rail.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("border.width: 0")),
        "ContentViewLayout.qml minimap viewport must remain borderless.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("color: contentsView.minimapViewportFillColor")),
        "ContentViewLayout.qml minimap viewport must use a subtle inline fill instead of a framed outline.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("width: contentsView.minimapCurrentLineWidth()")),
        "ContentViewLayout.qml current-line minimap indicator must follow the active visual-row silhouette width instead of spanning the whole rail.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("LV.WheelScrollGuard {")),
        "ContentViewLayout.qml minimap must route wheel scroll into the editor flickable.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("targetFlickable: contentsView.editorFlickable")),
        "ContentViewLayout.qml minimap wheel routing must target the resolved editor flickable.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("fontFamily: LV.Theme.fontBody")),
        "ContentViewLayout.qml must bind TextEditor typography to the LVRS body font family.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("fontLetterSpacing: 0")),
        "ContentViewLayout.qml must keep the Figma body token's zero letter spacing.");
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
        contentViewText.contains(QStringLiteral("property bool syncingEditorTextFromModel: false")),
        "ContentViewLayout.qml must guard model-driven body sync from recursive save attempts.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function persistEditorTextForNote(noteId, text)")),
        "ContentViewLayout.qml must route body persistence through a content view-model hook.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("function syncEditorTextFromSelection(noteId, text)")),
        "ContentViewLayout.qml must resync selection changes through an explicit editor-sync helper.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("return Boolean(contentViewModel.saveBodyTextForNote(noteId, text));")),
        "ContentViewLayout.qml must save editor text through contentViewModel.saveBodyTextForNote(noteId, text).");
    QVERIFY2(
        contentViewText.contains(QStringLiteral(
            "readonly property string selectedNoteBodyText: noteListModel && noteListModel.currentBodyText !== undefined ? String(noteListModel.currentBodyText) : \"\"")),
        "ContentViewLayout.qml must source editor text from noteListModel.currentBodyText.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("onSelectedNoteBodyTextChanged: {")),
        "ContentViewLayout.qml must resync editor text when the selected note body changes.");
    QVERIFY2(
        contentViewText.contains(
            QStringLiteral(
                "contentsView.syncEditorTextFromSelection(contentsView.selectedNoteId, contentsView.selectedNoteBodyText);")),
        "ContentViewLayout.qml must sync incoming note bodies through the guarded editor helper.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("if (contentsView.syncingEditorTextFromModel)")),
        "ContentViewLayout.qml must skip file persistence while replaying model-driven editor text.");
    QVERIFY2(
        contentViewText.contains(QStringLiteral("contentsView.scheduleEditorPersistence();")),
        "ContentViewLayout.qml must debounce persistence for user edits to the active note body.");
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
        sidebarLayoutText.contains(QStringLiteral("property var sidebarHierarchyViewModel: null")),
        "HierarchySidebarLayout.qml must declare sidebarHierarchyViewModel.");
    QVERIFY2(
        sidebarLayoutText.contains(
            QStringLiteral(
                "readonly property int currentHierarchy: normalizeHierarchyIndex(sidebarHierarchyViewModel && sidebarHierarchyViewModel.activeHierarchyIndex !== undefined ? sidebarHierarchyViewModel.activeHierarchyIndex : activeToolbarIndex)")),
        "HierarchySidebarLayout.qml must normalize currentHierarchy from sidebarHierarchyViewModel first.");
    QVERIFY2(
        sidebarLayoutText.contains(QStringLiteral("return normalizedIndex;")),
        "HierarchySidebarLayout.qml normalizeHierarchyIndex must explicitly return normalized valid index.");
    QVERIFY2(
        sidebarLayoutText.contains(QStringLiteral("activeToolbarIndex: hierarchyView.currentHierarchy")),
        "HierarchySidebarLayout.qml must bind sidebar activeToolbarIndex from currentHierarchy.");
    QVERIFY2(
        sidebarLayoutText.contains(
            QStringLiteral(
                "hierarchyViewModel: hierarchyView.sidebarHierarchyViewModel && hierarchyView.sidebarHierarchyViewModel.activeHierarchyViewModel !== undefined ? hierarchyView.sidebarHierarchyViewModel.activeHierarchyViewModel : hierarchyView.modelForHierarchy(hierarchyView.currentHierarchy)")),
        "HierarchySidebarLayout.qml must resolve hierarchyViewModel from sidebarHierarchyViewModel contract.");
    QVERIFY2(
        sidebarLayoutText.contains(
            QStringLiteral("hierarchyView.sidebarHierarchyViewModel.setActiveHierarchyIndex(nextIndex);")),
        "HierarchySidebarLayout.qml must route toolbar index updates through sidebarHierarchyViewModel interface.");

    const QString mainQmlPath = QDir(qmlRoot).absoluteFilePath(QStringLiteral("Main.qml"));
    QFile mainQmlFile(mainQmlPath);
    QVERIFY2(mainQmlFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(mainQmlPath));
    const QString mainQmlText = QString::fromUtf8(mainQmlFile.readAll());

    QVERIFY2(
        mainQmlText.contains(QStringLiteral("readonly property string activeMainLayout")),
        "Main.qml must declare activeMainLayout for platform layout branching.");
    QVERIFY2(
        mainQmlText.contains(QStringLiteral(
            "sourceComponent: applicationWindow.activeMainLayout === \"mobile\" ? mobileMainLayoutComponent : desktopMainLayoutComponent")),
        "Main.qml must select desktop/mobile root layout from activeMainLayout.");
    QVERIFY2(
        !mainQmlText.contains(QStringLiteral("useMobileMainLayout")),
        "Main.qml must not depend on legacy useMobileMainLayout helper.");
    QVERIFY2(
        mainQmlText.contains(QStringLiteral("sidebarHierarchyViewModel: applicationWindow.sidebarHierarchyVm")),
        "Main.qml must forward sidebarHierarchyViewModel to BodyLayout.");

    const QString sidebarViewPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/sidebar/SidebarHierarchyView.qml"));
    QFile sidebarViewFile(sidebarViewPath);
    QVERIFY2(sidebarViewFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(sidebarViewPath));
    const QString sidebarViewText = QString::fromUtf8(sidebarViewFile.readAll());

    QVERIFY2(
        sidebarViewText.contains(QStringLiteral(
            "readonly property var folderModel: hierarchyViewModel ? hierarchyViewModel.itemModel : null")),
        "SidebarHierarchyView.qml must source folder model directly from hierarchyViewModel.itemModel.");
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
        sidebarViewText.contains(QStringLiteral("function canMoveFolder(index)")),
        "SidebarHierarchyView.qml must expose canMoveFolder(index) wrapper for folder drag gating.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function canMoveFolderToRoot(sourceIndex)")),
        "SidebarHierarchyView.qml must expose canMoveFolderToRoot(sourceIndex) wrapper for root extraction gating.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("function canAcceptFolderDrop(sourceIndex, targetIndex, asChild)")),
        "SidebarHierarchyView.qml must expose canAcceptFolderDrop(...) wrapper for folder drop gating.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral(
            "sidebarHierarchyView.noteDropTargetIndex = sidebarHierarchyView.canAcceptNoteDrop(index, noteId) ? index : -1;")),
        "SidebarHierarchyView.qml must derive note-drop highlight state from hierarchyViewModel.canAcceptNoteDrop.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("Drag.keys: [\"whatson.hierarchy.folder\"]")),
        "SidebarHierarchyView.qml hierarchy delegates must advertise whatson.hierarchy.folder drag keys.");
    QVERIFY2(
        sidebarViewText.contains(QStringLiteral("sidebarHierarchyView.hierarchyViewModel.canAcceptFolderDrop")),
        "SidebarHierarchyView.qml must query hierarchyViewModel.canAcceptFolderDrop for folder reparenting.");
    QVERIFY2(
        sidebarViewText.contains(
            QStringLiteral("sidebarHierarchyView.hierarchyViewModel.moveFolder(sourceIndex, targetIndex, asChild)")),
        "SidebarHierarchyView.qml must route folder reparent drops through hierarchyViewModel.moveFolder(...).");
    QVERIFY2(
        sidebarViewText.contains(
            QStringLiteral("sidebarHierarchyView.hierarchyViewModel.moveFolderToRoot(sourceIndex)")),
        "SidebarHierarchyView.qml must expose folder extraction to root through hierarchyViewModel.moveFolderToRoot(...).");
    QVERIFY2(
        sidebarViewText.contains(
            QStringLiteral("sidebarHierarchyView.hierarchyViewModel.assignNoteToFolder(index, noteId)")),
        "SidebarHierarchyView.qml must route accepted note drops through hierarchyViewModel.assignNoteToFolder(index, noteId).");

    const QString listItemsPlaceholderPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/ListItemsPlaceholder.qml"));
    QFile listItemsPlaceholderFile(listItemsPlaceholderPath);
    QVERIFY2(
        listItemsPlaceholderFile.open(QIODevice::ReadOnly | QIODevice::Text),
        qPrintable(listItemsPlaceholderPath));
    const QString listItemsPlaceholderText = QString::fromUtf8(listItemsPlaceholderFile.readAll());
    QVERIFY2(
        listItemsPlaceholderText.contains(QStringLiteral("DragHandler {")),
        "ListItemsPlaceholder.qml must expose DragHandler on note delegates.");
    QVERIFY2(
        listItemsPlaceholderText.contains(QStringLiteral("Drag.keys: [\"whatson.library.note\"]")),
        "ListItemsPlaceholder.qml note delegates must advertise whatson.library.note drag keys.");
    QVERIFY2(
        listItemsPlaceholderText.contains(QStringLiteral("listItemsPlaceholder.noteModel.currentIndex = index;")),
        "ListItemsPlaceholder.qml must push tapped note selection into noteModel.currentIndex.");
    QVERIFY2(
        listItemsPlaceholderText.contains(QStringLiteral("function pushCurrentIndexToModel(index)")),
        "ListItemsPlaceholder.qml must expose an explicit helper that pushes ListView selection into the note model.");
    QVERIFY2(
        listItemsPlaceholderText.contains(QStringLiteral("function syncCurrentIndexFromModel()")),
        "ListItemsPlaceholder.qml must pull currentIndex from the note model back into ListView state.");
    QVERIFY2(
        listItemsPlaceholderText.contains(QStringLiteral("onNoteModelChanged: {")),
        "ListItemsPlaceholder.qml must resync the visible note selection whenever the active note model changes.");

    const QString noteListItemPath = QDir(qmlRoot).absoluteFilePath(QStringLiteral("view/panels/NoteListItem.qml"));
    QFile noteListItemFile(noteListItemPath);
    QVERIFY2(noteListItemFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(noteListItemPath));
    const QString noteListItemText = QString::fromUtf8(noteListItemFile.readAll());
    QVERIFY2(
        noteListItemText.contains(QStringLiteral("readonly property string displayDatePlaceholder: \"YYYY-MM-dd\"")),
        "NoteListItem.qml must expose the Figma date placeholder token.");
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
        listBarLayoutText.
        contains(QStringLiteral("listBarLayout.noteListModel.searchText = listBarLayout.searchText;")),
        "ListBarLayout.qml must push the active search text into the bound note list model.");

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

    const QString detailPanelPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/detail/DetailPanel.qml"));
    QFile detailPanelFile(detailPanelPath);
    QVERIFY2(detailPanelFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(detailPanelPath));
    const QString detailPanelText = QString::fromUtf8(detailPanelFile.readAll());
    QVERIFY2(
        detailPanelText.contains(QStringLiteral("readonly property var detailPanelVm: detailPanelViewModel")),
        "DetailPanel.qml must bind detailPanelVm from detailPanelViewModel context.");
    QVERIFY2(
        detailPanelText.contains(
            QStringLiteral(
                "readonly property var activeDetailContentVm: detailPanel.detailPanelVm ? detailPanel.detailPanelVm.activeContentViewModel : null")),
        "DetailPanel.qml must map active state to dedicated content view-model instance.");
    QVERIFY2(
        detailPanelText.contains(
            QStringLiteral(
                "toolbarButtonSpecs: detailPanel.detailPanelVm ? detailPanel.detailPanelVm.toolbarItems : []")),
        "DetailPanel.qml must source toolbar specs from C++ detailPanelViewModel.");
    QVERIFY2(
        detailPanelText.contains(QStringLiteral("detailPanel.detailPanelVm.requestStateChange(stateValue);")),
        "DetailPanel.qml must forward toolbar state changes to C++ detailPanelViewModel.");

    const QString detailToolbarPath = QDir(qmlRoot).absoluteFilePath(
        QStringLiteral("view/panels/detail/DetailPanelHeaderToolbar.qml"));
    QFile detailToolbarFile(detailToolbarPath);
    QVERIFY2(detailToolbarFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(detailToolbarPath));
    const QString detailToolbarText = QString::fromUtf8(detailToolbarFile.readAll());
    QVERIFY2(
        detailToolbarText.contains(QStringLiteral("signal detailStateChangeRequested(int stateValue)")),
        "DetailPanelHeaderToolbar.qml must expose detailStateChangeRequested(int stateValue).");
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
        detailContentsText.contains(QStringLiteral("state: detailContents.activeStateName")),
        "DetailContents.qml must bind QML state to activeStateName.");
}

QTEST_APPLESS_MAIN(QmlBindingSyntaxGuardTest)

#include "test_qml_binding_syntax_guard.moc"
