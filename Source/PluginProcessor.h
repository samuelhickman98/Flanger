/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define DELAYTIMESAMPSINIT 480
#define FEEDBACKGAININIT 0.85
#define LFODEPTHINIT 0.5
#define MAXDELAYTIME .025

enum motionType
{
    contrary = 1,
    sync
};

enum lfoType
{
    sine = 1,
    saw
};
//==============================================================================
/**
*/
class FlangerAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    FlangerAudioProcessor();
    ~FlangerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    int mNumInputChannels;
    double mSampleRate;
    double mBlockSize;
    double mFeedbackGain;
    double mLfoFreqSliderValue;
    atec::LFO::LfoType mLfoTypeComboBoxValue;
    void setLfoFreq(double freq);
    void setLfoType(int type);
    void setDepthTarget(double depth);
    double getDepthTarget();
    int mRingBuffSize;
    juce::SmoothedValue<double, juce::ValueSmoothingTypes::Linear> mDelayTimeSamps[2];
    bool mLfoFlag;
    bool mContraryMotionFlag;
    double slider;
    
private:
    
    int buffSize;
    int mRingBuffWriteIdx;
    int delayBlock;

    //juce::AudioBufer<float> mRingbuf;
    atec::RingBuffer mRingBuf;
    atec::LFO mLfoArray[2];
    juce::SmoothedValue<double, juce::ValueSmoothingTypes::Linear> mLfoDepth[2];
    double mMaxDelaySamps;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlangerAudioProcessor)
};
