// CV Recorder/Sequencer
// Ross Cameron 2020/07/11

// Title font - ARK-ES Dense Regular
// https://stued.io/projects/ark-typeface
// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"

// Magic numbers for the led positions
// These came from using tranform tools in illustrator, but I should write a function to generate them within the widget struct

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
		ENUMS(LED1_LIGHT, 32),
		NUM_LIGHTS
	};

	Fax()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(NSTEPS_PARAM, 0.f, 32.f, 0.f, "");
		configParam(CLOCK_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STEPADV_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RESET_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CV_PARAM, 0.f, 1.f, 0.f, "");
		configParam(START_PARAM, 0.f, 1.f, 0.f, "");
		configParam(REC_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STARTTOGGLE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RECTOGGLE_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs &args) override
	{
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

		addParam(createParamCentered<FF15GSnapKnob>(mm2px(Vec(21.0, 37.562)), module, Fax::NSTEPS_PARAM));
		addParam(createParamCentered<FF15GKnob>(mm2px(Vec(60.28, 37.562)), module, Fax::CLOCK_PARAM));
		addParam(createParamCentered<FFDPW>(mm2px(Vec(12.0, 62.056)), module, Fax::STEPADV_PARAM));
		addParam(createParamCentered<FFDPW>(mm2px(Vec(69.28, 62.056)), module, Fax::RESET_PARAM));
		addParam(createParamCentered<FF20GKnob>(mm2px(Vec(40.724, 68.343)), module, Fax::CV_PARAM));
		addParam(createParamCentered<FFDPW>(mm2px(Vec(16.0, 90.009)), module, Fax::START_PARAM));
		addParam(createParamCentered<FFDPW>(mm2px(Vec(65.28, 90.009)), module, Fax::REC_PARAM));
		addParam(createParamCentered<HCKSS>(mm2px(Vec(16.0, 99.716)), module, Fax::STARTTOGGLE_PARAM));
		addParam(createParamCentered<HCKSS>(mm2px(Vec(65.28, 99.716)), module, Fax::RECTOGGLE_PARAM));

		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(21.0, 23.417)), module, Fax::NSTEPS_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(60.28, 23.417)), module, Fax::CLOCK_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(40.64, 36.251)), module, Fax::IN_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(11.905, 74.976)), module, Fax::STEPADV_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(69.28, 74.976)), module, Fax::RESET_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(16.0, 113.225)), module, Fax::START_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(65.28, 113.225)), module, Fax::REC_INPUT));

		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(40.64, 100.386)), module, Fax::OUT_OUTPUT));

		for (int i = 0; i < 32; i++)
		{
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(ledPos[i][0], ledPos[i][1])), module, Fax::LED1_LIGHT + i));
		}
	}
};

Model *modelFax = createModel<Fax, FaxWidget>("Fax");