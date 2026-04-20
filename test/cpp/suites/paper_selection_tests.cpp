#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::paperSelection_tracksChosenPaperEnumState()
{
    ContentsPaperSelection selection;
    QSignalSpy selectedPaperKindChangedSpy(&selection, &ContentsPaperSelection::selectedPaperKindChanged);

    QCOMPARE(selection.selectedPaperKind(), ContentsPaperSelection::A4);
    QCOMPARE(selection.selectedPaperStandard(), QStringLiteral("A4"));
    QCOMPARE(
        ContentsPaperSelection::paperStandardForKind(ContentsPaperSelection::Letter),
        QStringLiteral("Letter"));
    QCOMPARE(selection.paperStandardForValue(ContentsPaperSelection::Legal), QStringLiteral("Legal"));

    selection.setSelectedPaperKind(ContentsPaperSelection::B5);
    QCOMPARE(selection.selectedPaperKind(), ContentsPaperSelection::B5);
    QCOMPARE(selection.selectedPaperStandard(), QStringLiteral("B5"));

    selection.setSelectedPaperKindByValue(ContentsPaperSelection::A5);
    QCOMPARE(selection.selectedPaperKind(), ContentsPaperSelection::A5);
    QCOMPARE(selection.selectedPaperStandard(), QStringLiteral("A5"));

    selection.setSelectedPaperKindByValue(999);
    QCOMPARE(selection.selectedPaperKind(), ContentsPaperSelection::Unknown);
    QCOMPARE(selection.selectedPaperStandard(), QStringLiteral("Unknown"));
    QCOMPARE(selectedPaperKindChangedSpy.count(), 3);
}
