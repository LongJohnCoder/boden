#import <Cocoa/Cocoa.h>
#import <bdn/mac/TextFieldCore.hh>

@interface BdnTextFieldDelegate : NSObject <NSTextFieldDelegate>

@property(nonatomic, assign) std::weak_ptr<bdn::mac::TextFieldCore> textFieldCore;
@property(nonatomic, weak) NSTextField *nsTextField;

- (void)controlTextDidChange:(NSNotification *)obj;
- (void)submitted;
- (void)setNsTextField:(NSTextField *)nsTextField;

@end

@implementation BdnTextFieldDelegate

- (void)setNsTextField:(NSTextField *)nsTextField
{
    _nsTextField = nsTextField;
    _nsTextField.delegate = self;
    _nsTextField.target = self;
    _nsTextField.action = @selector(submitted);
}

- (void)controlTextDidChange:(NSNotification *)obj
{
    if (auto core = self.textFieldCore.lock()) {
        NSTextView *textView = [obj.userInfo objectForKey:@"NSFieldEditor"];
        core->text = bdn::fk::nsStringToString(textView.textStorage.string);
    }
}

- (void)submitted
{
    if (auto core = self.textFieldCore.lock()) {
        core->submitCallback.fire();
    }
}

@end

namespace bdn::detail
{
    CORE_REGISTER(TextField, bdn::mac::TextFieldCore, TextField)
}

namespace bdn::mac
{
    NSTextField *TextFieldCore::_createNsTextView()
    {
        NSTextField *textField = [[NSTextField alloc] init];
        textField.allowsEditingTextAttributes = NO; // plain textfield, no attribution/formatting
        textField.cell.wraps = NO;                  // no word wrapping
        textField.cell.scrollable = YES;            // but scroll horizontally instead
        return textField;
    }

    TextFieldCore::TextFieldCore(const std::shared_ptr<bdn::ViewCoreFactory> &viewCoreFactory)
        : bdn::mac::ViewCore(viewCoreFactory, _createNsTextView())
    {}

    TextFieldCore::~TextFieldCore()
    {
        _delegate.textFieldCore = std::weak_ptr<TextFieldCore>();
        _delegate = nil;
    }

    void TextFieldCore::init()
    {
        bdn::mac::ViewCore::init();

        _delegate = [[BdnTextFieldDelegate alloc] init];
        _delegate.textFieldCore = shared_from_this<TextFieldCore>();
        _delegate.nsTextField = (NSTextField *)nsView();

        text.onChange() += [=](auto va) {
            NSTextField *textField = (NSTextField *)nsView();
            if (fk::nsStringToString(textField.stringValue) != va->get()) {
                textField.stringValue = fk::stringToNSString(va->get());
            }
        };
    }
}
