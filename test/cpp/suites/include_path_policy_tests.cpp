#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QDirIterator>

#include <algorithm>
#include <utility>

namespace
{
    QString repositoryRootPath()
    {
        QDir repositoryRoot(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath());
        repositoryRoot.cdUp();
        repositoryRoot.cdUp();
        repositoryRoot.cdUp();
        return repositoryRoot.absolutePath();
    }

    bool isHeaderExtension(const QString& suffix)
    {
        const QString normalizedSuffix = suffix.trimmed().toLower();
        return normalizedSuffix == QStringLiteral("h")
            || normalizedSuffix == QStringLiteral("hh")
            || normalizedSuffix == QStringLiteral("hpp")
            || normalizedSuffix == QStringLiteral("hxx");
    }

    bool isSourceExtension(const QString& suffix)
    {
        const QString normalizedSuffix = suffix.trimmed().toLower();
        return normalizedSuffix == QStringLiteral("c")
            || normalizedSuffix == QStringLiteral("cc")
            || normalizedSuffix == QStringLiteral("cpp")
            || normalizedSuffix == QStringLiteral("cxx")
            || normalizedSuffix == QStringLiteral("h")
            || normalizedSuffix == QStringLiteral("hh")
            || normalizedSuffix == QStringLiteral("hpp")
            || normalizedSuffix == QStringLiteral("hxx")
            || normalizedSuffix == QStringLiteral("mm");
    }

    QString canonicalProjectInclude(const QString& repositoryRelativePath)
    {
        static const QString srcAppPrefix = QStringLiteral("src/app/");
        static const QString srcExtensionPrefix = QStringLiteral("src/extension/");
        static const QString testPrefix = QStringLiteral("test/");

        if (repositoryRelativePath.startsWith(srcAppPrefix))
        {
            return QStringLiteral("app/") + repositoryRelativePath.mid(srcAppPrefix.size());
        }
        if (repositoryRelativePath.startsWith(srcExtensionPrefix))
        {
            return QStringLiteral("extension/") + repositoryRelativePath.mid(srcExtensionPrefix.size());
        }
        if (repositoryRelativePath.startsWith(testPrefix))
        {
            return repositoryRelativePath;
        }
        return {};
    }
} // namespace

void WhatSonCppRegressionTests::sourceTree_usesRepositoryAbsoluteProjectIncludes()
{
    const QDir repositoryRoot(repositoryRootPath());
    QVERIFY(repositoryRoot.exists());

    QStringList repositoryHeaderPaths;
    for (const QString& rootPath : {QStringLiteral("src"), QStringLiteral("test")})
    {
        QDirIterator it(
            repositoryRoot.filePath(rootPath),
            QDir::Files | QDir::NoDotAndDotDot,
            QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            const QString absolutePath = it.next();
            const QFileInfo fileInfo(absolutePath);
            if (!isHeaderExtension(fileInfo.suffix()))
            {
                continue;
            }

            repositoryHeaderPaths.append(QDir::cleanPath(repositoryRoot.relativeFilePath(absolutePath)));
        }
    }
    repositoryHeaderPaths.removeDuplicates();
    std::sort(repositoryHeaderPaths.begin(), repositoryHeaderPaths.end());

    auto resolveCanonicalProjectInclude = [&](const QString& sourceFilePath, const QString& includeLiteral) {
        const QFileInfo sourceFileInfo(sourceFilePath);
        const QFileInfo relativeCandidate(sourceFileInfo.absoluteDir(), includeLiteral);
        if (relativeCandidate.exists() && relativeCandidate.isFile() && isHeaderExtension(relativeCandidate.suffix()))
        {
            const QString repositoryRelativeCandidate =
                QDir::cleanPath(repositoryRoot.relativeFilePath(relativeCandidate.canonicalFilePath()));
            const QString canonical = canonicalProjectInclude(repositoryRelativeCandidate);
            if (!canonical.isEmpty())
            {
                return canonical;
            }
        }

        const QString normalizedLiteral = QDir::cleanPath(includeLiteral.trimmed());
        for (const QString& repositoryHeaderPath : std::as_const(repositoryHeaderPaths))
        {
            if (repositoryHeaderPath != normalizedLiteral
                && !repositoryHeaderPath.endsWith(QStringLiteral("/") + normalizedLiteral))
            {
                continue;
            }

            const QString canonical = canonicalProjectInclude(repositoryHeaderPath);
            if (!canonical.isEmpty())
            {
                return canonical;
            }
        }

        return QString{};
    };

    QStringList violations;
    const QRegularExpression includePattern(QStringLiteral(R"include(^\s*#include\s+"([^"]+)")include"));
    for (const QString& rootPath : {QStringLiteral("src"), QStringLiteral("test")})
    {
        QDirIterator it(
            repositoryRoot.filePath(rootPath),
            QDir::Files | QDir::NoDotAndDotDot,
            QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            const QString absolutePath = it.next();
            const QFileInfo fileInfo(absolutePath);
            if (!isSourceExtension(fileInfo.suffix()))
            {
                continue;
            }

            QFile sourceFile(absolutePath);
            if (!sourceFile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                continue;
            }

            int lineNumber = 0;
            while (!sourceFile.atEnd())
            {
                const QString line = QString::fromUtf8(sourceFile.readLine());
                ++lineNumber;

                const QRegularExpressionMatch match = includePattern.match(line);
                if (!match.hasMatch())
                {
                    continue;
                }

                const QString includeLiteral = match.captured(1).trimmed();
                if (includeLiteral.startsWith(QStringLiteral("app/"))
                    || includeLiteral.startsWith(QStringLiteral("extension/"))
                    || includeLiteral.startsWith(QStringLiteral("test/")))
                {
                    continue;
                }

                const QString canonicalInclude =
                    resolveCanonicalProjectInclude(absolutePath, includeLiteral);
                if (canonicalInclude.isEmpty())
                {
                    continue;
                }

                violations.append(
                    QStringLiteral("%1:%2 uses \"%3\" instead of \"%4\"")
                        .arg(QDir::cleanPath(repositoryRoot.relativeFilePath(absolutePath)))
                        .arg(lineNumber)
                        .arg(includeLiteral)
                        .arg(canonicalInclude));
            }
        }
    }

    QVERIFY2(violations.isEmpty(), qPrintable(violations.join(QLatin1Char('\n'))));
}
