// Cut-up Sequenced Samp
// Ross Cameron 2021/01/18
// Title font - Citytype Miami
// https://www.citype.net/city/miami
// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"
#include "ffCommon.hpp"
#include <samplerate.h>

struct Sequencer
{
	bool running = false;
	// 0 = fwd, 1 = rev, 2 = bounce, 3 = rnd
	int direction = 0;
	// Just used in bounce mode
	int bounceDir = 0;

	int index = 0; // Sequencer index

	void reset()
	{
		index = 0;
	}

	void setIndex(int step)
	{
		if (step < 8 && step > -1)
		{
			index = step;
		}
	}

	void startStop()
	{
		running = !running;
	}

	void directionChange()
	{
		// Cycle through direction modes
		++direction;
		direction %= 4;
	}

	void advanceIndex()
	{
		switch (direction)
		{
		case 0:
			// Forward
			++index;
			index %= 8;
			break;

		case 1:
			// Reverse
			--index;
			index = (index % 8 + 8) % 8;
			break;

		case 2:
			// Bouncing (plays each end twice so as to be a factor of four)
			if (bounceDir)
			{
				++index;
				if (index == 8)
				{
					bounceDir = !bounceDir;
					index = 7;
				}
			}
			else
			{
				--index;
				if (index == -1)
				{
					bounceDir = !bounceDir;
					index = 0;
				}
			}

			break;

		case 3:
			// Random
			{
				float rng = 7 * random::uniform();
				index = (int)round(rng);
			}
			break;

		default:
			break;
		}
	}
};

struct Sampler
{
	// Buffer that holds the original recorded sample
	std::vector<float> inBuffer;
	// Buffer that is sent to output, may be sample rate converted or just a copy of the input buffer
	std::vector<float> outBuffer;

	dsp::SampleRateConverter<1> inputSrc;
	float originalSamplerate = 441000.f;

	// Iterator used to index through the output buffer
	int playhead = 0;
	float output = 0.f;
	bool finished = true;

	float play(bool direction)
	{
		if (outBuffer.empty() || finished)
		{
			// Reached the end of the buffer or there isn't anything there in the first place
			output = 0.f;
			return output;
		}

		if (!direction)
		{
			// Forward playback
			if (playhead > (int)outBuffer.size() - 1)
			{
				// Reached the end of the buffer
				finished = true;
				output = 0.f;
				return output;
			}

			// Still within the buffer, send current sample in buffer to output
			output = outBuffer[playhead];
			// Increment index of the buffer
			++playhead;
			return output;
		}
		else
		{
			// Reverse playback
			if (playhead < 0)
			{
				// Reached the start of the buffer (end of reverse playback)
				finished = true;
				output = 0.f;
				return output;
			}

			// Still within the buffer, send current sample in buffer to output
			output = outBuffer[playhead];
			// Decrement index of buffer
			--playhead;
			return output;
		}
	}

	// Called every time the sequencer begins playback from a sampler
	void reset(bool direction, float pitch, float samplerate)
	{
		// Reset playback parameters
		finished = false;
		if (!direction)
		{
			// Forward - starting from the start of the buffer
			playhead = 0;
		}
		else
		{
			// Reverse - starting from the end of the buffer
			playhead = outBuffer.empty() ? 0 : outBuffer.size() - 1; // Is this necessary? if there's nothing in the sample vector then playhead shouldn't get used. probably best to be safe?
		}

		// Check for sample rate conversion
		if (pitch == 0.f)
		{
			// No conversion, output buffer is same as original sample
			outBuffer = inBuffer;
		}
		else if (!inBuffer.empty()) // Only convert if there's something there to convert
		{
			// Expecting the input paramter to be between -1.f and 1.f
			// Produces a pitch/speed change from 1/4 to 4 times
			float ratio = pow(4, -pitch);

			// New sample rate we're converting to
			float newRate = originalSamplerate * ratio;

			// Prepare sample rate convertor
			inputSrc.setRates(originalSamplerate, newRate);
			int inLen = inBuffer.size();
			int outLen = inLen * ratio;

			// Make sure we've got enough space reserved to fill the output buffer with interpolated samples
			outBuffer.reserve(outLen);
			// Process the sample rate conversion
			inputSrc.process((dsp::Frame<1> *)&inBuffer[0], &inLen, (dsp::Frame<1> *)&outBuffer[0], &outLen);
			// Trim off any excess from the output buffer
			outBuffer.resize(outLen);
			outBuffer.shrink_to_fit();
		}
	}

