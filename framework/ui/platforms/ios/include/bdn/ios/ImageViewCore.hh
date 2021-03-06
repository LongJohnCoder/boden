#pragma once

#include <bdn/ImageView.h>
#include <bdn/ios/ViewCore.hh>

namespace bdn::ios
{
    class ImageViewCore : public ViewCore, virtual public bdn::ImageView::Core
    {
      public:
        ImageViewCore(const std::shared_ptr<bdn::ViewCoreFactory> &viewCoreFactory);

      protected:
        void setUrl(const String &url);

      private:
        static UIView<UIViewWithFrameNotification> *createUIImageView();
        Size sizeForSpace(Size availableSize) const override;
    };
}
