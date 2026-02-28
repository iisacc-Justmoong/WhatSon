#pragma once

#include <functional>

namespace WhatSon::Permissions
{
    using PermissionCallback = std::function<void(bool granted)>;

    void requestPhotoLibraryPermission(const PermissionCallback& completion);
    void requestRemindersPermission(const PermissionCallback& completion);
    void requestAccessibilityPermission(const PermissionCallback& completion);
    void requestLocalNetworkPermission(const PermissionCallback& completion);
    void requestFullDiskAccessPermission(const PermissionCallback& completion);
} // namespace WhatSon::Permissions
