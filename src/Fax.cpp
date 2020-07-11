// CV Recorder/Sequencer
// Ross Cameron 2020/07/11

// Title font - ARK-ES Dense Regular
// https://stued.io/projects/ark-typeface
// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"

// Magic numbers for the led positions
// These came from using tranform tools in illustrator
// I should write a function to generate them within the widget struct, it's simple trig

float ledPos[32][2] = {
	{40.724, 55.342}, {45.699, 56.332}, {49.917, 59.150}, {52.735, 63.367}, {53.725, 68.343}, {52.735, 73.317}, {49.917, 77.535}, {45.699, 80.353}, {40.724, 81.343}, {35.749, 80.353}, {31.531, 77.535}, {28.713, 73.317}, {27.723, 68.343}, {28.713, 63.367}, {31.531, 59.150}, {35.749, 56.332}, {40.724, 51.342}, {47.230, 52.636}, {52.745, 56.321}, {56.430, 61.837}, {57.724, 68.343}, {56.430, 74.848}, {52.745, 80.363}, {47.230, 84.049}, {40.724, 85.343}, {34.218, 84.049}, {28.703, 80.363}, {25.018, 74.848}, {23.724, 68.343}, {25.018, 61.837}, {28.703, 56.321}, {34.218, 52.636}};

struct Fax : Module
{
	enum ParamIds
	{
		NSTEPS_PARAM,
		CLOCK_PARAM,
		STEPADV_PARAM,
		RESET_PARAM,
		CV_PARAM,
		START_PARAM,
		REC_PARAM,
		STARTTOGGLE_PARAM,
		RECTOGGLE_PARAM,
		PRE_PARAM,
		AUTO_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		NSTEPS_INPUT,
		CLOCK_INPUT,
		IN_INPUT,
		STEPADV_INPUT,
		RESET_INPUT,
		START_INPUT,
		REC_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		// ENUMS(LED1_LIGHT, 32),
		ENUMS(LED1_LIGHT, 96),
		REC_LIGHT,
		NUM_LIGHTS
	};

	Fax()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(NSTEPS_PARAM, 1.f, 32.f, 16.f, "");
		configParam(CLOCK_PARAM, -2.f, 6.f, 2.f, "Clock Rate", "BPM", 2.f, 60.f);
		configParam(STEPADV_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RESET_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CV_PARAM, -5.f, 5.f, 0.f, "");
		configParam(START_PARAM, 0.f, 1.f, 0.f, "");
		configParam(REC_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STARTTOGGLE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RECTOGGLE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(PRE_PARAM, 0.f, 1.f, 1.f, "");
		configParam(AUTO_PARAM, 0.f, 1.f, 1.f, "");
	}

	dsp::SchmittTrigger stepTrigger;
	dsp::SchmittTrigger resetTrigger;
	dsp::SchmittTrigger startTrigger;
	dsp::SchmittTrigger recordTrigger;

	// Initialise stopped
	bool running = false;

	bool recording = false;
	bool recordMode = false;

	bool autoStop = false;
	bool pre = true;

	float phase = 0.f;
	int index = 0;

	float newVolt = 0.f;
	float out = 0.f;

	float voltages[32] = {0.f};

	float getRate()
	{
		float rate = params[CLOCK_PARAM].getValue();
		rate += inputs[CLOCK_INPUT].getVoltage();
		rate = std::pow(2.f, rate);
		return rate;
	}

	int getSteps()
	{
		int steps = (int)params[NSTEPS_PARAM].getValue();
		steps += (int)inputs[NSTEPS_INPUT].getVoltage();
		steps = clamp(steps, 1, 32);
		return steps;
	}

	void record(float newVolt)
	{
		// float newVolt = inputs[IN_INPUT].getNormalVoltage(params[CV_PARAM].getValue());
		voltages[index] = newVolt;
	}

	void advanceIndex()
	{
		int max = getSteps() - 1;

		if (pre && recording)
		{
			record(newVolt);
		}

		++index;

		if (!pre && recording)
		{
			record(newVolt);
		}

		if (index > max)
		{
			index = 0;

			if (autoStop)
			{
				// Stops recording when the index rolls over
				recording = false;
			}
		}
	}

	void lfoPhase(float rate, float delta)
	{
		// Accumulate phase
		phase += rate * delta;

		// After one cycle advance the sequencer index
		if (phase >= 1.f)
		{
			advanceIndex();
			phase = 0.f;
		}
	}

	void reset()
	{
		if (resetTrigger.process(params[RESET_PARAM].getValue() + inputs[RESET_INPUT].getVoltage()))
		{
			index = 0;
		}
	}
	void skip()
	{
		if (stepTrigger.process(params[STEPADV_PARAM].getValue() + inputs[STEPADV_INPUT].getVoltage()))
		{
			advanceIndex();
		}
	}

