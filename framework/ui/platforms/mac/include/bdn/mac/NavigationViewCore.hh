#pragma once

#import <bdn/mac/ViewCore.hh>

#include <bdn/NavigationView.h>

#include <deque>

@class BdnBackButtonClickHandler;

namespace bdn::mac
{
    class NavigationViewCore : public ViewCore, public bdn::NavigationView::Core
    {
      public:
        NavigationViewCore(const std::shared_ptr<bdn::ViewCoreFactory> &viewCoreFactory);
        ~NavigationViewCore();

        void init() override;

      public:
        void pushView(std::shared_ptr<View> view, String title) override;
        void popView() override;
        std::list<std::shared_ptr<View>> childViews() override;

      public:
        void setLayout(std::shared_ptr<Layout> layout) override;

      private:
        void updateCurrentView();
        void reLayout();

      private:
        NSView *_navigationBar;
        NSButton *_backButton;
        NSTextField *_title;

        std::shared_ptr<View> _currentView;
        std::shared_ptr<FixedView> _container;

        struct StackEntry
        {
            std::shared_ptr<View> view;
            String title;
        };

        std::deque<StackEntry> _stack;

        BdnBackButtonClickHandler *_backButtonClickHandler;
    };
}
