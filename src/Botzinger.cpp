// Arbitrary Rhythm Generator
// Ross Cameron 2021/09/25
// Title font - Moche Regular
// https://www.pepite.world/fonderie/moche/
// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"
#include "ffCommon.hpp"

struct Botzinger : Module
{
	enum ParamIds
	{
		ENUMS(TIME_PARAM, 8),
		ENUMS(REPEAT_PARAM, 8),
		ENUMS(WIDTH_PARAM, 8),
		RATE_PARAM,
		START_PARAM,
		DIRECTION_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		ENUMS(TIME_INPUT, 8),
		ENUMS(REPEAT_INPUT, 8),
		ENUMS(WIDTH_INPUT, 8),
		CLOCK_INPUT,
		RESET_INPUT,
		START_INPUT,
		DIRECTION_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		ENUMS(OUTS_OUTPUT, 8),
		MAIN_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		ENUMS(STEP_LIGHT, 8),
		NUM_LIGHTS
	};
	// All the triggers that may be input
	dsp::SchmittTrigger resetTrigger;
	dsp::SchmittTrigger startTrigger;
	dsp::SchmittTrigger directionTrigger;
	dsp::SchmittTrigger clockTrigger;

	// Timers for the free running pulse generator
	dsp::PulseGenerator pulse;
	dsp::Timer stepTime;
	dsp::Timer beatTime;

	// Counters for the clocked pulse generator
	int stepClockCount = 0;
	int beatClockCount = 0;

	bool clocked = false;
	Sequencer sequencer;

	// How long each step/beat/on pulse is, as a fraction of the global rate
	float globalRate = 0.f;
	float stepLength = 0.f;
	float beatLength = 0.f;
	float pulseWidth = 0.f;

	int repeats = 1;

	Botzinger()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		for (int i = 0; i < 8; ++i)
		{
			int j = i + 1;

			configParam(TIME_PARAM + i, 0.f, 1.f, .4f, string::f("Step %d Time", j), "%", 0.f, 100.f);
			configParam(REPEAT_PARAM + i, 1.f, 32.f, 1.f, string::f("Step %d Repeats", j));
			configParam(WIDTH_PARAM + i, 0.f, 1.f, 0.25, string::f("Step %d Width", j), "%", 0.f, 100.f);

			configInput(TIME_INPUT + i, string::f("Step %d Time CV", j));
			configInput(REPEAT_INPUT + i, string::f("Step %d Repeats CV", j));
			configInput(WIDTH_INPUT + i, string::f("Step %d Width CV", j));

			configOutput(OUTS_OUTPUT, string::f("Step %d", j));

			configLight(STEP_LIGHT, string::f("Step %d", j));
		}

		configParam(RATE_PARAM, -2.f, 4.f, 0.f, "Global Rate Multiplier", "", 10.f);
		configParam(START_PARAM, 0.f, 1.f, 0.f, "Start/Stop");
		configParam(DIRECTION_PARAM, 0.f, 1.f, 0.f, "Sequencer Direction");

		configInput(CLOCK_INPUT, "Clock Trig");
		configInput(RESET_INPUT, "Reset Trig");
		configInput(START_INPUT, "Start Trig");
		configInput(DIRECTION_INPUT, "Direction Trig");

		configOutput(MAIN_OUTPUT, "Main");

