#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QGuiApplication>
#include <QtTest>

int main(int argc, char** argv)
{
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
    {
        qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("offscreen"));
    }

    QGuiApplication application(argc, argv);
    WhatSonCppRegressionTests tests;
    return QTest::qExec(&tests, argc, argv);
}
