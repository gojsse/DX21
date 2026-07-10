#include "PluginProcessor.h"
#include "PluginEditor.h"

OP4Processor::OP4Processor() : AudioProcessor(BusesProperties()
    .withOutput("Output", juce::AudioChannelSet::stereo())) {
  engine = std::make_unique<FMEngine>();
  engine->setPatch(currentPatch);
}

void OP4Processor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  engine->prepare(sampleRate, samplesPerBlock);
}

void OP4Processor::releaseResources() {
  engine->release();
}

void OP4Processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  const int numSamples = buffer.getNumSamples();

  // Clear any output channels beyond what we render to.
  for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
    buffer.clear(i, 0, numSamples);

  float* left = buffer.getWritePointer(0);
  float* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : left;
  engine->processBlock(left, right, numSamples, midiMessages);
}

juce::AudioProcessorEditor* OP4Processor::createEditor() {
  return new OP4Editor(*this);
}

void OP4Processor::getStateInformation(juce::MemoryBlock& destData) {
  // TODO(M2): serialize currentPatch (via VCED codec) into destData for DAW recall.
  juce::ignoreUnused(destData);
}

void OP4Processor::setStateInformation(const void* data, int sizeInBytes) {
  // TODO(M2): deserialize currentPatch and engine->setPatch(currentPatch).
  juce::ignoreUnused(data, sizeInBytes);
}

// Factory
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new OP4Processor();
}
