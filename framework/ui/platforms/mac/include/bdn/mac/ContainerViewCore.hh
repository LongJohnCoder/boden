#pragma once

#include <bdn/ContainerView.h>
#import <bdn/mac/ViewCore.hh>

namespace bdn::mac
{
    class ContainerViewCore : public ViewCore, virtual public bdn::ContainerView::Core
    {
      private:
        static NSView *_createContainer();

      public:
        ContainerViewCore(const std::shared_ptr<bdn::ViewCoreFactory> &viewCoreFactory);
        explicit ContainerViewCore(const std::shared_ptr<bdn::ViewCoreFactory> &viewCoreFactory, NSView *view);

      public:
        void init() override;

        void addChildView(std::shared_ptr<View> child) override;
        void removeChildView(std::shared_ptr<View> child) override;

        std::list<std::shared_ptr<View>> childViews() override;

      private:
        std::list<std::shared_ptr<View>> _children;
    };
}