	void sequencerstep()
	{
		out = voltages[index];

		float ledValue = out / 10.0;

		// Set position lights
		for (int i = 0; i < 32; ++i)
		{
			lights[LED1_LIGHT + i * 3].setBrightness(0);
			lights[LED1_LIGHT + i * 3 + 1].setBrightness(0);
		}
		lights[LED1_LIGHT + index * 3].setBrightness(0.5 + 0.5 * -1 * ledValue);
		lights[LED1_LIGHT + index * 3 + 1].setBrightness(0.5 + 0.5 * ledValue);
	}

	void startControls()
	{
		// Get the start/stop mode
		// false = trig, true = gate,
		bool startMode = (bool)params[STARTTOGGLE_PARAM].getValue();

		if (startMode)
		{
			// Gate (momentary) start
			if (params[START_PARAM].getValue() || inputs[START_INPUT].getVoltage())
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
			if (startTrigger.process(params[START_PARAM].getValue() + inputs[START_INPUT].getVoltage()))
			{
				running = !running;
			}
		}
	}

	void recordControls()
	{
		if (params[AUTO_PARAM].getValue())
		{
			autoStop = true;
		}
		else
		{
			autoStop = false;
		}

		if (params[PRE_PARAM].getValue())
		{
			pre = true;
		}
		else
		{
			pre = false;
		}

		// Get the record mode
		// false = trig, true = gate,
		recordMode = (bool)params[RECTOGGLE_PARAM].getValue();

		if (recordMode)
		{
			// Gate (momentary) record
			if (params[REC_PARAM].getValue() || inputs[REC_INPUT].getVoltage())
			{
				recording = true;
			}
			else
			{
				recording = false;
			}
		}
		else
		{
			// Trig (toggle) recording
			if (recordTrigger.process(params[REC_PARAM].getValue() + inputs[REC_INPUT].getVoltage()))
			{
				recording = !recording;
			}
		}
	}

	void process(const ProcessArgs &args) override
	{
		startControls();

		if (running)
		{
			// Get clock rate
			float clockRate = getRate();

			// Accumulate LFO
			lfoPhase(clockRate, args.sampleTime);
		}

		recordControls();

		newVolt = inputs[IN_INPUT].getNormalVoltage(params[CV_PARAM].getValue());

		skip();
		reset();

		sequencerstep();

		if (recording)
		{
			lights[REC_LIGHT].setBrightness(1);
			out = newVolt;
		}
		else
		{
			lights[REC_LIGHT].setBrightness(0);
		}

		outputs[OUT_OUTPUT].setVoltage(out);
	}
};

struct FaxWidget : ModuleWidget
{
	FaxWidget(Fax *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Fax.svg")));

		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<FF15GSnapKnob>(mm2px(Vec(24.0, 37.562)), module, Fax::NSTEPS_PARAM));
		addParam(createParamCentered<FF15GKnob>(mm2px(Vec(57.28, 37.562)), module, Fax::CLOCK_PARAM));
		addParam(createParamCentered<FFDPW>(mm2px(Vec(12.0, 62.056)), module, Fax::STEPADV_PARAM));
		addParam(createParamCentered<FFDPW>(mm2px(Vec(69.28, 62.056)), module, Fax::RESET_PARAM));
		addParam(createParamCentered<FF20GKnob>(mm2px(Vec(40.724, 68.343)), module, Fax::CV_PARAM));
		addParam(createParamCentered<FFDPW>(mm2px(Vec(16.0, 90.009)), module, Fax::START_PARAM));
		addParam(createParamCentered<FFDPW>(mm2px(Vec(65.28, 90.009)), module, Fax::REC_PARAM));
		addParam(createParamCentered<HCKSS>(mm2px(Vec(16.0, 99.716)), module, Fax::STARTTOGGLE_PARAM));
		addParam(createParamCentered<HCKSS>(mm2px(Vec(65.28, 99.716)), module, Fax::RECTOGGLE_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(9.0, 29.647)), module, Fax::PRE_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(72.28, 29.647)), module, Fax::AUTO_PARAM));

		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(24.0, 23.417)), module, Fax::NSTEPS_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(57.28, 23.417)), module, Fax::CLOCK_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(40.64, 36.251)), module, Fax::IN_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(11.905, 74.976)), module, Fax::STEPADV_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(69.28, 74.976)), module, Fax::RESET_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(16.0, 113.225)), module, Fax::START_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(65.28, 113.225)), module, Fax::REC_INPUT));

		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(40.64, 100.386)), module, Fax::OUT_OUTPUT));

		for (int i = 0; i < 32; i++)
		{
			addChild(createLightCentered<MediumLight<RedGreenBlueLight>>(mm2px(Vec(ledPos[i][0], ledPos[i][1])), module, Fax::LED1_LIGHT + (i * 3)));
		}
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(56.28, 113.225)), module, Fax::REC_LIGHT));
	}
};

Model *modelFax = createModel<Fax, FaxWidget>("Fax");