#include "test/cpp/whatson_cpp_regression_tests.hpp"

class FakeStructuredEditorSurface final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool visible MEMBER visible)
    Q_PROPERTY(bool activeFocus MEMBER activeFocus)

public:
    bool visible = true;
    bool activeFocus = false;
    int requestCount = 0;
    int forceFocusCount = 0;
    int lastSourceOffset = -1;

    Q_INVOKABLE void requestFocus(const QVariant& request)
    {
        requestCount += 1;
        lastSourceOffset = request.toMap().value(QStringLiteral("sourceOffset"), -1).toInt();
    }

    Q_INVOKABLE void forceActiveFocus()
    {
        activeFocus = true;
        forceFocusCount += 1;
    }
};

class FakeInlineEditorSurface final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool visible MEMBER visible)
    Q_PROPERTY(bool activeFocus MEMBER activeFocus)
    Q_PROPERTY(int cursorPosition MEMBER cursorPosition)

public:
    bool visible = true;
    bool activeFocus = false;
    int cursorPosition = -1;
    int focusCount = 0;
    int cursorSetCount = 0;

    Q_INVOKABLE void forceActiveFocus()
    {
        activeFocus = true;
        focusCount += 1;
    }

    Q_INVOKABLE void setCursorPositionPreservingNativeInput(const QVariant& cursorPositionValue)
    {
        cursorPosition = cursorPositionValue.toInt();
        cursorSetCount += 1;
    }
};

void WhatSonCppRegressionTests::contentsActiveEditorSurfaceAdapter_routesFocusToStructuredSurfaceFirst()
{
    FakeStructuredEditorSurface structuredSurface;
    FakeInlineEditorSurface inlineSurface;

    ContentsActiveEditorSurfaceAdapter adapter;
    adapter.setStructuredDocumentFlow(&structuredSurface);
    adapter.setContentEditor(&inlineSurface);
    adapter.setStructuredSurfaceActive(true);
    adapter.setInlineSurfaceActive(true);

    QVariantMap focusRequest;
    focusRequest.insert(QStringLiteral("sourceOffset"), 42);
    focusRequest.insert(QStringLiteral("logicalCursorPosition"), 7);

    QVERIFY(adapter.requestFocus(focusRequest));
    QCOMPARE(adapter.activeSurfaceKind(), QStringLiteral("structured"));
    QCOMPARE(structuredSurface.requestCount, 1);
    QCOMPARE(structuredSurface.forceFocusCount, 1);
    QCOMPARE(structuredSurface.lastSourceOffset, 42);
    QCOMPARE(inlineSurface.focusCount, 0);
    QCOMPARE(inlineSurface.cursorSetCount, 0);
    QVERIFY(adapter.isInputFocused());
}

void WhatSonCppRegressionTests::contentsActiveEditorSurfaceAdapter_routesFocusToInlineSurfaceOnlyWhenMounted()
{
    FakeStructuredEditorSurface structuredSurface;
    FakeInlineEditorSurface inlineSurface;

    ContentsActiveEditorSurfaceAdapter adapter;
    adapter.setStructuredDocumentFlow(&structuredSurface);
    adapter.setContentEditor(&inlineSurface);
    adapter.setStructuredSurfaceActive(false);
    adapter.setInlineSurfaceActive(false);

    QVariantMap focusRequest;
    focusRequest.insert(QStringLiteral("sourceOffset"), 42);
    focusRequest.insert(QStringLiteral("logicalCursorPosition"), 7);

    QVERIFY(!adapter.requestFocus(focusRequest));
    QCOMPARE(adapter.activeSurfaceKind(), QStringLiteral("none"));
    QCOMPARE(inlineSurface.focusCount, 0);

    adapter.setInlineSurfaceActive(true);
    QVERIFY(adapter.requestFocus(focusRequest));
    QCOMPARE(adapter.activeSurfaceKind(), QStringLiteral("inline"));
    QCOMPARE(structuredSurface.requestCount, 0);
    QCOMPARE(inlineSurface.focusCount, 1);
    QCOMPARE(inlineSurface.cursorSetCount, 1);
    QCOMPARE(inlineSurface.cursorPosition, 7);
    QCOMPARE(adapter.currentLogicalCursorPosition(), 7);
    QVERIFY(adapter.isInputFocused());
}

#include "active_editor_surface_adapter_tests.moc"
