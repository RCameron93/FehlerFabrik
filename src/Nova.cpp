// Cut-up Sequenced Sampler
// Ross Cameron 2021/01/18
// Title font - Citytype Miami
// https://www.citype.net/city/miami
// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"
#include "ffCommon.hpp"
#include <array>

// Each sample buffer is 2097152 samples long - 47 seconds at 44.1KHz
static const int bufferSize = 1 << 21;

// struct Sequencer
// {
// 	bool running = false;
// 	// 0 = fwd, 1 = rev, 2 = bounce, 3 = rnd
// 	int direction = 0;
// 	// Just used in bounce mode
// 	int bounceDir = 0;

// 	int index = 0; // Sequencer index

// 	void reset()
// 	{
// 		index = 0;
// 	}

// 	void setIndex(int step)
// 	{
// 		if (step < 8 && step > -1)
// 		{
// 			index = step;
// 		}
// 	}

// 	void startStop()
// 	{
// 		running = !running;
// 	}

// 	void directionChange()
// 	{
// 		// Cycle through direction modes
// 		++direction;
// 		direction %= 4;
// 	}

// 	void advanceIndex()
// 	{
// 		switch (direction)
// 		{
// 		case 0:
// 			// Forward
// 			++index;
// 			index %= 8;
// 			break;

// 		case 1:
// 			// Reverse
// 			--index;
// 			index = (index % 8 + 8) % 8;
// 			break;

// 		case 2:
// 			// Bouncing (plays each end twice so as to be a factor of four)
// 			if (bounceDir)
// 			{
// 				++index;
// 				if (index == 8)
// 				{
// 					bounceDir = !bounceDir;
// 					index = 7;
// 				}
// 			}
// 			else
// 			{
// 				--index;
// 				if (index == -1)
// 				{
// 					bounceDir = !bounceDir;
// 					index = 0;
// 				}
// 			}

// 			break;

// 		case 3:
// 			// Random
// 			{
// 				float rng = 7 * random::uniform();
// 				index = (int)round(rng);
// 			}
// 			break;

// 		default:
// 			break;
// 		}
// 	}
// };

// Data structure for storing sampled audio
struct SampleBuffer
{
	std::array<float, bufferSize> sampleBuff;
	int index = 0;
	int length = 0;
	int capacity = bufferSize;
	float originalSampleRate = 0.f;
	float currentSampleRate = 0.f;
	bool full = false;
	bool finished = false;
	bool empty = true;

	void push(float input, float sampleRate)
	{
		if (index < capacity)
		{
			empty = false;
			sampleBuff[index] = input;
			originalSampleRate = sampleRate;
			++index;
			++length;
			return;
		}
		else
		{
			full = true;
			return;
		}
	}

	void resetIndex(bool reversed)
	{
		finished = false;

		if (!reversed)
		{
			index = 0;
		}
		else
		{
			index = length - 1;
		}
	}

	void clear()
	{
		// sampleBuff.fill(0.f);
		empty = true;
		length = 0;
		originalSampleRate = 0.f;
		currentSampleRate = 0.f;
	}

	float play(bool reversed)
	{
		if (finished)
		{
			return 0.f;
		}

		else if (!reversed)
		{
			float output = sampleBuff[index];
			++index;
			if (index > length - 1)
			{
				finished = true;
			}
			return output;
		}

		else
		{
			float output = sampleBuff[index];
			--index;
			if (index < 0)
			{
				finished = true;
			}
			return output;
		}
	}
};

struct Sampler
{
	SampleBuffer inBuffer;
	SampleBuffer outBuffer;

	dsp::SampleRateConverter<1> inputSrc;

	float output = 0.f;
	float previousPitch = 0.f;

	// Clears everything from the input and output buffers
	void clear()
	{
		inBuffer.clear();
		outBuffer.clear();
	}

	// Reset all indices ready to begin playback
	void resetIndex(bool reverse)
	{
		// We reset to the end of the recorded buffer if we're about to playback in reverse
		inBuffer.resetIndex(reverse);
		outBuffer.resetIndex(reverse);
	}

