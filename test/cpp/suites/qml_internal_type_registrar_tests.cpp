#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::qmlInternalTypeRegistrar_usesLvrsManifestRegistration()
{
    const QString registrarHeader = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.hpp"));
    const QString registrarSource = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.cpp"));
    const QString mainSource = readUtf8SourceFile(QStringLiteral("src/app/main.cpp"));

    QVERIFY(registrarHeader.contains(QStringLiteral("#include \"backend/runtime/qmltyperegistrar.h\"")));
    QVERIFY(registrarHeader.contains(
        QStringLiteral("QList<lvrs::QmlTypeRegistration> internalQmlTypeRegistrationManifest()")));
    QVERIFY(registrarHeader.contains(QStringLiteral("lvrs::QmlTypeRegistrationReport registerInternalQmlTypes()")));

    QVERIFY(registrarSource.contains(QStringLiteral("internalQmlTypeRegistrationManifest()")));
    QVERIFY(registrarSource.contains(QStringLiteral("whatsonInternalCreatableType<ContentsEditorSelectionBridge>")));
    QVERIFY(registrarSource.contains(QStringLiteral("whatsonInternalCreatableType<ContentsDisplaySelectionSyncCoordinator>")));
    QVERIFY(registrarSource.contains(QStringLiteral("whatsonInternalCreatableType<MobileHierarchyRouteStateStore>")));
    QVERIFY(registrarSource.contains(QStringLiteral("whatsonInternalCreatableType<ContentsStructuredDocumentHost>")));
    QVERIFY(registrarSource.contains(QStringLiteral("whatsonInternalCreatableType<ContentsPagePrintLayoutRenderer>")));
    QVERIFY(registrarSource.contains(QStringLiteral("whatsonInternalCreatableType<ContentsDisplaySurfacePolicy>")));
    QVERIFY(registrarSource.contains(QStringLiteral("whatsonInternalCreatableType<ContentsDisplayGeometryViewModel>")));
    QVERIFY(registrarSource.contains(QStringLiteral("whatsonInternalCreatableType<ContentsDisplayMutationViewModel>")));
    QVERIFY(registrarSource.contains(QStringLiteral("whatsonInternalCreatableType<ContentsDisplayPresentationViewModel>")));
    QVERIFY(registrarSource.contains(QStringLiteral("whatsonInternalCreatableType<ContentsDisplaySelectionMountViewModel>")));
    QVERIFY(registrarSource.contains(QStringLiteral("whatsonInternalCreatableType<ContentsInlineStyleOverlayRenderer>")));
    QVERIFY(registrarSource.contains(QStringLiteral("whatsonInternalCreatableType<ContentsPlainTextSourceMutator>")));
    QVERIFY(registrarSource.contains(QStringLiteral("whatsonInternalCreatableType<WhatSonIosHubPickerBridge>")));
    QVERIFY(!registrarSource.contains(QStringLiteral("whatsonInternalCreatableType<ContentsEditorBodyTagInsertionPlanner>")));
    QVERIFY(registrarSource.contains(QStringLiteral("QStringLiteral(\"WhatSon.App.Internal\")")));
    QVERIFY(registrarSource.contains(QStringLiteral("lvrs::registerQmlTypes(internalQmlTypeRegistrationManifest())")));
    QVERIFY(!registrarSource.contains(QStringLiteral("qmlRegisterType<")));

    const qsizetype manifestEntryCount = registrarSource.count(QStringLiteral("whatsonInternalCreatableType<"));
    QVERIFY(manifestEntryCount >= 40);

    QVERIFY(mainSource.contains(QStringLiteral("const lvrs::QmlTypeRegistrationReport internalQmlTypeRegistrationReport")));
    QVERIFY(mainSource.contains(QStringLiteral("registerInternalQmlTypes()")));
    QVERIFY(mainSource.contains(QStringLiteral("internalQmlTypeRegistrationReport.errorMessage()")));
}
