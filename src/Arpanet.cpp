// 1601 Style Step Sequencer
// Ross Cameron 2020/06/04

// Title Font - ZXX Camo
// https://www.librarystack.org/zxx/
// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"

struct Arpanet : Module
{
	enum ParamIds
	{
		ENUMS(GATE1_PARAM, 16),
		ENUMS(SLIDER1_PARAM, 16),
		STARTTOGGLE_PARAM,
		SKIP_PARAM,
		SKIPTOGGLE_PARAM,
		STARTSTOP_PARAM,
		CLOCK_PARAM,
		FM_PARAM,
		PULSE_PARAM,
		RESET_PARAM,
		LENGTH_PARAM,
		RANDOM_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		QUANTCV_INPUT,
		SKIP_INPUT,
		START_INPUT,
		QUANTA_INPUT,
		QUANTB_INPUT,
		RESET_INPUT,
		STOP_INPUT,
		STARTSTOP_INPUT,
		FM_INPUT,
		PULSE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		GATEBUS1_OUTPUT,
		GATEBUS2_OUTPUT,
		GATEBUS3_OUTPUT,
		POSITION1_OUTPUT,
		CLOCKEDGATE1_OUTPUT,
		QUANTA_OUTPUT,
		QUANTB_OUTPUT,
		CLOCK_OUTPUT,
		SEQA_OUTPUT,
		SEQB_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		ENUMS(POS1_LIGHT, 16),
		CLOCK_LIGHT,
		NUM_LIGHTS
	};

	Arpanet()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		for (int i = 0; i < 16; ++i)
		{
			configSwitch(GATE1_PARAM + i, 0.f, 2.f, 1.f, string::f("Step %d Gate Assign", i + 1), {"Bus 3", "Bus 2", "Bus 1"});
			configParam(SLIDER1_PARAM + i, 0.f, 12.f, 6.f, string::f("Step %d Voltage", i + 1), "V");
			configLight(POS1_LIGHT, string::f("Step %d", i + 1));
		}

		configSwitch(STARTTOGGLE_PARAM, 0.f, 1.f, 0.f, "Start CV Mode", {"Trigger", "Gate"});
		configButton(SKIP_PARAM, "Skip Sequencer Step");
		configSwitch(SKIPTOGGLE_PARAM, 0.f, 2.f, 1.f, "Gate Bus 3 Assign", {"Reset", "", "Skip Step"});
		configButton(STARTSTOP_PARAM, "Sequencer Start/Stop");
		configParam(CLOCK_PARAM, -2.f, 6.f, 2.f, "Clock Rate", "BPM", 2.f, 60.f);
		configParam(FM_PARAM, 0.f, 1.f, 0.f, "Clock FM Amount");
		configParam(PULSE_PARAM, 0.05f, 1.f, 0.05f, "Clock Pulse-Width");
		configButton(RESET_PARAM, "Sequencer Reset");
		configSwitch(LENGTH_PARAM, 0.f, 1.f, 0.f, "Sequence Length", {"16 Steps", "8 Steps"});
		configSwitch(RANDOM_PARAM, 0.f, 1.f, 0.f, "Direction Mode", {"Sequential", "Random"});

		configInput(QUANTCV_INPUT, "Quantizer CV Offset");
		configInput(SKIP_INPUT, "Skip Step Trig");
		configInput(START_INPUT, "Start Trig");
		configInput(QUANTA_INPUT, "Quantizer A");
		configInput(QUANTB_INPUT, "Quantizer B");
		configInput(RESET_INPUT, "Reset Trig");
		configInput(STOP_INPUT, "Stop Trig");
		configInput(STARTSTOP_INPUT, "Start/Stop Trig");
		configInput(FM_INPUT, "Clock FM CV");
		configInput(PULSE_INPUT, "Clock Pulse Width CV");