	// Convert from original sample rate a buffer was recorded at to new one determined by the pitch control
	void convert(float pitch, float sampleRate)
	{
		previousPitch = pitch;

		// Clear previous outBuffer
		outBuffer.clear();

		// Expecting the input paramter to be between -1.f and 1.f
		// Produces a pitch/speed change from 1/4 to 4 times
		float ratio = pow(4, -pitch);

		// New sample rate we're converting to
		float newRate = inBuffer.originalSampleRate * ratio;
		outBuffer.currentSampleRate = newRate;

		// Prepare sample rate converter
		inputSrc.setRates(inBuffer.originalSampleRate, newRate);
		int inLen = inBuffer.length + 1;
		int outLen = std::min((int)(inLen * ratio), (int)bufferSize);
		outBuffer.length = outLen;

		// Process the sample rate conversion
		inputSrc.process((dsp::Frame<1> *)&inBuffer.sampleBuff[0], &inLen, (dsp::Frame<1> *)&outBuffer.sampleBuff[0], &outLen);
	}

	void preparePlayback(bool reverse, float pitch)
	{
		resetIndex(reverse);

		// This should only (and always) be true after a recording has just been completed
		// This is where we'll initially set previousPitch for this recording
		if (outBuffer.currentSampleRate == 0.f)
		{
			// Just copy from the in buffer to the output without any conversion
			if (pitch == 0.f)
			{
				// This should copy over to the output buffer
				inBuffer.currentSampleRate = inBuffer.originalSampleRate;
				outBuffer = inBuffer;
				previousPitch = 0.f;
				return;
			}
			// Copy into the outBuffer via sample rate conversion
			else
			{
				convert(pitch, inBuffer.originalSampleRate);
				return;
			}
		}
		// Output buffer has a sample rate, and it's what the pitch control says it should be
		else if (pitch == previousPitch)
		{
			// Do nothing here, outBuffer should be correct
			return;
		}
		// Output buffer has a sample rate, but we have a new pitch value
		else
		{
			convert(pitch, inBuffer.originalSampleRate);
			return;
		}
	}

	void prepareRecording()
	{
		// Clear both buffers
		clear();
		// We always record moving forwards through the buffer
		resetIndex(false);
	}

	// Records a sample into the input buffer
	void record(float input, float sampleRate)
	{
		inBuffer.push(input, sampleRate);
	}

	// Gets the the next sample from the output buffer
	float play(bool reverse)
	{
		output = outBuffer.play(reverse);
		return output;
	}

	// We use this value to determine the colour of the sequencer LED during playback
	float playheadPercent()
	{
		float percent = (float)outBuffer.index / (float)outBuffer.length;
		return percent;
	}
};

struct Nova : Module
{
	enum ParamIds
	{
		START_PARAM,
		RESET_PARAM,
		DIRECTION_PARAM,
		RECORD_PARAM,
		ENUMS(GAINS_PARAM, 8),
		ENUMS(MUTES_PARAM, 8),
		ENUMS(SKIPS_PARAM, 8),
		ENUMS(REVERSES_PARAM, 8),
		ENUMS(TRIGGERS_PARAM, 8),
		ATTACK_PARAM,
		RELEASE_PARAM,
		PITCH_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		IN_INPUT,
		CLOCK_INPUT,
		START_INPUT,
		RESET_INPUT,
		DIRECTION_INPUT,
		RECORD_INPUT,
		ENUMS(TRIGGERS_INPUT, 8),

		NUM_INPUTS
	};
	enum OutputIds
	{
		ENUMS(OUTS_OUTPUT, 8),
		MAINOUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		ENUMS(SEQS_LIGHT, 24),
		REC_LIGHT,
		NUM_LIGHTS
	};
	Sequencer sequencer;
	Sampler samplers[8];
	Ramp ramp;

	dsp::SchmittTrigger clockTrigger;
	dsp::SchmittTrigger startTrigger;
	dsp::SchmittTrigger clearTrigger;
	dsp::SchmittTrigger directionTrigger;
	dsp::SchmittTrigger recordTrigger;
	dsp::SchmittTrigger jumpTriggers[8];

