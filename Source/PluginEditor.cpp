#include "PluginEditor.h"
#include <UIData.h>
#include <cstring>

static std::vector<std::byte> toBytes (const char* data, int size)
{
    std::vector<std::byte> v ((size_t) size);
    std::memcpy (v.data(), data, (size_t) size);
    return v;
}

SHZEditor::SHZEditor (SHZProcessor& p) : AudioProcessorEditor (&p), proc (p)
{
    using Opt = juce::WebBrowserComponent::Options;
    Opt opts = Opt{}
        .withNativeIntegrationEnabled()
        .withKeepPageLoadedWhenBrowserIsHidden()
        .withWinWebView2Options (Opt::WinWebView2{}
            .withUserDataFolder (juce::File::getSpecialLocation (juce::File::tempDirectory)
                                     .getChildFile ("SISHHIN_HZ_MACHINE_WebView2")))
        .withResourceProvider ([this] (const juce::String& url) { return getResource (url); })
        .withNativeFunction ("loadIR",
            [this] (const juce::Array<juce::var>&, juce::WebBrowserComponent::NativeFunctionCompletion done)
            {
                chooser = std::make_unique<juce::FileChooser> ("Load impulse response", juce::File(),
                                                               "*.wav;*.aif;*.aiff;*.flac");
                juce::Component::SafePointer<SHZEditor> safe (this);
                chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                    [safe, done] (const juce::FileChooser& fc)
                    {
                        if (safe == nullptr) { done (juce::var()); return; }
                        const auto f = fc.getResult();
                        if (f.existsAsFile())
                            safe->proc.loadIRFile (f);
                        done (safe->proc.getIRName());
                    });
            })
        .withNativeFunction ("clearIR",
            [this] (const juce::Array<juce::var>&, juce::WebBrowserComponent::NativeFunctionCompletion done)
            {
                proc.clearIR();
                done (juce::String());
            })
        .withNativeFunction ("getIR",
            [this] (const juce::Array<juce::var>&, juce::WebBrowserComponent::NativeFunctionCompletion done)
            {
                done (proc.getIRName());
            });

    // one relay per parameter; relays must contribute their options BEFORE the browser is created
    for (const auto& pd : kParams)
    {
        relays.push_back (std::make_unique<juce::WebSliderRelay> (pd.id));
        opts = std::move (opts).withOptionsFrom (*relays.back());
    }

    web = std::make_unique<juce::WebBrowserComponent> (opts);
    addAndMakeVisible (*web);

    for (int i = 0; i < Id::Count; ++i)
    {
        auto* param = proc.apvts.getParameter (kParams[i].id);
        attachments.push_back (std::make_unique<juce::WebSliderParameterAttachment> (*param, *relays[(size_t) i], nullptr));
    }

    web->goToURL (juce::WebBrowserComponent::getResourceProviderRoot());

    setResizable (true, true);
    setResizeLimits (720, 520, 1700, 1200);
    setSize (1080, 760);
    startTimerHz (30);
}

SHZEditor::~SHZEditor()
{
    stopTimer();
}

void SHZEditor::resized()
{
    if (web != nullptr)
        web->setBounds (getLocalBounds());
}

void SHZEditor::timerCallback()
{
    auto* o = new juce::DynamicObject();
    o->setProperty ("in",  (double) proc.engine.inPeak.load());
    o->setProperty ("out", (double) proc.engine.outPeak.load());
    web->emitEventIfBrowserIsVisible ("meters", juce::var (o));
}

std::optional<juce::WebBrowserComponent::Resource> SHZEditor::getResource (const juce::String& url)
{
    const auto path = url.upToFirstOccurrenceOf ("?", false, false);

    if (path == "/" || path.isEmpty() || path == "/index.html")
        return juce::WebBrowserComponent::Resource { toBytes (UIData::index_html, UIData::index_htmlSize), "text/html" };

    if (path == "/js/juce/index.js")
        return juce::WebBrowserComponent::Resource { toBytes (UIData::index_js, UIData::index_jsSize), "text/javascript" };

    return std::nullopt;
}
