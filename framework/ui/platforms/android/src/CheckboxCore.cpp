#include <bdn/android/CheckboxCore.h>

namespace bdn::detail
{
    CORE_REGISTER(Checkbox, bdn::android::CheckboxCore, Checkbox)
}

namespace bdn::android
{

    CheckboxCore::CheckboxCore(const std::shared_ptr<ViewCoreFactory> &viewCoreFactory)
        : ViewCore(viewCoreFactory, createAndroidViewClass<wrapper::CheckBox>(viewCoreFactory)),
          _jCheckBox(getJViewAS<wrapper::CheckBox>())
    {
        _jCheckBox.setSingleLine(true);

        label.onChange() += [=](auto va) {
            _jCheckBox.setText(va->get());
            scheduleLayout();
        };

        state.onChange() += [=](auto va) {
            _jCheckBox.setChecked(va->get() == TriState::on);
            _state = state;
        };

        bdn::android::wrapper::NativeViewCoreClickListener listener;
        _jCheckBox.setOnClickListener(listener);
    }

    TriState CheckboxCore::getState() const { return _state; }

    void CheckboxCore::clicked() { _clickCallback.fire(); }
}
