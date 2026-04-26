#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::contentsDisplaySurfacePolicy_usesStructuredSurfaceAsCanonicalNoteBody()
{
    ContentsDisplaySurfacePolicy policy;

    QCOMPARE(policy.activeSurfaceKind(), QStringLiteral("none"));
    QCOMPARE(policy.structuredDocumentSurfaceRequested(), false);
    QCOMPARE(policy.structuredDocumentFlowVisible(), false);
    QCOMPARE(policy.nativeInputPriority(), false);

    policy.setHasSelectedNote(true);

    QCOMPARE(policy.activeSurfaceKind(), QStringLiteral("structured"));
    QCOMPARE(policy.structuredDocumentSurfaceRequested(), true);
    QCOMPARE(policy.structuredDocumentFlowVisible(), true);
    QCOMPARE(policy.nativeInputPriority(), true);
    QCOMPARE(policy.documentPresentationProjectionEnabled(), false);
}

void WhatSonCppRegressionTests::contentsDisplaySurfacePolicy_disablesLegacyInlineSurface()
{
    ContentsDisplaySurfacePolicy policy;
    policy.setHasSelectedNote(true);

    QCOMPARE(policy.inlineDocumentSurfaceRequested(), false);
    QCOMPARE(policy.inlineDocumentSurfaceReady(), false);
    QCOMPARE(policy.inlineDocumentSurfaceLoading(), false);
    QCOMPARE(policy.inlineHtmlImageRenderingEnabled(), false);
    QCOMPARE(policy.resourceBlocksRenderedInlineByHtmlProjection(), false);

    policy.setFormattedPreviewRequested(true);

    QCOMPARE(policy.activeSurfaceKind(), QStringLiteral("formattedPreview"));
    QCOMPARE(policy.structuredDocumentSurfaceRequested(), false);
    QCOMPARE(policy.inlineDocumentSurfaceRequested(), false);
    QCOMPARE(policy.formattedTextRendererVisible(), true);
    QCOMPARE(policy.documentPresentationProjectionEnabled(), true);
}
