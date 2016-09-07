#ifndef BDN_WINUWP_TextViewCore_H_
#define BDN_WINUWP_TextViewCore_H_

#include <bdn/TextView.h>
#include <bdn/winuwp/ChildViewCore.h>

namespace bdn
{
namespace winuwp
{

class TextViewCore : public ChildViewCore, BDN_IMPLEMENTS ITextViewCore
{
private:
	static ::Windows::UI::Xaml::Controls::TextBlock^ _createTextBlock(TextView* pOuter)
	{
        BDN_WINUWP_TO_STDEXC_BEGIN;

		return ref new ::Windows::UI::Xaml::Controls::TextBlock();		

        BDN_WINUWP_TO_STDEXC_END;
	}

public:

	TextViewCore(	TextView* pOuter)
		: ChildViewCore(pOuter, _createTextBlock(pOuter), ref new ViewCoreEventForwarder(this) )
	{
        BDN_WINUWP_TO_STDEXC_BEGIN;

		_pTextBlock = dynamic_cast< ::Windows::UI::Xaml::Controls::TextBlock^ >( getFrameworkElement() );

		setPadding( pOuter->padding() );
		setText( pOuter->text() );

        BDN_WINUWP_TO_STDEXC_END;
	}

    void dispose() override
    {
        _pTextBlock = nullptr;

        ChildViewCore::dispose();
    }

	void setPadding(const Nullable<UiMargin>& pad) override
	{
        BDN_WINUWP_TO_STDEXC_BEGIN;

		// Apply the padding to the control, so that the content is positioned accordingly.
        UiMargin uiPadding;
        if(pad.isNull())
        {
            // we should use a default padding that looks good.
            // Xaml uses zero padding as the default, so we cannot use their
            // default value. So we choose our own default that matches the
            // normal aesthetic of Windows apps.
            uiPadding = UiMargin(UiLength::sem, 0.4, 1);
        }
        else
            uiPadding = pad;

		Margin padding = UiProvider::get().uiMarginToPixelMargin(uiPadding);

		_doSizingInfoUpdateOnNextLayout = true;		

		double uiScaleFactor = UiProvider::get().getUiScaleFactor();

		_pTextBlock->Padding = ::Windows::UI::Xaml::Thickness(
			padding.left/uiScaleFactor,
			padding.top/uiScaleFactor,
			padding.right/uiScaleFactor,
			padding.bottom/uiScaleFactor );
        
        BDN_WINUWP_TO_STDEXC_END;
	}

	void setText(const String& text)
	{
        BDN_WINUWP_TO_STDEXC_BEGIN;

        // we cannot simply schedule a sizing info update here. The desired size of the control will still
		// be outdated when the sizing happens.
		// Instead we wait for the "layout updated" event that will happen soon after we set the
		// content. That is when we update our sizing info.
        _doSizingInfoUpdateOnNextLayout = true;		
        
		_pTextBlock->Text = ref new ::Platform::String( text.asWidePtr() );

        BDN_WINUWP_TO_STDEXC_END;
	}


protected:

	void _layoutUpdated() override
	{
		if(_doSizingInfoUpdateOnNextLayout)
		{
			_doSizingInfoUpdateOnNextLayout = false;

            View* pOuterView = getOuterView();
            if(pOuterView!=nullptr)
			    pOuterView->needSizingInfoUpdate();
		}
	}
    
	::Windows::UI::Xaml::Controls::TextBlock^ _pTextBlock;

	double      _doSizingInfoUpdateOnNextLayout = true;
	
};

}
}

#endif