	bool recording = false;

	float gains[8] = {1.f};
	// 0 = muted, 1 = active
	float mutes[8] = {1.f};
	// 0 = fwd 1 = rev
	bool reverses[8] = {0};
	bool skips[8] = {0};

	Nova()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configButton(START_PARAM, "Sequencer Start");
		configButton(RESET_PARAM, "Sequencer Reset");
		configButton(DIRECTION_PARAM, "Sequencer Direction");
		configButton(RECORD_PARAM, "Sampler Record Start");

		for (int i = 0; i < 8; ++i)
		{
			configParam(GAINS_PARAM + i, 0.f, 1.f, 1.f, string::f("Sample %d Gain", i + 1), "dB", -10.f, 20.f);
			configSwitch(MUTES_PARAM + i, 0.f, 1.f, 0.f, string::f("Sample %d Mute", i + 1), {"Unmuted", "Muted"});
			configSwitch(SKIPS_PARAM + i, 0.f, 1.f, 0.f, string::f("Sample %d Skip", i + 1), {"", "Skipped"});
			configSwitch(REVERSES_PARAM + i, 0.f, 1.f, 0.f, string::f("Sample %d Reverse", i + 1), {"Forward", "Reversed"});
			configButton(TRIGGERS_PARAM + i, string::f("Sample %d Trigger", i + 1));

			configInput(TRIGGERS_INPUT + i, string::f("Step %d Trigger", i + 1));
			configOutput(OUTS_OUTPUT + i, string::f("Step %d", i + 1));
			configLight(SEQS_LIGHT + i * 3, string::f("Step %d", i * 3 + 1));
		}

		configParam(ATTACK_PARAM, 0.f, 1.f, 0.f, "Global Sample Attack");
		configParam(RELEASE_PARAM, 0.f, 1.f, 1.f, "Global Sample Release");
		configParam(PITCH_PARAM, -1.f, 1.f, 0.f, "Global Sample Pitch", "x", 4.f);

		configInput(IN_INPUT, "Sampler");
		configInput(CLOCK_INPUT, "Clock Trigger");
		configInput(START_INPUT, "Start Trigger");
		configInput(RESET_INPUT, "Reset Trigger");
		configInput(DIRECTION_INPUT, "Direction Trigger");
		configInput(RECORD_INPUT, "Record Arm Trigger");

		configOutput(MAINOUT_OUTPUT, "Main");

		configLight(REC_LIGHT, "Record Arm");

