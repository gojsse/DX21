#include "PluginProcessor.h"

OP4Processor::OP4Processor() : AudioProcessor(BusesProperties()
    .withOutput("Output", juce::AudioChannelSet::stereo())) {
  engine = std::make_unique<FMEngine>();
}

void OP4Processor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  engine->prepare(sampleRate, samplesPerBlock);
}

void OP4Processor::releaseResources() {
  engine->release();
}

void OP4Processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  // TODO: process MIDI messages
  // TODO: call engine->processBlock(buffer)

  buffer.clear(); // stub: silence
}

juce::AudioProcessorEditor* OP4Processor::createEditor() {
  return nullptr; // WebView editor (M1)
}

void OP4Processor::getStateInformation(juce::MemoryBlock& destData) {
  // TODO: serialize currentPatch to destData
}

void OP4Processor::setStateInformation(const void* data, int sizeInBytes) {
  // TODO: deserialize currentPatch from data
}

// Factory
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new OP4Processor();
}
