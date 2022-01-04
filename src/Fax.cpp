// CV Recorder/Sequencer
// Ross Cameron 2020/07/11

// Title font - ARK-ES Dense Regular
// https://stued.io/projects/ark-typeface
// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"

// Magic numbers for the led positions
// These came from using tranform tools in Adobe Illustrator
// I should write a function to generate them within the widget struct, it's simple trig

const float ledPos[32][2] = {
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
		ENUMS(LED1_LIGHT, 96),
		REC_LIGHT,
		NUM_LIGHTS
	};

	Fax()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configParam(NSTEPS_PARAM, 1.f, 32.f, 16.f, "Sequencer Steps");
		configParam(CLOCK_PARAM, -2.f, 6.f, 2.f, "Clock Rate", "BPM", 2.f, 60.f);
		configButton(STEPADV_PARAM, "Step");
		configButton(RESET_PARAM, "Reset");
		configParam(CV_PARAM, -5.f, 5.f, 0.f, "CV");
		configButton(START_PARAM, "Start");
		configButton(REC_PARAM, "Record");
		configSwitch(STARTTOGGLE_PARAM, 0.f, 1.f, 0.f, "Start Mode", {"Trigger", "Gate"});
		configSwitch(RECTOGGLE_PARAM, 0.f, 1.f, 0.f, "Record Mode", {"Trigger", "Gate"});
		configSwitch(PRE_PARAM, 0.f, 1.f, 0.f, "Pre/Post", {"Post", "Pre"});
		configSwitch(AUTO_PARAM, 0.f, 1.f, 1.f, "Auto Stop", {"Continue", "Stop"});

		configInput(NSTEPS_INPUT, "Sequencer Steps CV");
		configInput(CLOCK_INPUT, "Clock Rate CV");
		configInput(IN_INPUT, "CV");
		configInput(STEPADV_INPUT, "Step Advance Trigger");
		configInput(RESET_INPUT, "Reset Trigger");
		configInput(START_INPUT, "Start Trigger");
		configInput(REC_INPUT, "Record Trigger");

		configOutput(OUT_OUTPUT, "CV");

		for (int i = 0; i < 32; ++i)
		{
			configLight(LED1_LIGHT + i * 3, string::f("Step %d", i + 1));
		}
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
	bool pre = false;

	float phase = 0.f;
	int index = 0;

	// Initialise in auto
	// Auto - menuChannels == -1 - channels = N channels of whatever is input
	// !Auto - menuChannels >= 0 - channels = set by context menu
	int menuChannels = -1;
	int channels = 1;

	// One voltage to record and/or output for each poly channel
	float newVolt[16] = {0.f};
	float out[16] = {0.f};

	float voltages[16][32] = {{0.f}};

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

	void record(float newVolt, int c)
	{
		voltages[c][index] = newVolt;
	}

	void advanceIndex()
	{
		int max = getSteps() - 1;

		if (pre && recording)
		{
			for (int c = 0; c < 16; ++c)
			{
				record(newVolt[c], c);
			}
		}

		++index;

		if (!pre && recording)
		{
			for (int c = 0; c < 16; ++c)
			{
				record(newVolt[c], c);
			}
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
		for (int c = 0; c < 16; ++c)
		{
			out[c] = voltages[c][index];
		}

		// Set every light to off
		for (int i = 0; i < 32; ++i)
		{
			lights[LED1_LIGHT + i * 3].setBrightness(0);
			lights[LED1_LIGHT + i * 3 + 1].setBrightness(0);
			lights[LED1_LIGHT + i * 3 + 2].setBrightness(0);
		}

		// Set the light for the current step
		// LEDs only represent the output voltage if in mono
		if (channels < 2)
		{
			float ledValue = out[0] / 10.0;

			lights[LED1_LIGHT + index * 3].setBrightness(0.5 + 0.5 * -1 * ledValue);
			lights[LED1_LIGHT + index * 3 + 1].setBrightness(0.5 + 0.5 * ledValue);
			lights[LED1_LIGHT + index * 3 + 2].setBrightness(0);
		}
		// If poly, LEDs are solid blue.
		else
		{

			lights[LED1_LIGHT + index * 3].setBrightness(0);
			lights[LED1_LIGHT + index * 3 + 1].setBrightness(0);
			lights[LED1_LIGHT + index * 3 + 2].setBrightness(1);
		}
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

	void getInputVoltages()
	{
		// Get number of channels
		// Auto
		if (menuChannels == -1)
		{
			channels = inputs[IN_INPUT].getChannels();
		}
		// Menu determined
		else
		{
			channels = menuChannels + 1;
		}

		// Get voltages, either from channels or from the knob
		if (channels)
		{
			// Get voltage from each incoming poly channel
			for (int c = 0; c < channels; ++c)
			{
				newVolt[c] = inputs[IN_INPUT].getPolyVoltage(c);
			}
			// If the input is monophonic then input is duplicated to all output channels
			if (channels == 1)
			{
				for (int c = channels; c < 16; ++c)
				{
					newVolt[c] = newVolt[0];
				}
			}
			else
			{
				// If there's less than 16 channels coming in, set the remaining ones to 0v
				for (int c = channels; c < 16; ++c)
				{
					newVolt[c] = 0;
				}
			}
			// NOTE - all of these newVolt values are just to represent whatever voltage is being input at the current frame
			// They'll only be stored and output if recording is active.
		}
		else
		{
			// No channels are connected, so we take our value to record from the big knob
			newVolt[0] = params[CV_PARAM].getValue();

			// All channels are set to the same voltage
			for (int c = 1; c < 16; ++c)
			{
				newVolt[c] = newVolt[0];
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

		getInputVoltages();

		skip();
		reset();

		sequencerstep();

		if (recording)
		{
			lights[REC_LIGHT].setBrightness(1);
			for (int c = 0; c < channels; ++c)
			{
				out[c] = newVolt[c];
			}
		}
		else
		{
			lights[REC_LIGHT].setBrightness(0);
		}

		for (int c = 0; c < channels; ++c)
		{
			outputs[OUT_OUTPUT].setVoltage(out[c], c);
		}

		outputs[OUT_OUTPUT].setChannels(channels);

		// HACKY make sure we still output an already recorded channel if the current Nchans == 0
		if (channels == 0)
		{
			outputs[OUT_OUTPUT].setVoltage(out[0], 0);
			outputs[OUT_OUTPUT].setChannels(1);
		}
	}

	void onReset() override
	{
		// autoPoly = true;
		running = false;
		menuChannels = -1;
		index = 0;
		phase = 0.f;

		for (int i = 0; i < 16; ++i)
		{
			newVolt[i] = 0.f;
			out[i] = 0.f;
			for (int j = 0; j < 32; ++j)
			{
				voltages[i][j] = 0.f;
			}
		}
	}

	json_t *dataToJson() override
	{
		json_t *rootJ = json_object();
		// json_object_set_new(rootJ, "Auto Polyphony", json_boolean(autoPoly));
		json_object_set_new(rootJ, "Polyphony Channels", json_integer(menuChannels));
		json_object_set_new(rootJ, "Index", json_integer(index));
		json_object_set_new(rootJ, "Running", json_integer(running));

		// stored voltages
		json_t *chansJ = json_array();
		for (int i = 0; i < 16; ++i)
		{
			json_t *stepsJ = json_array();

			for (int j = 0; j < 32; ++j)
			{
				json_array_insert_new(stepsJ, j, json_real(voltages[i][j]));
			}

			json_array_insert_new(chansJ, i, stepsJ);
		}
		json_object_set_new(rootJ, "Stored Voltages", chansJ);

		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override
	{
		// json_t *autoJ = json_object_get(rootJ, "Auto Polyphony");
		// if (autoJ)
		// 	autoPoly = json_boolean_value(autoJ);

		json_t *channelsJ = json_object_get(rootJ, "Polyphony Channels");
		if (channelsJ)
			menuChannels = json_integer_value(channelsJ);

		json_t *indexJ = json_object_get(rootJ, "Index");
		if (indexJ)
			index = json_integer_value(indexJ);

		json_t *runningJ = json_object_get(rootJ, "Running");
		if (runningJ)
			running = json_is_true(runningJ);

		json_t *chansJ = json_object_get(rootJ, "Stored Voltages");
		if (chansJ)
		{
			for (int i = 0; i < 16; ++i)
			{
				json_t *chanJ = json_array_get(chansJ, i);
				if (chanJ)
				{
					for (int j = 0; j < 32; ++j)
					{
						json_t *stepJ = json_array_get(chanJ, j);
						if (stepJ)
						{
							voltages[i][j] = (float)json_real_value(stepJ);
						}
					}
				}
			}
		}
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

	void appendContextMenu(Menu *menu) override
	{
		Fax *fax = dynamic_cast<Fax *>(module);
		assert(fax);

		struct ChannelValueItem : MenuItem
		{
			Fax *fax;
			int c;
			void onAction(const event::Action &e) override
			{
				fax->menuChannels = c;
			}
		};

		struct FaxPolyChansItem : MenuItem
		{
			Fax *fax;
			Menu *createChildMenu() override
			{
				Menu *menu = new Menu;
				for (int c = -1; c < 16; ++c)
				{
					ChannelValueItem *item = new ChannelValueItem;
					if (c == -1)
						item->text = "Auto";
					else
						item->text = string::f("%d", c + 1);
					item->rightText = CHECKMARK(fax->menuChannels == c);
					item->fax = fax;
					item->c = c;
					menu->addChild(item);
				}
				return menu;
			}
		};

		menu->addChild(new MenuEntry);
		FaxPolyChansItem *faxPolyChansItem = new FaxPolyChansItem;
		faxPolyChansItem->text = "Polyphony Channels";
		if (fax->menuChannels == -1)
			faxPolyChansItem->rightText = string::f("Auto ") + RIGHT_ARROW;
		else
			faxPolyChansItem->rightText = string::f("%d", fax->channels) + " " + RIGHT_ARROW;
		faxPolyChansItem->fax = fax;
		menu->addChild(faxPolyChansItem);
	}
};

Model *modelFax = createModel<Fax, FaxWidget>("Fax");