		configBypass(IN_INPUT, MAINOUT_OUTPUT);
	}

	void displayLED()
	{
		// Set all to off
		for (int i = 0; i < 8; ++i)
		{
			lights[SEQS_LIGHT + i * 3].setBrightness(0);
			lights[SEQS_LIGHT + i * 3 + 1].setBrightness(0);
			lights[SEQS_LIGHT + i * 3 + 2].setBrightness(0);
		}

		int *index = &sequencer.index;

		if (!mutes[*index])
		{
			// Sample is muted
			lights[SEQS_LIGHT + sequencer.index * 3].setBrightness(1);
			lights[SEQS_LIGHT + sequencer.index * 3 + 1].setBrightness(0);
		}
		else if (samplers[*index].inBuffer.empty)
		{
			// No sample recorded
			lights[SEQS_LIGHT + sequencer.index * 3].setBrightness(0);
			lights[SEQS_LIGHT + sequencer.index * 3 + 1].setBrightness(1);
		}
		else
		{
			// Sample has been recorded
			// Should change from green to yellow to indicate playback progress through the sample buffer
			lights[SEQS_LIGHT + sequencer.index * 3].setBrightness(samplers[*index].playheadPercent());
			lights[SEQS_LIGHT + sequencer.index * 3 + 1].setBrightness(1);
		}
	}

	void process(const ProcessArgs &args) override
	{
		float outs[8] = {0.f};
		float mainOut = 0.f;

		int jumpTo = -1;

		// ENVELOPE PARAMS
		// Get rates
		// These are probably capable of being too fast, a lot of the knob range isn't useful
		float attack = params[ATTACK_PARAM].getValue();
		float release = params[RELEASE_PARAM].getValue();

		// Get pitch change amount
		float pitch = params[PITCH_PARAM].getValue();

		// SEQUENCER PARAMS
		// Externally clocked only

		// Check for starts/stops
		if (startTrigger.process(inputs[START_INPUT].getVoltage() + params[START_PARAM].getValue()))
		{
			sequencer.startStop();
		}

		// Check for direction change
		if (directionTrigger.process(inputs[DIRECTION_INPUT].getVoltage() + params[DIRECTION_PARAM].getValue()))
		{
			sequencer.directionChange();
		}

		// Check for recording armed
		if (recordTrigger.process(inputs[RECORD_INPUT].getVoltage() + params[RECORD_PARAM].getValue()))
		{
			recording = !recording;
		}

		// SAMPLERS PARAMS
		// Check each sample players state
		for (int i = 0; i < 8; ++i)
		{
			// Get gains
			gains[i] = params[GAINS_PARAM + i].getValue();

			// Check for mutes
			// Annoyingly I think we have to have 0 = unmuted, 1 = muted for the button to look right, so we do a funny conversion here to get a float we can multiply our sample by
			mutes[i] = (float)!bool(params[MUTES_PARAM + i].getValue());

			// Check for reverses
			reverses[i] = (bool)params[REVERSES_PARAM + i].getValue();

			// Check for skips
			skips[i] = (bool)params[SKIPS_PARAM + i].getValue();

			// Check for jumps
			if (jumpTriggers[i].process(inputs[TRIGGERS_INPUT + i].getVoltage() + params[TRIGGERS_PARAM + i].getValue()))
			{
				jumpTo = i;
			}
		}

		// Check for sample buffer clear
		if (clearTrigger.process(inputs[RESET_INPUT].getVoltage() + params[RESET_PARAM].getValue()))
		{
			for (int i = 0; i < 8; ++i)
			{
				samplers[i].clear();
			}
		}

		// SEQUENCER PROCESSING
		if (sequencer.running)
		{
			// A little easier on the eyes to use this
			int *index = &sequencer.index;

			// Sequencer advanced by clock trigger
			// Really should encapsulate this so we're not repeating ourselves bellow
			if (clockTrigger.process(inputs[CLOCK_INPUT].getVoltage()))
			{
				sequencer.advanceIndex();

				// If the current step is skipped, just keep advancing until we find one that isn't
				// Should this be a do while?
				while (skips[*index])
				{
					sequencer.advanceIndex();
				}

				// If we're recording a new sample, clear out anything from the buffer
				if (recording)
				{
					samplers[*index].prepareRecording();
				}
				else
				{
					// Still need to reset index to begin playback
					// But here we also perform any necessary sample rate conversion
					samplers[*index].preparePlayback(reverses[*index], pitch);
				}

				// Reset the gain envelope
				ramp.out = 0.f;
				ramp.gate = true;
			}

			// Sequencer advanced by jumping to a specific step
			if (jumpTo > -1)
			{
				sequencer.setIndex(jumpTo);

				// We always play a triggered step regardless of whether it's skipped or not

				// If we're recording a new sample, clear out anything from the buffer
				if (recording)
				{
					samplers[jumpTo].prepareRecording();
				}
				else
				{
					// Still need to reset index to begin playback
					// But here we also perform an necessary sample rate conversion
					samplers[jumpTo].preparePlayback(reverses[jumpTo], pitch);
				}

				// Reset the gain envelope
				ramp.out = 0.f;
				ramp.gate = true;
			}

			//SAMPLER PROCESSING
			if (recording)
			{
				// Get the input
				float in = inputs[IN_INPUT].getVoltage();

				// Send it to the sampler, along with the sample rate it's being recorded at
				samplers[*index].record(in, args.sampleRate);

				// Output whatever we're recording as we're recording it
				outs[*index] = in;
				mainOut = in;
			}
			else
			{
				// Playback
				samplers[*index].play(reverses[*index]);

				// Process the ramp even when it's not being used because params might be changed over the playback of a long recording
				float envelope = 1.f;
				ramp.process(0.3, attack, release, args.sampleTime, false);

				// If the knobs are at default we don't use the envelope
				if (attack > 0.f || release < 1.f)
				{
					envelope = ramp.out;
				}

				// Multiply the output from the sampler by the relevant gains and send it to the outs
				outs[*index] = samplers[*index].output * gains[*index] * envelope;
				mainOut = samplers[*index].output * gains[*index] * mutes[*index] * envelope;
			}
		}

		// Set all the individual step outs
		for (int i = 0; i < 8; ++i)
		{
			outputs[OUTS_OUTPUT + i].setVoltage(outs[i]);
		}

		// Set the master out
		outputs[MAINOUT_OUTPUT].setVoltage(mainOut);

		// If we're recording, turn the record light on
		lights[REC_LIGHT].setBrightness(recording);

		// Display the LED for the current step
		displayLED();
	}
};

