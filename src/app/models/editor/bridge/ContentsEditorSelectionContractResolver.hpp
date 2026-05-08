#pragma once

#include <QString>
#include <QVariantMap>

class QObject;

class ContentsEditorSelectionContractResolver final
{
public:
    ContentsEditorSelectionContractResolver() = delete;

    static bool noteBackedSelectionEnabled(const QObject* noteListModel);
    static bool hasReadableProperty(const QObject* object, const char* propertyName);
    static bool hasInvokableMethod(const QObject* object, const char* methodSignature);
    static QString readStringProperty(const QObject* object, const char* propertyName);
    static int readIntProperty(const QObject* object, const char* propertyName);
    static QVariantMap currentNoteEntry(const QObject* noteListModel);
    static QString noteIdFromEntry(const QVariantMap& noteEntry);
    static QString noteDirectoryPathFromEntry(const QVariantMap& noteEntry);
    static QString noteIdFromModelRow(const QObject* noteListModel, int row);
    static QString noteDirectoryPathFromModelRow(const QObject* noteListModel, int row);
};