	void erase()
	{
		// Reset the buffers back to empty and free any memory we're not using
		inBuffer.clear();
		inBuffer.shrink_to_fit();
		outBuffer.clear();
		outBuffer.shrink_to_fit();
	}

	void record(float in, float samplerate)
	{
		inBuffer.push_back(in);
		originalSamplerate = samplerate;
	}

	float playheadPercent()
	{
		// Returns playheads current position of sample playback as a decimal percentage
		// Used to determine LED colour during sample playback
		float position = (float)playhead;
		float total = (float)inBuffer.size();

		float percent = (position + 1.f) / total;

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
		configParam(START_PARAM, 0.f, 1.f, 0.f, "Sequencer Start");
		configParam(RESET_PARAM, 0.f, 1.f, 0.f, "Sequencer Reset");
		configParam(DIRECTION_PARAM, 0.f, 1.f, 0.f, "Sequencer Direction");
		configParam(RECORD_PARAM, 0.f, 1.f, 0.f, "Sampler Record Start");

		for (int i = 0; i < 8; ++i)
		{
			configParam(GAINS_PARAM + i, 0.f, 1.f, 1.f, string::f("Sample %d Gain", i + 1), "dB", -10.f, 20.f);
			configParam(MUTES_PARAM + i, 0.f, 1.f, 0.f, string::f("Sample %d Mute", i + 1));
			configParam(SKIPS_PARAM + i, 0.f, 1.f, 0.f, string::f("Sample %d Skip", i + 1));
			configParam(REVERSES_PARAM + i, 0.f, 1.f, 0.f, string::f("Sample %d Reverse", i + 1));
			configParam(TRIGGERS_PARAM + i, 0.f, 1.f, 0.f, string::f("Sample %d Trigger", i + 1));
		}

		configParam(ATTACK_PARAM, 0.f, 1.f, 0.f, "Global Sample Attack");
		configParam(RELEASE_PARAM, 0.f, 1.f, 1.f, "Global Sample Release");
		configParam(PITCH_PARAM, -1.f, 1.f, 0.f, "Global Sample Pitch", "x", 4.f);
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
		else if (samplers[*index].inBuffer.empty())
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
				samplers[i].erase();
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
					samplers[*index].erase();
				}

				// Reset the sampler playhead for this step so we can begin recording or playback
				// This is also where we do any necessary sample rate conversion
				samplers[*index].reset(reverses[*index], pitch, args.sampleTime);

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
					samplers[jumpTo].erase();
				}

				// Reset the sampler playhead for this step so we can begin recording or playback
				// This is also where we do any necessary sample rate conversion
				samplers[jumpTo].reset(reverses[jumpTo], pitch, args.sampleRate);

				// Reset the gain envelope
				ramp.out = 0.f;
				ramp.gate = true;
			}

			//SAMPLER PROCESSING
			if (recording)
			{
				// Get the input
				float in = inputs[IN_INPUT].getVoltage();

				// Send it to the sampler, along with the sample rate it's recorded at
				samplers[*index].record(in, args.sampleRate);

				// Output whatever we're recording as we're recording it
				outs[*index] = in;
				mainOut = in;
			}
			else
			{
				// Playback
				samplers[*index].play(reverses[*index]);

				// Process the ramp even when it's not being used because params might be changed over the playback of a big inBuffer
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