		sequencer.running = true;
	}

	void resetTimers()
	{
		stepTime.reset();
		beatTime.reset();
		pulse.reset();
	}

	void getFreeRunParameters()
	{
		// Expect the globalRate param to return a value -2<x<4
		globalRate = params[RATE_PARAM].getValue();
		// Convert to a decade scale - 10^x seconds
		globalRate = pow(10.f, globalRate);

		// stepLength is a percentage of the global rate
		stepLength = params[TIME_PARAM + sequencer.index].getValue();
		if (inputs[TIME_INPUT + sequencer.index].isConnected())
		{
			stepLength += inputs[TIME_INPUT + sequencer.index].getVoltage() * 0.1f;
			stepLength = clamp(stepLength, 0.f, 1.f);
		}
		stepLength *= globalRate;

		// beatLength is essentially how long a beat is if it is repeated n times in a step
		repeats = params[REPEAT_PARAM + sequencer.index].getValue();
		beatLength = stepLength / repeats;

		// pulseWidth is a percentage of beatLength, basically pulsewidth
		pulseWidth = params[WIDTH_PARAM + sequencer.index].getValue();
		if (inputs[WIDTH_INPUT + sequencer.index].isConnected())
		{
			pulseWidth += inputs[WIDTH_INPUT + sequencer.index].getVoltage() * 0.1f;
			pulseWidth = clamp(pulseWidth, 0.f, 1.f);
		}
		pulseWidth *= beatLength;
	}

	void nextStep()
	{
		// Remove any voltage from the current output
		// This seems to be necessary to clear the individual gate outputs
		outputs[OUTS_OUTPUT + sequencer.index].setVoltage(0.f);
		// Set the current sequencer index light to off
		lights[STEP_LIGHT + sequencer.index].setBrightness(0.f);

		// Move the internal sequener to the next step
		sequencer.advanceIndex();

		if (clocked)
		{
			beatClockCount = 0;
			stepClockCount = 0;
		}
		else
		{
			// Restart all timers
			resetTimers();

			// Start a new pulse timer
			// The length of which is determined by pulseWidth
			pulse.trigger(pulseWidth);
		}

		// Set the new sequencer index light to on
		lights[STEP_LIGHT + sequencer.index].setBrightness(10.f);
	}

	void nextBeat()
	{
		// Remove any voltage from the current output
		// This seems to be necessary to clear the individual gate outputs
		outputs[OUTS_OUTPUT + sequencer.index].setVoltage(0.f);

		if (clocked)
		{
			beatClockCount = 0;
		}
		else
		{
			// Restart beat and on timers
			beatTime.reset();
			pulse.reset();

			// Start a new pulse timer
			// The length of which is determined by pulseWidth
			pulse.trigger(pulseWidth);
		}
	}

	void checkTriggers()
	{
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

		// Check for resets
		if (resetTrigger.process(inputs[RESET_INPUT].getVoltage()))
		{
			// Clear both timers
			resetTimers();

			stepClockCount = 0;
			beatClockCount = 0;

			// Clear the output port of the current step of any voltage
			outputs[OUTS_OUTPUT + sequencer.index].setVoltage(0.f);
			// Set the current sequencer index light to off
			lights[STEP_LIGHT + sequencer.index].setBrightness(0.f);

			sequencer.reset();
		}
	}

	void getClockMode()
	{
		bool previousState = clocked;

		clocked = inputs[CLOCK_INPUT].isConnected();

		if (clocked != previousState)
		{
			// We've just changed clock modes
			// Reset stuff
			resetTimers();
			stepClockCount = 0;
			beatClockCount = 0;
		}
	}

	void getClockedParameters()
	{
		// Expect the globalRate param to return a value -2<x<4
		globalRate = params[RATE_PARAM].getValue();
		// Convert to a decade scale - 10^x seconds
		globalRate = pow(10.f, globalRate);

		// stepLength
		stepLength = params[TIME_PARAM + sequencer.index].getValue();
		if (inputs[TIME_INPUT + sequencer.index].isConnected())
		{
			stepLength += inputs[TIME_INPUT + sequencer.index].getVoltage() * 0.1f;
			stepLength = clamp(stepLength, 0.f, 1.f);
		}
		stepLength *= globalRate;

		// beatLength is essentially how long a beat is if it is repeated n times in a step
		repeats = params[REPEAT_PARAM + sequencer.index].getValue();
		beatLength = stepLength / repeats;

		// pulseWidth is a percentage of beatLength, basically pulsewidth
		pulseWidth = params[WIDTH_PARAM + sequencer.index].getValue();
		if (inputs[WIDTH_INPUT + sequencer.index].isConnected())
		{
			pulseWidth += inputs[WIDTH_INPUT + sequencer.index].getVoltage() * 0.1f;
			pulseWidth = clamp(pulseWidth, 0.f, 1.f);
		}
		pulseWidth *= beatLength;
	}

	void process(const ProcessArgs &args) override
	{
		float out = 0.f;

		getClockMode();

		checkTriggers();

		if (sequencer.running)
		{
			if (clocked)
			{
				getClockedParameters();

				// Run sequencer clocked
				// // Round off parameter values to a round number?
				// stepLength = round(stepLength);
				// beatLength = round(beatLength);
				// pulseWidth = round(pulseWidth);

				// Count how many clock pulses have arrived since we started this step
				if (clockTrigger.process(inputs[CLOCK_INPUT].getVoltage()))
				{
					++stepClockCount;
					++beatClockCount;
				}

				if (beatClockCount <= pulseWidth)
				{
					out = 10.f;
				}
				if (beatClockCount > beatLength)
				{
					nextBeat();
				}
				if (stepClockCount > stepLength)
				{
					nextStep();
				}
			}
			else
			{
				getFreeRunParameters();

				if (beatTime.process(args.sampleTime) > beatLength)
				{
					nextBeat();
				}
				if (stepTime.process(args.sampleTime) > stepLength)
				{
					nextStep();
				}

				out = 10.f * float(pulse.process(args.sampleTime));
			}
		}

		outputs[OUTS_OUTPUT + sequencer.index].setVoltage(out);
		outputs[MAIN_OUTPUT].setVoltage(out);
	}
};

