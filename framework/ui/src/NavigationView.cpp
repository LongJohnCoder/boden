#include <bdn/NavigationView.h>

#include <utility>

namespace bdn
{
    namespace detail
    {
        VIEW_CORE_REGISTRY_IMPLEMENTATION(NavigationView)
    }

    NavigationView::NavigationView(std::shared_ptr<ViewCoreFactory> viewCoreFactory) : View(std::move(viewCoreFactory))
    {
        detail::VIEW_CORE_REGISTER(NavigationView, View::viewCoreFactory());
    }

    void NavigationView::pushView(std::shared_ptr<View> view, String title)
    {
        if (auto navigationViewCore = core<NavigationView::Core>()) {
            navigationViewCore->pushView(std::move(view), std::move(title));
        }
    }

    void NavigationView::popView()
    {
        if (auto navigationViewCore = core<NavigationView::Core>()) {
            navigationViewCore->popView();
        }
    }

    std::list<std::shared_ptr<View>> NavigationView::childViews()
    {
        if (auto navigationViewCore = core<NavigationView::Core>()) {
            return navigationViewCore->childViews();
        }
        return {};
    }

    void NavigationView::bindViewCore() { View::bindViewCore(); }
}
