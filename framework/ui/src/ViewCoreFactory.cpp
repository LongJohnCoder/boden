#include <bdn/ViewCoreFactory.h>

namespace bdn
{
    std::shared_ptr<View::Core> ViewCoreFactory::createViewCore(const std::type_info &viewType)
    {
        auto core = create(viewType.name(), shared_from_this());

        if (!core) {
            throw ViewCoreTypeNotSupportedError(viewType.name());
        }

        return *core;
    }

    ViewCoreFactory::ContextStack *ViewCoreFactory::contextStack()
    {
        static ContextStack s_contextStack;
        return &s_contextStack;
    }

    void ViewCoreFactory::pushContext(std::shared_ptr<UIContext> &context)
    {
        auto stack = contextStack();
        stack->push_back(context);
    }

    void ViewCoreFactory::popContext()
    {
        auto stack = contextStack();
        stack->pop_back();
    }
}
