/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FlangerAudioProcessor::FlangerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    mSampleRate = 48000;
    mBlockSize = 1024;
   // mDelayTimeSamps = DELAYTIMESAMPSINIT;
   // mFeedbackGain = FEEDBACKGAININIT;
   // mLfoFreqSliderValue = LFOFREQINIT;
    
    
    
    mRingBuf.debug(true);
}

FlangerAudioProcessor::~FlangerAudioProcessor()
{
}

//==============================================================================
void FlangerAudioProcessor::setLfoFreq(double freq)
{
    for (int channel = 0; channel < mNumInputChannels; channel++)
    {
        if (mContraryMotionFlag)
        {
            if (channel == 0)
                mLfoArray[channel].setFreq(freq);
            else
                mLfoArray[channel].setFreq(freq * -1);
        }
        else
        {
            mLfoArray[channel].setFreq(freq);
        }
        
        mLfoArray[channel].setPhase(0.0);
    }
}

void FlangerAudioProcessor::setLfoType(int type)
{
    for (int channel = 0; channel < mNumInputChannels; channel++)
    {
        switch (mLfoTypeComboBoxValue)
        {
            case 1:
                mLfoArray[channel].setType(atec::LFO::sin);
                break;
            case 2:
                mLfoArray[channel].setType(atec::LFO::saw);
                break;
            default:
                break;
        }
    }
}

void FlangerAudioProcessor::setDepthTarget(double depth)
{
    for (int channel = 0; channel < mNumInputChannels; channel++)
        mLfoDepth[channel].setTargetValue(depth);
}

double FlangerAudioProcessor::getDepthTarget()
{
    return mLfoDepth[0].getTargetValue();
}


const juce::String FlangerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FlangerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FlangerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FlangerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FlangerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FlangerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FlangerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FlangerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FlangerAudioProcessor::getProgramName (int index)
{
    return {};
}

void FlangerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FlangerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    mNumInputChannels = getTotalNumInputChannels();
    
    mSampleRate = sampleRate;
    mBlockSize = samplesPerBlock;
    
    mMaxDelaySamps = MAXDELAYTIME * mSampleRate;
    
    // make the ring buffer size a multiple of the host block size for wraparound convenience
    mRingBuffSize = sampleRate * 1.25;
    // in units of blocks
    mRingBuffSize = floor(mRingBuffSize/(float)mBlockSize);
    mRingBuffSize *= mBlockSize;
    
    // 3-second ring buffer
    mRingBuf.setSize(mNumInputChannels, 3.0f * mSampleRate, mBlockSize);
    mRingBuf.init();
    
    mDelayTimeSamps[0].setTargetValue(DELAYTIMESAMPSINIT);
    mDelayTimeSamps[1].setTargetValue(DELAYTIMESAMPSINIT);
    // set ramp time to 3 sec
    mDelayTimeSamps[0].reset(mSampleRate, 3.0f);
    mDelayTimeSamps[1].reset(mSampleRate, 3.0f);
    
    mRingBuffWriteIdx = 0;
    
    for (int channel = 0; channel < mNumInputChannels; channel++)
    {
        mLfoArray[channel].setType(atec::LFO::saw);
        mLfoArray[channel].setSampleRate(mSampleRate);
        mLfoArray[channel].setFreq(7.0f);
        
        mLfoDepth[channel].setTargetValue(LFODEPTHINIT);
        mLfoDepth[channel].reset(mSampleRate, 3.0f);
    }
}

void FlangerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FlangerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void FlangerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    auto buffSize = buffer.getNumSamples();
    juce::AudioBuffer<float> delayBlock;

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // buffer to store per-block processing, should be the same size as incoming buffer from host.
    delayBlock.setSize(totalNumInputChannels, buffSize);
    // clear the delay block buffer for good measure (all channels)
    delayBlock.clear();
    
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        //auto* channelData = buffer.getWritePointer (channel);
        auto* delayBlockPtr = delayBlock.getWritePointer(channel);
        //auto* ringBuffPtr = mRingBuf.getReadPointer(channel);
      
        for (int i = 0; i < buffSize; i++)
        {
            double thisDelayTime;
            
            if (mLfoFreqSliderValue > 0.0)
                thisDelayTime = mLfoArray[channel].getNextSample() * mLfoDepth[channel].getNextValue() * mMaxDelaySamps;
            
            else
                thisDelayTime = mLfoArray[channel].getNextSample() * mMaxDelaySamps;
            
            delayBlockPtr[i] = mRingBuf.readInterpSample(channel, i, thisDelayTime);
            // return delayBlock buffer
            // want to return samples from the Ring Buffer into the delay block buffer, using the readInterpSample function of Ring Buffer
        }

        // reduce amplitude of the delayed block
        delayBlock.applyGain(channel, 0, buffSize, mFeedbackGain);
        // ..do something to the data...
        buffer.addFrom(channel, 0, delayBlock, channel, 0, buffSize);
    }
    
    mRingBuf.write(buffer);
    
    buffer.applyGain(0, buffSize, 0.25);
}

//==============================================================================
bool FlangerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FlangerAudioProcessor::createEditor()
{
    return new FlangerAudioProcessorEditor (*this);
}

//==============================================================================
void FlangerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void FlangerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FlangerAudioProcessor();
}