struct BotzingerWidget : ModuleWidget
{
	BotzingerWidget(Botzinger *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Botzinger.svg")));

		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		for (int i = 0; i < 8; ++i)
		{
			float deltaX = 15.0;

			addParam(createLightParamCentered<LEDLightSlider<RedLight>>(mm2px(Vec(31.462 + (i * deltaX), 50.814)), module, Botzinger::TIME_PARAM + i, Botzinger::STEP_LIGHT + i));

			addParam(createParamCentered<FF08GSnapKnob>(mm2px(Vec(31.462 + (i * deltaX), 89.104)), module, Botzinger::REPEAT_PARAM + i));
			addParam(createParamCentered<FF08GKnob>(mm2px(Vec(31.462 + (i * deltaX), 100.669)), module, Botzinger::WIDTH_PARAM + i));

			addInput(createInputCentered<FF01JKPort>(mm2px(Vec(31.462 + (i * deltaX), 24.189)), module, Botzinger::TIME_INPUT + i));
			addInput(createInputCentered<FF01JKPort>(mm2px(Vec(31.462 + (i * deltaX), 76.492)), module, Botzinger::REPEAT_INPUT + i));

			addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(31.462 + (i * deltaX), 113.225)), module, Botzinger::OUTS_OUTPUT + i));
		}

		addParam(createParamCentered<FF15GSnapKnob>(mm2px(Vec(161.638, 50.45)), module, Botzinger::RATE_PARAM));
		addParam(createParamCentered<FFDPW>(mm2px(Vec(167.958, 76.492)), module, Botzinger::START_PARAM));
		addParam(createParamCentered<FFDPW>(mm2px(Vec(167.958, 97.487)), module, Botzinger::DIRECTION_PARAM));

		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(155.317, 24.189)), module, Botzinger::CLOCK_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(167.958, 24.189)), module, Botzinger::RESET_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(155.317, 76.492)), module, Botzinger::START_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(155.317, 97.487)), module, Botzinger::DIRECTION_INPUT));

		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(155.317, 113.225)), module, Botzinger::MAIN_OUTPUT));
	}
};

Model *modelBotzinger = createModel<Botzinger, BotzingerWidget>("Botzinger");