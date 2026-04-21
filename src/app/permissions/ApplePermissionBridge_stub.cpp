#include "ApplePermissionBridge.hpp"
#include "models/file/WhatSonDebugTrace.hpp"

namespace WhatSon::Permissions
{
    void requestPhotoLibraryPermission(const PermissionCallback& completion)
    {
        WhatSon::Debug::trace(QStringLiteral("permissions.stub.photo"),
                              QStringLiteral("request"),
                              QStringLiteral("hasCompletion=%1")
                              .arg(completion ? QStringLiteral("1") : QStringLiteral("0")));
        if (completion)
        {
            WhatSon::Debug::trace(QStringLiteral("permissions.stub.photo"),
                                  QStringLiteral("complete"),
                                  QStringLiteral("granted=1"));
            completion(true);
        }
    }

    void requestRemindersPermission(const PermissionCallback& completion)
    {
        WhatSon::Debug::trace(QStringLiteral("permissions.stub.reminders"),
                              QStringLiteral("request"),
                              QStringLiteral("hasCompletion=%1")
                              .arg(completion ? QStringLiteral("1") : QStringLiteral("0")));
        if (completion)
        {
            WhatSon::Debug::trace(QStringLiteral("permissions.stub.reminders"),
                                  QStringLiteral("complete"),
                                  QStringLiteral("granted=1"));
            completion(true);
        }
    }

    void requestAccessibilityPermission(const PermissionCallback& completion)
    {
        WhatSon::Debug::trace(QStringLiteral("permissions.stub.accessibility"),
                              QStringLiteral("request"),
                              QStringLiteral("hasCompletion=%1")
                              .arg(completion ? QStringLiteral("1") : QStringLiteral("0")));
        if (completion)
        {
            WhatSon::Debug::trace(QStringLiteral("permissions.stub.accessibility"),
                                  QStringLiteral("complete"),
                                  QStringLiteral("granted=1"));
            completion(true);
        }
    }

    void requestLocalNetworkPermission(const PermissionCallback& completion)
    {
        WhatSon::Debug::trace(QStringLiteral("permissions.stub.localNetwork"),
                              QStringLiteral("request"),
                              QStringLiteral("hasCompletion=%1")
                              .arg(completion ? QStringLiteral("1") : QStringLiteral("0")));
        if (completion)
        {
            WhatSon::Debug::trace(QStringLiteral("permissions.stub.localNetwork"),
                                  QStringLiteral("complete"),
                                  QStringLiteral("granted=1"));
            completion(true);
        }
    }

    void requestFullDiskAccessPermission(const PermissionCallback& completion)
    {
        WhatSon::Debug::trace(QStringLiteral("permissions.stub.fullDisk"),
                              QStringLiteral("request"),
                              QStringLiteral("hasCompletion=%1")
                              .arg(completion ? QStringLiteral("1") : QStringLiteral("0")));
        if (completion)
        {
            WhatSon::Debug::trace(QStringLiteral("permissions.stub.fullDisk"),
                                  QStringLiteral("complete"),
                                  QStringLiteral("granted=1"));
            completion(true);
        }
    }
} // namespace WhatSon::Permissions
