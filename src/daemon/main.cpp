#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTextStream>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("whats_on_daemon"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("WhatSon background daemon skeleton"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption healthcheckOption(
        QStringList() << QStringLiteral("healthcheck"),
        QStringLiteral("Run one-shot healthcheck and exit."));
    parser.addOption(healthcheckOption);

    parser.process(app);

    QTextStream out(stdout);

    if (parser.isSet(healthcheckOption))
    {
        out << "status=ok\n";
        return 0;
    }

    out << "WhatSon daemon skeleton initialized.\n";
    out << "No background jobs are registered yet.\n";
    return 0;
}
