/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class FlangerAudioProcessorEditor  :public juce::AudioProcessorEditor, juce::Slider::Listener, juce::ComboBox::Listener
{
public:
    FlangerAudioProcessorEditor (FlangerAudioProcessor&);
    ~FlangerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    FlangerAudioProcessor& audioProcessor;
    
    juce::Slider mFeedbackSlider;
    juce::Label mFeedbackLabel;
    
    juce::Label mLfoFrequencyLabel;
    juce::Slider mLfoFrequencySlider;
    
    juce::ComboBox mLfoTypeBox;
    juce::Label mLfoTypeLabel;
    
    juce::ComboBox mLfoContraryMotionTypeBox;
    juce::Label mLfoContraryMotionTypeLabel;
    
    juce::Slider mLfoDepthSlider;
    juce::Label mLfoDepthLabel;
    
    juce::TextButton mClearBufButton;
    
    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;
    //void buttonClicked(juce::Button* button) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlangerAudioProcessorEditor)
};