		configOutput(GATEBUS1_OUTPUT, "Gate Bus 1");
		configOutput(GATEBUS2_OUTPUT, "Gate Bus 2");
		configOutput(GATEBUS3_OUTPUT, "Gate Bus 3");
		configOutput(POSITION1_OUTPUT, "Position 1");
		configOutput(CLOCKEDGATE1_OUTPUT, "Clocked Gate Bus 1");
		configOutput(QUANTA_OUTPUT, "Quantizer A");
		configOutput(QUANTB_OUTPUT, "Quantizer B");
		configOutput(CLOCK_OUTPUT, "Clock");
		configOutput(SEQA_OUTPUT, "Sequencer A");
		configOutput(SEQB_OUTPUT, "Sequencer B");
	}

	dsp::SchmittTrigger resetTrigger;
	dsp::SchmittTrigger skipTrigger;
	dsp::SchmittTrigger startTrigger;
	dsp::SchmittTrigger stopTrigger;
	dsp::SchmittTrigger runningTrigger;

	bool running = true;

	float phase = 0.f;
	int index = 0;
	int indexA = 0;
	int indexB = 0;

	int clock = 1;

	// Init bus assignment to Bus 2
	int currentBus = 1;
	float gates[3] = {0.f};

	bool random = false;
	bool dual = false;

	float outA = 0.f;
	float outB = 0.f;

	float getRate(float fm)
	{
		float rate = params[CLOCK_PARAM].getValue();
		rate += params[FM_PARAM].getValue() * fm;
		rate = std::pow(2.f, rate);
		return rate;
	}

	float getWidth()
	{
		float width = params[PULSE_PARAM].getValue() * (0.1f * inputs[PULSE_INPUT].getNormalVoltage(10.f));
		return width;
	}

	void setGateBusses()
	{
		// Set all to zero volts
		for (int i = 0; i < 3; ++i)
		{
			gates[i] = 0.f;
		}

		// Set currently assigned bus to 10v
		gates[currentBus] = 10.f;

		// Output busses
		for (int i = 0; i < 3; ++i)
		{
			outputs[GATEBUS1_OUTPUT + i].setVoltage(gates[i]);
		}

		// Output clocked gate 1
		outputs[CLOCKEDGATE1_OUTPUT].setVoltage(gates[0] * clock);
	}

	void advanceIndex()
	{
		// Random mode
		if (random)
		{
			float rng = 15 * random::uniform();
			index = (int)round(rng);
		}
		// Sequenatial mode
		else
		{
			// Advance sequence
			++index;

			// Reset if we've reached the max number of steps
			if (index > 15)
			{
				index = 0;
			}
		}
	}

	bool lfoPhase(float rate, float delta, float width)
	{
		// Accumulate phase
		phase += rate * delta;

		// After one cycle advance the sequencer index
		if (phase >= 1.f)
		{
			advanceIndex();
			phase = 0.f;
		}

		// Outputs a squarewave with duty cycle determined by width input
		bool lfo = phase < width;
		return lfo;
	}

	float quantise(float in)
	{
		// Scale up from 0v/12v to 0v/24v
		in *= 2.f;

		// Cast to an int to round
		int q = (int)in;

		// Attenuate and cast to float
		float out = q / 12.f;
		return out;
	}

	void skips()
	{
		// Process skips
		if (skipTrigger.process(params[SKIP_PARAM].getValue() + inputs[SKIP_INPUT].getVoltage()))
		{
			advanceIndex();
		}
		if (params[SKIPTOGGLE_PARAM].getValue() == 2 && currentBus == 2)
		{
			advanceIndex();
		}
	}

	void resets()
	{
		if (resetTrigger.process(params[RESET_PARAM].getValue() + inputs[RESET_INPUT].getVoltage()))
		{
			index = 0;
		}
		if (params[SKIPTOGGLE_PARAM].getValue() == 0 && currentBus == 2)
		{
			index = 0;
		}
	}

	void sequencerStep()
	{
		// Get sequencer voltages
		outA = params[SLIDER1_PARAM + indexA].getValue();
		outB = params[SLIDER1_PARAM + indexB].getValue();

		// Set position lights
		for (int i = 0; i < 16; ++i)
		{
			lights[POS1_LIGHT + i].setBrightness(0);
		}
		lights[POS1_LIGHT + indexA].setBrightness(1);
		lights[POS1_LIGHT + indexB].setBrightness(1);
	}

	void startControls()
	{
		// Get the start/stop mode
		// false = trig, true = gate,
		bool startMode = (bool)params[STARTTOGGLE_PARAM].getValue();

		if (startMode)
		{
			// Gate (momentary) start
			// Only starts if there's nothing present on the stop input
			if (inputs[STOP_INPUT].getVoltage())
			{
				running = false;
			}
			else if (params[STARTSTOP_PARAM].getValue() || inputs[START_INPUT].getVoltage() || inputs[STARTSTOP_INPUT].getVoltage())
			{
				running = true;
			}
			else
			{
				running = false;
			}
		}
		else
		{
			// Trig (toggle) start
			if (runningTrigger.process(params[STARTSTOP_PARAM].getValue() + inputs[STARTSTOP_INPUT].getVoltage()))
			{
				running = !running;
			}
			if (startTrigger.process(inputs[START_INPUT].getVoltage()))
			{
				running = true;
			}
			if (stopTrigger.process(inputs[STOP_INPUT].getVoltage()))
			{
				running = false;
			}
		}
	}

	void process(const ProcessArgs &args) override
	{
		// Check if in random or sequential mode
		random = (bool)params[RANDOM_PARAM].getValue();

		// Get the length mode
		dual = (bool)params[LENGTH_PARAM].getValue();

		startControls();

		if (running)
		{
			// Get clock rate
			// FM input is normaled to Gate Bus 1
			float clockRate = getRate(inputs[FM_INPUT].getNormalVoltage(gates[0] / 2.f));

			// Get pulse width
			float pulseWidth = getWidth();

			// Accumulate LFO to advance clock and index
			clock = (int)lfoPhase(clockRate, args.sampleTime, pulseWidth);

			// Output clock
			outputs[CLOCK_OUTPUT].setVoltage(clock * 10);
			lights[CLOCK_LIGHT].setSmoothBrightness(clock, args.sampleTime);
		}
		else
		{
			// Choke clock output when not running
			outputs[CLOCK_OUTPUT].setVoltage(0);
			lights[CLOCK_LIGHT].setSmoothBrightness(0, args.sampleTime);
		}

		if (dual)
		{
			// 8 step dual mode
			// Set dual indexes
			indexA = index % 8;
			indexB = indexA + 8;
		}
		else
		{
			// 16 step mode
			indexA = index;
			indexB = index;
		}
		// Get gate bus assignment for this step
		currentBus = 2 - (int)params[GATE1_PARAM + indexA].getValue();

		// Output gate busses
		setGateBusses();

		// // Process skips
		skips();

		// // Process resets
		resets();

		sequencerStep();

		// Output pos 1 gate
		float pos1Out = (indexA == 0) ? 10.f : 0.f;
		outputs[POSITION1_OUTPUT].setVoltage(pos1Out);

		// Output sequencer voltages
		outputs[SEQA_OUTPUT].setVoltage(outA);
		outputs[SEQB_OUTPUT].setVoltage(outB);

		// Quantise
		float quantA = quantise(inputs[QUANTA_INPUT].getNormalVoltage(outA));
		quantA += inputs[QUANTCV_INPUT].getVoltage();
		outputs[QUANTA_OUTPUT].setVoltage(quantA);

		float quantB = quantise(inputs[QUANTB_INPUT].getNormalVoltage(outB));
		quantB += inputs[QUANTCV_INPUT].getVoltage();
		outputs[QUANTB_OUTPUT].setVoltage(quantB);
	}
};

