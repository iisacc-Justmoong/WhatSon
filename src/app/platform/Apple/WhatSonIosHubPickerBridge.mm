#include "WhatSonIosHubPickerBridge.hpp"

#if defined(Q_OS_IOS)

#import <UIKit/UIKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#import <MobileCoreServices/MobileCoreServices.h>

#include <QMetaObject>
#include <QPointer>

namespace
{
    UIWindow* activePresentationWindow()
    {
        UIApplication* application = UIApplication.sharedApplication;
        if (application == nil)
        {
            return nil;
        }

        if (@available(iOS 13.0, *))
        {
            for (UIScene* scene in application.connectedScenes)
            {
                if (![scene isKindOfClass:UIWindowScene.class])
                {
                    continue;
                }

                UIWindowScene* windowScene = (UIWindowScene*)scene;
                if (windowScene.activationState != UISceneActivationStateForegroundActive)
                {
                    continue;
                }

                for (UIWindow* window in windowScene.windows)
                {
                    if (window.isKeyWindow)
                    {
                        return window;
                    }
                }

                for (UIWindow* window in windowScene.windows)
                {
                    if (!window.hidden)
                    {
                        return window;
                    }
                }
            }
        }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        if (application.keyWindow != nil)
        {
            return application.keyWindow;
        }

        for (UIWindow* window in application.windows)
        {
            if (window.isKeyWindow)
            {
                return window;
            }
        }

        if (application.windows.count > 0)
        {
            return application.windows.firstObject;
        }
#pragma clang diagnostic pop

        return nil;
    }

    UIViewController* topPresenterForWindow(UIWindow* window)
    {
        if (window == nil)
        {
            return nil;
        }

        UIViewController* presenter = window.rootViewController;
        while (presenter.presentedViewController != nil && !presenter.presentedViewController.isBeingDismissed)
        {
            presenter = presenter.presentedViewController;
        }
        return presenter;
    }

    UIDocumentPickerViewController* createHubPickerController(const QUrl& initialDirectoryUrl)
    {
        UIDocumentPickerViewController* controller = nil;
        if (@available(iOS 14.0, *))
        {
            NSMutableArray<UTType*>* allowedContentTypes = [NSMutableArray array];
            [allowedContentTypes addObject:UTTypeItem];
            [allowedContentTypes addObject:UTTypeContent];
            [allowedContentTypes addObject:UTTypeData];
            controller = [[UIDocumentPickerViewController alloc] initForOpeningContentTypes:allowedContentTypes
                                                                                      asCopy:NO];
        }
        else
        {
            NSMutableArray<NSString*>* allowedTypeIdentifiers = [NSMutableArray array];
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
            [allowedTypeIdentifiers addObject:(__bridge NSString*)kUTTypeItem];
            [allowedTypeIdentifiers addObject:(__bridge NSString*)kUTTypeContent];
            [allowedTypeIdentifiers addObject:(__bridge NSString*)kUTTypeData];
            controller = [[UIDocumentPickerViewController alloc] initWithDocumentTypes:allowedTypeIdentifiers
                                                                                 inMode:UIDocumentPickerModeOpen];
#pragma clang diagnostic pop
        }

        if (controller == nil)
        {
            return nil;
        }

        controller.allowsMultipleSelection = NO;
        controller.modalPresentationStyle = UIModalPresentationFormSheet;
        if ([controller respondsToSelector:@selector(setShouldShowFileExtensions:)])
        {
            controller.shouldShowFileExtensions = YES;
        }

        if (initialDirectoryUrl.isValid())
        {
            NSURL* initialDirectory = initialDirectoryUrl.toNSURL();
            if (initialDirectory != nil)
            {
                controller.directoryURL = initialDirectory;
            }
        }

        return controller;
    }
}

@class WhatSonIosHubPickerDelegate;

class WhatSonIosHubPickerBridgePrivate
{
public:
    QPointer<WhatSonIosHubPickerBridge> bridge;
    UIDocumentPickerViewController* controller = nil;
    WhatSonIosHubPickerDelegate* delegate = nil;
};

@interface WhatSonIosHubPickerDelegate
    : NSObject<UIDocumentPickerDelegate, UINavigationControllerDelegate, UIAdaptivePresentationControllerDelegate>
@property(nonatomic, assign) WhatSonIosHubPickerBridgePrivate* owner;
@property(nonatomic, assign) BOOL finished;
@end

@implementation WhatSonIosHubPickerDelegate

- (void)dispatchAcceptForUrl:(NSURL*)url
{
    if (self.finished)
    {
        return;
    }

    self.finished = YES;
    WhatSonIosHubPickerBridgePrivate* owner = self.owner;
    if (owner == nullptr || owner->bridge == nullptr)
    {
        return;
    }

    const QUrl selectedUrl = QUrl::fromNSURL(url);
    QMetaObject::invokeMethod(
        owner->bridge,
        "handlePickerAccepted",
        Qt::QueuedConnection,
        Q_ARG(QUrl, selectedUrl));
}

- (void)dispatchCancel
{
    if (self.finished)
    {
        return;
    }

    self.finished = YES;
    WhatSonIosHubPickerBridgePrivate* owner = self.owner;
    if (owner == nullptr || owner->bridge == nullptr)
    {
        return;
    }

    QMetaObject::invokeMethod(
        owner->bridge,
        "handlePickerCanceled",
        Qt::QueuedConnection);
}

