#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::structuredDocumentBlocksModel_updatesRowsWithoutResettingStableSuffixBlocks()
{
    ContentsStructuredDocumentBlocksModel model;
    QSignalSpy rowsInsertedSpy(&model, &QAbstractItemModel::rowsInserted);
    QSignalSpy rowsRemovedSpy(&model, &QAbstractItemModel::rowsRemoved);
    QSignalSpy modelResetSpy(&model, &QAbstractItemModel::modelReset);
    QSignalSpy dataChangedSpy(&model, &QAbstractItemModel::dataChanged);

    const QVariantList initialBlocks{
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("text")},
            {QStringLiteral("sourceStart"), 0},
            {QStringLiteral("sourceEnd"), 5},
            {QStringLiteral("sourceText"), QStringLiteral("alpha")},
        },
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("text")},
            {QStringLiteral("sourceStart"), 6},
            {QStringLiteral("sourceEnd"), 10},
            {QStringLiteral("sourceText"), QStringLiteral("beta")},
        },
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("resource")},
            {QStringLiteral("sourceStart"), 11},
            {QStringLiteral("sourceEnd"), 24},
            {QStringLiteral("sourceText"), QStringLiteral("<resource />")},
        },
    };
    model.setBlocks(initialBlocks);
    QCOMPARE(model.count(), 3);
    QCOMPARE(rowsInsertedSpy.count(), 1);
    QCOMPARE(rowsRemovedSpy.count(), 0);
    QCOMPARE(modelResetSpy.count(), 0);

    rowsInsertedSpy.clear();
    rowsRemovedSpy.clear();
    dataChangedSpy.clear();

    const QVariantList editedBlocks{
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("text")},
            {QStringLiteral("sourceStart"), 0},
            {QStringLiteral("sourceEnd"), 2},
            {QStringLiteral("sourceText"), QStringLiteral("al")},
        },
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("text")},
            {QStringLiteral("sourceStart"), 3},
            {QStringLiteral("sourceEnd"), 7},
            {QStringLiteral("sourceText"), QStringLiteral("beta")},
        },
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("resource")},
            {QStringLiteral("sourceStart"), 8},
            {QStringLiteral("sourceEnd"), 21},
            {QStringLiteral("sourceText"), QStringLiteral("<resource />")},
        },
    };
    model.setBlocks(editedBlocks);

    QCOMPARE(model.count(), 3);
    QCOMPARE(rowsInsertedSpy.count(), 0);
    QCOMPARE(rowsRemovedSpy.count(), 0);
    QCOMPARE(modelResetSpy.count(), 0);
    QCOMPARE(dataChangedSpy.count(), 1);

    const auto changedRange = dataChangedSpy.at(0);
    QCOMPARE(changedRange.at(0).value<QModelIndex>().row(), 0);
    QCOMPARE(changedRange.at(1).value<QModelIndex>().row(), 2);

    QCOMPARE(
        model.data(model.index(2, 0), ContentsStructuredDocumentBlocksModel::BlockDataRole).toMap().value(
            QStringLiteral("sourceStart")).toInt(),
        8);
}

void WhatSonCppRegressionTests::structuredDocumentBlocksModel_removesOnlyChangedMiddleRows()
{
    ContentsStructuredDocumentBlocksModel model;
    QSignalSpy rowsInsertedSpy(&model, &QAbstractItemModel::rowsInserted);
    QSignalSpy rowsRemovedSpy(&model, &QAbstractItemModel::rowsRemoved);
    QSignalSpy modelResetSpy(&model, &QAbstractItemModel::modelReset);
    QSignalSpy dataChangedSpy(&model, &QAbstractItemModel::dataChanged);

    model.setBlocks(QVariantList{
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("text")},
            {QStringLiteral("sourceStart"), 0},
            {QStringLiteral("sourceEnd"), 5},
            {QStringLiteral("sourceText"), QStringLiteral("alpha")},
        },
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("text")},
            {QStringLiteral("sourceStart"), 6},
            {QStringLiteral("sourceEnd"), 10},
            {QStringLiteral("sourceText"), QStringLiteral("beta")},
        },
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("text")},
            {QStringLiteral("sourceStart"), 11},
            {QStringLiteral("sourceEnd"), 16},
            {QStringLiteral("sourceText"), QStringLiteral("gamma")},
        },
    });

    rowsInsertedSpy.clear();
    rowsRemovedSpy.clear();
    dataChangedSpy.clear();

    model.setBlocks(QVariantList{
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("text")},
            {QStringLiteral("sourceStart"), 0},
            {QStringLiteral("sourceEnd"), 5},
            {QStringLiteral("sourceText"), QStringLiteral("alpha")},
        },
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("text")},
            {QStringLiteral("sourceStart"), 6},
            {QStringLiteral("sourceEnd"), 11},
            {QStringLiteral("sourceText"), QStringLiteral("gamma")},
        },
    });

    QCOMPARE(model.count(), 2);
    QCOMPARE(rowsInsertedSpy.count(), 0);
    QCOMPARE(rowsRemovedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 0);
    QCOMPARE(dataChangedSpy.count(), 1);

    const auto removedRange = rowsRemovedSpy.takeFirst();
    QCOMPARE(removedRange.at(1).toInt(), 1);
    QCOMPARE(removedRange.at(2).toInt(), 1);

    const auto changedRange = dataChangedSpy.takeFirst();
    QCOMPARE(changedRange.at(0).value<QModelIndex>().row(), 1);
    QCOMPARE(changedRange.at(1).value<QModelIndex>().row(), 1);
    QCOMPARE(
        model.data(model.index(1, 0), ContentsStructuredDocumentBlocksModel::BlockDataRole).toMap().value(
            QStringLiteral("sourceText")).toString(),
        QStringLiteral("gamma"));
}
