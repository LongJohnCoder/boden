#pragma once

#include <Cocoa/Cocoa.h>

#include <bdn/Window.h>

#import <bdn/mac/util.hh>

@class BdnMacWindowContentViewParent_;

namespace bdn::mac
{
    class WindowCore : public ViewCore, virtual public bdn::Window::Core
    {
      public:
        WindowCore(const std::shared_ptr<bdn::ViewCoreFactory> &viewCoreFactory);
        ~WindowCore();

      public:
        void init() override;

        NSWindow *getNSWindow() { return _nsWindow; }

        bool canMoveToParentView(std::shared_ptr<View> newParentView) const override;

        void _movedOrResized();

        void scheduleLayout() override;

      private:
        Rect getContentArea();
        Rect getScreenWorkArea() const;
        Size getMinimumSize() const;
        Margin getNonClientMargin() const;

        NSScreen *_getNsScreen() const;

        void updateContent(const std::shared_ptr<View> &newContent);

      private:
        bool _isInMoveOrResize = false;
        NSWindow *_nsWindow;
        BdnMacWindowContentViewParent_ *_nsContentParent;

        NSObject *_ourDelegate;
    };
}