struct NovaWidget : ModuleWidget
{
	NovaWidget(Nova *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Nova.svg")));

		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<FFDPW>(mm2px(Vec(28.976, 59.34)), module, Nova::START_PARAM));
		addParam(createParamCentered<FFDPW>(mm2px(Vec(28.976, 77.302)), module, Nova::RESET_PARAM));
		addParam(createParamCentered<FFDPW>(mm2px(Vec(28.976, 95.263)), module, Nova::DIRECTION_PARAM));
		addParam(createParamCentered<FFDPW>(mm2px(Vec(28.976, 113.225)), module, Nova::RECORD_PARAM));

		addParam(createParamCentered<FF15GKnob>(mm2px(Vec(190.051, 39.694)), module, Nova::ATTACK_PARAM));
		addParam(createParamCentered<FF15GKnob>(mm2px(Vec(190.051, 65.493)), module, Nova::RELEASE_PARAM));
		addParam(createParamCentered<FF15GKnob>(mm2px(Vec(190.051, 91.272)), module, Nova::PITCH_PARAM));

		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(22.88, 23.417)), module, Nova::IN_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(22.88, 41.379)), module, Nova::CLOCK_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(16.335, 59.34)), module, Nova::START_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(16.335, 77.302)), module, Nova::RESET_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(16.335, 95.263)), module, Nova::DIRECTION_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(16.335, 113.225)), module, Nova::RECORD_INPUT));

		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(190.051, 110.766)), module, Nova::MAINOUT_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(33, 105.200)), module, Nova::REC_LIGHT));

		for (int i = 0; i < 8; ++i)
		{
			float deltaX = 15.05f;
			addParam(createParamCentered<FF10GKnob>(mm2px(Vec(61.531 + (i * deltaX), 23.43)), module, Nova::GAINS_PARAM + i));
			addParam(createParamCentered<FFDPTW>(mm2px(Vec(61.531 + (i * deltaX), 38.834)), module, Nova::MUTES_PARAM + i));
			addParam(createParamCentered<FFDPTW>(mm2px(Vec(61.531 + (i * deltaX), 54.238)), module, Nova::SKIPS_PARAM + i));
			addParam(createParamCentered<FFDPTW>(mm2px(Vec(61.531 + (i * deltaX), 69.642)), module, Nova::REVERSES_PARAM + i));
			addParam(createParamCentered<FFDPW>(mm2px(Vec(61.531 + (i * deltaX), 85.046)), module, Nova::TRIGGERS_PARAM + i));
			addInput(createInputCentered<FF01JKPort>(mm2px(Vec(61.531 + (i * deltaX), 97.487)), module, Nova::TRIGGERS_INPUT + i));
			addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(61.531 + (i * deltaX), 110.766)), module, Nova::OUTS_OUTPUT + i));
			addChild(createLightCentered<MediumLight<RedGreenBlueLight>>(mm2px(Vec(61.531 + (i * deltaX), 117.503)), module, Nova::SEQS_LIGHT + (i * 3)));
		}
	}
};

Model *modelNova = createModel<Nova, NovaWidget>("Nova");