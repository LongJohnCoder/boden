#include <bdn/ImageView.h>

namespace bdn
{
    namespace detail
    {
        VIEW_CORE_REGISTRY_IMPLEMENTATION(ImageView)
    }

    ImageView::ImageView(std::shared_ptr<ViewCoreFactory> viewCoreFactory) : View(std::move(viewCoreFactory))
    {
        detail::VIEW_CORE_REGISTER(ImageView, View::viewCoreFactory());
    }

    void ImageView::bindViewCore()
    {
        View::bindViewCore();

        auto imageCore = core<ImageView::Core>();
        imageCore->url.bind(url);
    }
}
