#pragma once

#include <bdn/Layout.h>
#include <bdn/OfferedValue.h>
#include <bdn/Rect.h>
#include <bdn/WeakCallback.h>
#include <bdn/property/Property.h>

#include <list>

namespace bdn
{
    class ViewCoreFactory;

    class View : public Base
    {
        friend class Window;

      public:
        class Core;

      public:
        Property<Rect> geometry;
        Property<bool> visible;
        Property<std::shared_ptr<LayoutStylesheet>> layoutStylesheet;

      public:
        View() = delete;
        View(std::shared_ptr<ViewCoreFactory> viewCoreFactory = nullptr);
        View(const View &o) = delete;

        ~View() override;

        auto shared_from_this() { return std::static_pointer_cast<View>(Base::shared_from_this()); }
        auto shared_from_this() const { return std::static_pointer_cast<View const>(Base::shared_from_this()); }

      public:
        virtual Size sizeForSpace(Size availableSpace = Size::none()) const;

        std::shared_ptr<Layout> getLayout();
        void setLayout(std::shared_ptr<Layout> layout);
        void offerLayout(std::shared_ptr<Layout> layout);

        template <class T> void setLayoutStylesheet(T sheet) { layoutStylesheet = std::make_shared<T>(sheet); }

        std::shared_ptr<ViewCoreFactory> viewCoreFactory() { return _viewCoreFactory; }

        virtual std::list<std::shared_ptr<View>> childViews() { return std::list<std::shared_ptr<View>>(); }
        virtual void removeAllChildViews() {}
        virtual void childViewStolen(const std::shared_ptr<View> &childView) {}

        virtual std::shared_ptr<View> getParentView() { return _parentView.lock(); }

        void scheduleLayout();

        template <class T> auto core() { return std::dynamic_pointer_cast<T>(viewCore()); }

        std::shared_ptr<View::Core> viewCore();
        std::shared_ptr<View::Core> viewCore() const;

        virtual const std::type_info &typeInfoForCoreCreation() const;

        void setParentView(const std::shared_ptr<View> &parentView);

      protected:
        virtual void bindViewCore();

      protected:
        void onCoreLayout();
        void onCoreDirty();

      private:
        bool canMoveToParentView(const std::shared_ptr<View> &parentView);
        void updateLayout(const std::shared_ptr<Layout> &oldLayout, const std::shared_ptr<Layout> &newLayout);

      private:
        void lazyInitCore() const;

      protected:
        OfferedValue<std::shared_ptr<Layout>> _layout;

      private:
        mutable std::shared_ptr<View::Core> _core;
        WeakCallback<void()>::Receiver _layoutCallbackReceiver;
        WeakCallback<void()>::Receiver _dirtyCallbackReceiver;

        std::shared_ptr<ViewCoreFactory> _viewCoreFactory;
        std::weak_ptr<View> _parentView;
        bool _hasLayoutSchedulePending{false};

      public:
        class Core
        {
            friend class View;

          public:
            Property<Rect> geometry;
            Property<bool> visible;

          public:
            Core() = delete;
            Core(std::shared_ptr<bdn::ViewCoreFactory> viewCoreFactory);
            virtual ~Core() = default;

          public:
            virtual void init() = 0;

            virtual Size sizeForSpace(Size availableSize = Size::none()) const { return Size{0, 0}; }

            virtual bool canMoveToParentView(std::shared_ptr<View> newParentView) const = 0;

            virtual void scheduleLayout() = 0;

            void startLayout() { _layoutCallback.fire(); }
            void markDirty() { _dirtyCallback.fire(); }

            virtual void setLayout(std::shared_ptr<Layout> layout) { _layout = std::move(layout); }
            std::shared_ptr<Layout> layout() { return _layout; }

            virtual void visitInternalChildren(const std::function<void(std::shared_ptr<View::Core>)> & /*unused*/) {}

            std::shared_ptr<bdn::ViewCoreFactory> viewCoreFactory() { return _viewCoreFactory; }

          private:
            std::shared_ptr<bdn::ViewCoreFactory> _viewCoreFactory;
            std::shared_ptr<Layout> _layout;

            WeakCallback<void()> _layoutCallback;
            WeakCallback<void()> _dirtyCallback;
        };
    };

    template <typename ViewType, typename P> void registerCoreCreatingProperties(ViewType *view, P p)
    {
        p->onChange() += [=](auto) { view->viewCore(); };
    }

    template <typename ViewType, typename P, typename... Prest>
    void registerCoreCreatingProperties(ViewType *view, P p, Prest... rest)
    {
        registerCoreCreatingProperties(view, p);
        registerCoreCreatingProperties(view, rest...);
    }

    class SingleChildHelper
    {
      public:
        void update(const std::shared_ptr<View> &self, const std::shared_ptr<View> &newChild)
        {
            if (_currentChild) {
                _currentChild->setParentView(nullptr);
                _currentChild->offerLayout(nullptr);
            }
            _currentChild = newChild;

            if (newChild) {
                newChild->setParentView(self);
                newChild->offerLayout(self->getLayout());
            }
        }

      private:
        std::shared_ptr<View> _currentChild;
    };
}
