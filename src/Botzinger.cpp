// Arbitrary Rhythm Generator
// Ross Cameron 2021/09/25
// Title font - Moche Regular
// https://www.pepite.world/fonderie/moche/
// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"
#include "ffCommon.hpp"

struct Botzinger : Module {
	enum ParamIds {
		ENUMS(TIME_PARAM, 8),
		ENUMS(REPEAT_PARAM, 8),
		ENUMS(WIDTH_PARAM, 8),
		RATE_PARAM,
		START_PARAM,
		DIRECTION_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(TIME_INPUT, 8),
		ENUMS(REPEAT_INPUT, 8),
		ENUMS(WIDTH_INPUT, 8),
		CLOCK_INPUT,
		RESET_INPUT,
		START_INPUT,
		DIRECTION_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(OUTS_OUTPUT, 8),
		MAIN_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(STEP_LIGHT, 8),
		NUM_LIGHTS
	};

	dsp::PulseGenerator pulse;
	dsp::Timer time;

	dsp::SchmittTrigger resetTrigger;
	dsp::SchmittTrigger startTrigger;
	dsp::SchmittTrigger directionTrigger;

	Sequencer sequencer;


	float multiplier = 0.f;
	float stepLength = 0.f;
	float onLength = 0.f;

	Botzinger() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		for (int i = 0; i < 8; ++i)
		{
			configParam(TIME_PARAM + i, 0.f, 1.f, 0.f, "Step Time", "%", 0.f, 100.f);
			configParam(REPEAT_PARAM + i, 0.f, 1.f, 0.f, "");
			configParam(WIDTH_PARAM + i, 0.f, 1.f, 0.5f, "Gate Width", "%", 0.f, 100.f);
		}
				
		configParam(RATE_PARAM, -2.f, 4.f, 1.f, "Global Rate Multiplier", " Seconds", 10.f);
		configParam(START_PARAM, 0.f, 1.f, 0.f, "Start/Stop");
		configParam(DIRECTION_PARAM, 0.f, 1.f, 0.f, "Sequencer Direction");

		sequencer.running = true;
	}

	void getParameters()
	{
		// Expect the multiplier param to return a value 0<x<1
		multiplier = params[RATE_PARAM].getValue();
		// Convert to a decade scale - 10^x seconds
		multiplier = pow(10.f, multiplier);
		
		// stepLength is a percentage of the global rate multiplier
		stepLength = params[TIME_PARAM + sequencer.index].getValue() * multiplier;
		// onLength is a percentage of stepLength
		onLength = params[WIDTH_PARAM + sequencer.index].getValue() * stepLength;
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

		// Restart both timers
		time.reset();
		pulse.reset();

		// Start a new pulse timer
		// The length of which is determined by onLength
		pulse.trigger(onLength);

		// Set the new sequencer index light to on
		lights[STEP_LIGHT + sequencer.index].setBrightness(10.f);
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
			time.reset();
			pulse.reset();

			// Clear the output port of the current step of any voltage
			outputs[OUTS_OUTPUT + sequencer.index].setVoltage(0.f);

			sequencer.reset();
		}
	}

	void process(const ProcessArgs& args) override {
		
		checkTriggers();

		getParameters();

		// We only advance the step timer if the sequencer is running
		if (sequencer.running && time.process(args.sampleTime) > stepLength)
		{
			nextStep();
		}

		float out = 10.f * float(pulse.process(args.sampleTime));

		outputs[OUTS_OUTPUT + sequencer.index].setVoltage(out);
		outputs[MAIN_OUTPUT].setVoltage(out);

	}
};


struct BotzingerWidget : ModuleWidget {
	BotzingerWidget(Botzinger* module) {
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

			addParam(createParamCentered<FF08GKnob>(mm2px(Vec(31.462 + (i * deltaX), 89.104)), module, Botzinger::REPEAT_PARAM + i));
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


Model* modelBotzinger = createModel<Botzinger, BotzingerWidget>("Botzinger");