- (void)dispatchFailure:(NSString*)message
{
    if (self.finished)
    {
        return;
    }

    self.finished = YES;
    WhatSonIosHubPickerBridgePrivate* owner = self.owner;
    if (owner == nullptr || owner->bridge == nullptr)
    {
        return;
    }

    const QString errorText = message == nil
                                  ? QStringLiteral("The native iOS hub picker failed.")
                                  : QString::fromUtf8(message.UTF8String).trimmed();
    QMetaObject::invokeMethod(
        owner->bridge,
        "handlePickerFailed",
        Qt::QueuedConnection,
        Q_ARG(QString, errorText));
}

- (void)documentPicker:(UIDocumentPickerViewController*)controller didPickDocumentsAtURLs:(NSArray<NSURL*>*)urls
{
    Q_UNUSED(controller);

    NSURL* selectedUrl = urls.count > 0 ? urls.firstObject : nil;
    if (selectedUrl == nil)
    {
        [self dispatchFailure:@"The native iOS hub picker returned no selected URL."];
        return;
    }

    [self dispatchAcceptForUrl:selectedUrl];
}

- (void)documentPickerWasCancelled:(UIDocumentPickerViewController*)controller
{
    Q_UNUSED(controller);
    [self dispatchCancel];
}

- (void)presentationControllerDidDismiss:(UIPresentationController*)presentationController
{
    Q_UNUSED(presentationController);
    [self dispatchCancel];
}

@end

WhatSonIosHubPickerBridge::WhatSonIosHubPickerBridge(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<WhatSonIosHubPickerBridgePrivate>())
{
    d->bridge = this;
}

WhatSonIosHubPickerBridge::~WhatSonIosHubPickerBridge()
{
    if (d->delegate != nil)
    {
        d->delegate.owner = nullptr;
    }

    if (d->controller != nil)
    {
        d->controller.delegate = nil;
        if (d->controller.presentationController != nil)
        {
            d->controller.presentationController.delegate = nil;
        }
        [d->controller dismissViewControllerAnimated:NO completion:nil];
        [d->controller release];
        d->controller = nil;
    }

    if (d->delegate != nil)
    {
        [d->delegate release];
        d->delegate = nil;
    }
}

bool WhatSonIosHubPickerBridge::busy() const noexcept
{
    return m_busy;
}

QString WhatSonIosHubPickerBridge::lastError() const
{
    return m_lastError;
}

bool WhatSonIosHubPickerBridge::open(const QUrl& initialDirectoryUrl)
{
    clearLastError();
    if (m_busy)
    {
        setLastError(QStringLiteral("The native iOS hub picker is already open."));
        return false;
    }

    UIWindow* window = activePresentationWindow();
    UIViewController* presenter = topPresenterForWindow(window);
    if (presenter == nil)
    {
        setLastError(QStringLiteral("Failed to find an active iOS presenter for the hub picker."));
        return false;
    }

    UIDocumentPickerViewController* controller = createHubPickerController(initialDirectoryUrl);
    if (controller == nil)
    {
        setLastError(QStringLiteral("Failed to create the native iOS hub picker."));
        return false;
    }

    WhatSonIosHubPickerDelegate* delegate = [[WhatSonIosHubPickerDelegate alloc] init];
    delegate.owner = d.get();
    delegate.finished = NO;
    controller.delegate = delegate;
    if (controller.presentationController != nil)
    {
        controller.presentationController.delegate = delegate;
    }

    d->controller = controller;
    d->delegate = delegate;
    setBusy(true);
    [presenter presentViewController:controller animated:YES completion:nil];
    return true;
}

void WhatSonIosHubPickerBridge::clearLastError()
{
    setLastError(QString());
}

void WhatSonIosHubPickerBridge::handlePickerAccepted(const QUrl& selectedUrl)
{
    if (d->controller != nil)
    {
        d->controller.delegate = nil;
        if (d->controller.presentationController != nil)
        {
            d->controller.presentationController.delegate = nil;
        }
        [d->controller release];
        d->controller = nil;
    }
    if (d->delegate != nil)
    {
        d->delegate.owner = nullptr;
        [d->delegate release];
        d->delegate = nil;
    }

    setBusy(false);
    if (!selectedUrl.isValid())
    {
        setLastError(QStringLiteral("The native iOS hub picker returned an invalid URL."));
        return;
    }

    clearLastError();
    emit accepted(selectedUrl);
}

void WhatSonIosHubPickerBridge::handlePickerCanceled()
{
    if (d->controller != nil)
    {
        d->controller.delegate = nil;
        if (d->controller.presentationController != nil)
        {
            d->controller.presentationController.delegate = nil;
        }
        [d->controller release];
        d->controller = nil;
    }
    if (d->delegate != nil)
    {
        d->delegate.owner = nullptr;
        [d->delegate release];
        d->delegate = nil;
    }

    setBusy(false);
    clearLastError();
    emit canceled();
}

void WhatSonIosHubPickerBridge::handlePickerFailed(const QString& errorMessage)
{
    if (d->controller != nil)
    {
        d->controller.delegate = nil;
        if (d->controller.presentationController != nil)
        {
            d->controller.presentationController.delegate = nil;
        }
        [d->controller dismissViewControllerAnimated:YES completion:nil];
        [d->controller release];
        d->controller = nil;
    }
    if (d->delegate != nil)
    {
        d->delegate.owner = nullptr;
        [d->delegate release];
        d->delegate = nil;
    }

    setBusy(false);
    setLastError(errorMessage);
}

void WhatSonIosHubPickerBridge::setBusy(const bool busy)
{
    if (m_busy == busy)
    {
        return;
    }

    m_busy = busy;
    emit busyChanged();
}

void WhatSonIosHubPickerBridge::setLastError(const QString& errorMessage)
{
    const QString normalizedError = errorMessage.trimmed();
    if (m_lastError == normalizedError)
    {
        return;
    }

    m_lastError = normalizedError;
    emit lastErrorChanged();
}

#endif
