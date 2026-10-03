#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include <optional>
#include "PluginProcessor.h"

class SHZEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit SHZEditor (SHZProcessor&);
    ~SHZEditor() override;

    void resized() override;
    void timerCallback() override;

private:
    std::optional<juce::WebBrowserComponent::Resource> getResource (const juce::String& url);

    SHZProcessor& proc;

    // destruction order matters: attachments -> browser -> relays
    std::vector<std::unique_ptr<juce::WebSliderRelay>> relays;
    std::unique_ptr<juce::WebBrowserComponent> web;
    std::vector<std::unique_ptr<juce::WebSliderParameterAttachment>> attachments;
    std::unique_ptr<juce::FileChooser> chooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SHZEditor)
};