struct ArpanetWidget : ModuleWidget
{
	ArpanetWidget(Arpanet *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Arpanet.svg")));

		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		for (int i = 0; i < 8; ++i)
		{
			addParam(createParamCentered<CKSSThree>(mm2px(Vec(9.465 + (i * 10), 41.019)), module, Arpanet::GATE1_PARAM + i));
			addParam(createParamCentered<BefacoSlidePot>(mm2px(Vec(9.465 + (i * 10), 81.99)), module, Arpanet::SLIDER1_PARAM + i));
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(9.465 + (i * 10), 110.334)), module, Arpanet::POS1_LIGHT + i));
		}

		// Need a second loop to account for the 8.798mm gap between the A and B groups of sliders/lights/switches
		for (int i = 8; i < 16; ++i)
		{
			addParam(createParamCentered<CKSSThree>(mm2px(Vec(18.263 + (i * 10), 41.019)), module, Arpanet::GATE1_PARAM + i));
			addParam(createParamCentered<BefacoSlidePot>(mm2px(Vec(18.263 + (i * 10), 81.99)), module, Arpanet::SLIDER1_PARAM + i));
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(18.263 + (i * 10), 110.334)), module, Arpanet::POS1_LIGHT + i));
		}

		addParam(createParamCentered<HCKSS>(mm2px(Vec(219.565, 68.243)), module, Arpanet::STARTTOGGLE_PARAM));
		addParam(createParamCentered<FFDPW>(mm2px(Vec(198.312, 72.24)), module, Arpanet::SKIP_PARAM));
		addParam(createParamCentered<CKSSThree>(mm2px(Vec(185.0, 81.99)), module, Arpanet::SKIPTOGGLE_PARAM));
		addParam(createParamCentered<FFDPW>(mm2px(Vec(219.514, 81.99)), module, Arpanet::STARTSTOP_PARAM));
		addParam(createParamCentered<BefacoSlidePot>(mm2px(Vec(246.143, 82.039)), module, Arpanet::CLOCK_PARAM));
		addParam(createParamCentered<BefacoSlidePot>(mm2px(Vec(259.175, 82.039)), module, Arpanet::FM_PARAM));
		addParam(createParamCentered<BefacoSlidePot>(mm2px(Vec(272.205, 82.039)), module, Arpanet::PULSE_PARAM));
		addParam(createParamCentered<FFDPW>(mm2px(Vec(198.309, 91.74)), module, Arpanet::RESET_PARAM));
		addParam(createParamCentered<HCKSS>(mm2px(Vec(44.542, 118.093)), module, Arpanet::LENGTH_PARAM));
		addParam(createParamCentered<HCKSS>(mm2px(Vec(136.158, 118.093)), module, Arpanet::RANDOM_PARAM));

		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(310.721, 39.262)), module, Arpanet::QUANTCV_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(198.312, 52.719)), module, Arpanet::SKIP_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(219.564, 52.719)), module, Arpanet::START_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(294.668, 97.34)), module, Arpanet::QUANTA_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(326.774, 97.34)), module, Arpanet::QUANTB_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(198.312, 111.244)), module, Arpanet::RESET_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(219.564, 111.244)), module, Arpanet::STOP_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(233.223, 111.244)), module, Arpanet::STARTSTOP_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(259.175, 111.244)), module, Arpanet::FM_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(272.206, 111.244)), module, Arpanet::PULSE_INPUT));

		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(198.312, 26.462)), module, Arpanet::GATEBUS1_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(212.361, 26.462)), module, Arpanet::GATEBUS2_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(226.409, 26.462)), module, Arpanet::GATEBUS3_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(243.675, 26.462)), module, Arpanet::POSITION1_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(269.736, 26.462)), module, Arpanet::CLOCKEDGATE1_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(294.668, 26.462)), module, Arpanet::QUANTA_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(326.774, 26.462)), module, Arpanet::QUANTB_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(259.175, 50.51)), module, Arpanet::CLOCK_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(294.668, 111.244)), module, Arpanet::SEQA_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(326.774, 111.244)), module, Arpanet::SEQB_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(246.144, 50.51)), module, Arpanet::CLOCK_LIGHT));
	}
};

Model *modelArpanet = createModel<Arpanet, ArpanetWidget>("Arpanet");