#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::displayPaperModels_hostPageAndPrintViewModeObjectsUnderModelsDirectory()
{
    const QString a4PaperBackgroundHeaderSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/display/paper/ContentsA4PaperBackground.hpp"));
    const QString a4PaperBackgroundImplSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/display/paper/ContentsA4PaperBackground.cpp"));
    const QString paperSelectionHeaderSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/display/paper/ContentsPaperSelection.hpp"));
    const QString paperSelectionImplSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/display/paper/ContentsPaperSelection.cpp"));
    const QString textFormatRendererHeaderSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/format/ContentsTextFormatRenderer.hpp"));
    const QString textFormatRendererImplSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/format/ContentsTextFormatRenderer.cpp"));
    const QString oldPaperRendererHeaderSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/display/paper/ContentsTextFormatRenderer.hpp"));
    const QString oldPaperRendererImplSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/display/paper/ContentsTextFormatRenderer.cpp"));
    const QString printLayoutHeaderSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/display/paper/print/ContentsPagePrintLayoutRenderer.hpp"));
    const QString printLayoutImplSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/display/paper/print/ContentsPagePrintLayoutRenderer.cpp"));
    const QString appCmakeSource = readUtf8SourceFile(QStringLiteral("src/app/CMakeLists.txt"));
    const QString displayCmakeSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/display/CMakeLists.txt"));
    const QString editorCmakeSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/CMakeLists.txt"));
    const QString paperCmakeSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/display/paper/CMakeLists.txt"));
    const QString printCmakeSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/display/paper/print/CMakeLists.txt"));
    const QString registrarSource = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.cpp"));

    QVERIFY(!a4PaperBackgroundHeaderSource.isEmpty());
    QVERIFY(!a4PaperBackgroundImplSource.isEmpty());
    QVERIFY(!paperSelectionHeaderSource.isEmpty());
    QVERIFY(!paperSelectionImplSource.isEmpty());
    QVERIFY(!textFormatRendererHeaderSource.isEmpty());
    QVERIFY(!textFormatRendererImplSource.isEmpty());
    QVERIFY(oldPaperRendererHeaderSource.isEmpty());
    QVERIFY(oldPaperRendererImplSource.isEmpty());
    QVERIFY(!printLayoutHeaderSource.isEmpty());
    QVERIFY(!printLayoutImplSource.isEmpty());
    QVERIFY(readUtf8SourceFile(QStringLiteral("src/app/models/editor/renderer/ContentsTextFormatRenderer.hpp")).isEmpty());
    QVERIFY(readUtf8SourceFile(QStringLiteral("src/app/models/editor/renderer/ContentsTextHighlightRenderer.hpp")).isEmpty());
    QVERIFY(readUtf8SourceFile(QStringLiteral("src/app/models/editor/renderer/ContentsPagePrintLayoutRenderer.hpp")).isEmpty());

    QVERIFY(appCmakeSource.contains(QStringLiteral("add_subdirectory(models/display)")));
    QVERIFY(appCmakeSource.contains(QStringLiteral("add_subdirectory(models/editor)")));
    QVERIFY(displayCmakeSource.contains(QStringLiteral("add_subdirectory(paper)")));
    QVERIFY(editorCmakeSource.contains(QStringLiteral("whatson_app_register_directory_sources")));
    QVERIFY(editorCmakeSource.contains(QStringLiteral("RECURSE")));
    QVERIFY(paperCmakeSource.contains(QStringLiteral("add_subdirectory(print)")));
    QVERIFY(paperCmakeSource.contains(QStringLiteral("whatson_app_register_directory_sources")));
    QVERIFY(printCmakeSource.contains(QStringLiteral("whatson_app_register_directory_sources")));

    QVERIFY(registrarSource.contains(
        QStringLiteral("#include \"app/models/display/paper/ContentsPaperSelection.hpp\"")));
    QVERIFY(registrarSource.contains(
        QStringLiteral("#include \"app/models/display/paper/ContentsA4PaperBackground.hpp\"")));
    QVERIFY(registrarSource.contains(
        QStringLiteral("whatsonInternalCreatableType<ContentsPaperSelection>(")));
    QVERIFY(registrarSource.contains(
        QStringLiteral("#include \"app/models/editor/format/ContentsTextFormatRenderer.hpp\"")));
    QVERIFY(registrarSource.contains(
        QStringLiteral("#include \"app/models/display/paper/print/ContentsPagePrintLayoutRenderer.hpp\"")));
    QVERIFY(registrarSource.contains(
        QStringLiteral("whatsonInternalCreatableType<ContentsA4PaperBackground>(")));
}
