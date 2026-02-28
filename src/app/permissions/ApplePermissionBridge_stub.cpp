#include "ApplePermissionBridge.hpp"

namespace WhatSon::Permissions
{
    void requestPhotoLibraryPermission(const PermissionCallback& completion)
    {
        if (completion)
        {
            completion(true);
        }
    }

    void requestRemindersPermission(const PermissionCallback& completion)
    {
        if (completion)
        {
            completion(true);
        }
    }

    void requestAccessibilityPermission(const PermissionCallback& completion)
    {
        if (completion)
        {
            completion(true);
        }
    }

    void requestLocalNetworkPermission(const PermissionCallback& completion)
    {
        if (completion)
        {
            completion(true);
        }
    }

    void requestFullDiskAccessPermission(const PermissionCallback& completion)
    {
        if (completion)
        {
            completion(true);
        }
    }
} // namespace WhatSon::Permissions
