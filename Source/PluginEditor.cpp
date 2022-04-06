/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FlangerAudioProcessorEditor::FlangerAudioProcessorEditor (FlangerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (700, 500);
    
    mLfoFrequencySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    mLfoFrequencySlider.setRange(0.0f, 3.0f, 0.01f);
    mLfoFrequencySlider.setValue(audioProcessor.mLfoFreqSliderValue);
    addAndMakeVisible(&mLfoFrequencySlider);
    mLfoFrequencySlider.addListener(this);
    
    mFeedbackSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    mFeedbackSlider.setRange(0.0f, 99.0f);
    mFeedbackSlider.setValue(audioProcessor.mFeedbackGain * 100.0f);
    addAndMakeVisible(&mFeedbackSlider);
    mFeedbackSlider.addListener(this);
        
    mLfoDepthSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    mLfoDepthSlider.setRange(0.0f, 100.0f, 1.0f);
    mLfoDepthSlider.setValue(audioProcessor.getDepthTarget() * 100.0f);
    addAndMakeVisible(&mLfoDepthSlider);
    mLfoDepthSlider.addListener(this);
    
    mLfoTypeBox.addItem("Sine", 1);
    mLfoTypeBox.addItem("Saw", 2);
    mLfoTypeBox.setSelectedId(audioProcessor.mLfoFlag);
    addAndMakeVisible(&mLfoTypeBox);
    mLfoTypeBox.addListener(this);
    
    mLfoContraryMotionTypeBox.addItem("Contrary", 1);
    mLfoContraryMotionTypeBox.addItem("Sync", 2);
    mLfoContraryMotionTypeBox.setSelectedId(audioProcessor.mContraryMotionFlag);
    addAndMakeVisible(&mLfoContraryMotionTypeBox);
    mLfoContraryMotionTypeBox.addListener(this);
    
    addAndMakeVisible(&mLfoFrequencyLabel);
    mLfoFrequencyLabel.setText("Frequency", juce::dontSendNotification);
    mLfoFrequencyLabel.attachToComponent(&mLfoFrequencySlider, true);
    mLfoFrequencyLabel.setColour(juce::Label::textColourId, juce::Colours::magenta);
    mLfoFrequencyLabel.setJustificationType(juce::Justification::right);
    
    addAndMakeVisible(&mFeedbackLabel);
    mFeedbackLabel.setText("Feedback", juce::dontSendNotification);
    mFeedbackLabel.attachToComponent(&mFeedbackSlider, true);
    mFeedbackLabel.setColour(juce::Label::textColourId, juce::Colours::magenta);
    mFeedbackLabel.setJustificationType(juce::Justification::right);
    
    addAndMakeVisible(&mLfoDepthLabel);
    mLfoDepthLabel.setText("Depth", juce::dontSendNotification);
    mLfoDepthLabel.attachToComponent(&mLfoDepthSlider, true);
    mLfoDepthLabel.setColour(juce::Label::textColourId, juce::Colours::magenta);
    mLfoDepthLabel.setJustificationType(juce::Justification::right);
    
    addAndMakeVisible(&mLfoTypeLabel);
    mLfoTypeLabel.setText("LFO Type", juce::dontSendNotification);
    mLfoTypeLabel.attachToComponent(&mLfoTypeBox, true);
    mLfoTypeLabel.setColour(juce::Label::textColourId, juce::Colours::magenta);
    mLfoTypeLabel.setJustificationType(juce::Justification::right);
    
    addAndMakeVisible(&mLfoContraryMotionTypeLabel);
    mLfoContraryMotionTypeLabel.setText("Motion", juce::dontSendNotification);
    mLfoContraryMotionTypeLabel.attachToComponent(&mLfoContraryMotionTypeBox, true);
    mLfoContraryMotionTypeLabel.setColour(juce::Label::textColourId, juce::Colours::magenta);
    mLfoContraryMotionTypeLabel.setJustificationType(juce::Justification::right);
}

FlangerAudioProcessorEditor::~FlangerAudioProcessorEditor()
{
    mLfoFrequencySlider.removeListener(this);
    mFeedbackSlider.removeListener(this);
    mLfoDepthSlider.removeListener(this);
    mLfoTypeBox.removeListener(this);
}

//==============================================================================
void FlangerAudioProcessorEditor::sliderValueChanged(juce::Slider *slider)

{
    if (slider == &mLfoDepthSlider)
    {
        double depth = mLfoDepthSlider.getValue()/100.0f;
        
        audioProcessor.setDepthTarget(depth);
        
        DBG("LFO Depth: " + juce::String(depth));
    }
    else if (slider == &mFeedbackSlider)
        audioProcessor.mFeedbackGain = mFeedbackSlider.getValue()/100.0f;
    else if (slider == &mLfoFrequencySlider)
    {
        audioProcessor.mLfoFreqSliderValue = mLfoFrequencySlider.getValue();
        audioProcessor.setLfoFreq(audioProcessor.mLfoFreqSliderValue);
    }
    
}

void FlangerAudioProcessorEditor::comboBoxChanged(juce::ComboBox *comboBox)
{
       switch (mLfoContraryMotionTypeBox.getSelectedId())
       {
           case contrary:
               audioProcessor.mContraryMotionFlag = true;
               break;
           case sync:
               audioProcessor.mContraryMotionFlag = false;
               break;
           default:
               break;
       }
    
        switch (mLfoTypeBox.getSelectedId())
        {
            case sine:
                audioProcessor.mLfoFlag = true;
                break;
            case saw:
                audioProcessor.mLfoFlag = false;
        }
}
//==============================================================================
void FlangerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Flanger", getLocalBounds(), juce::Justification::centred, 1);
}

void FlangerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    mLfoFrequencySlider.setBounds(100, 25, 300, 50);
    
    mLfoDepthSlider.setBounds(200, 100, 300, 50);
    
    mFeedbackSlider.setBounds(300, 150, 300, 50);
    
    mLfoTypeBox.setBounds(150, 300, 75, 50);
    
    mLfoContraryMotionTypeBox.setBounds(350, 300, 75, 50);
}

