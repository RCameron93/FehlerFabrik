#include "plugin.hpp"

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
		LED1_LIGHT,
		LED2_LIGHT,
		LED3_LIGHT,
		LED4_LIGHT,
		LED5_LIGHT,
		LED6_LIGHT,
		LED7_LIGHT,
		LED8_LIGHT,
		LED9_LIGHT,
		LED10_LIGHT,
		LED11_LIGHT,
		LED12_LIGHT,
		LED13_LIGHT,
		LED14_LIGHT,
		LED15_LIGHT,
		LED16_LIGHT,
		LED17_LIGHT,
		LED18_LIGHT,
		LED19_LIGHT,
		LED20_LIGHT,
		LED21_LIGHT,
		LED22_LIGHT,
		LED23_LIGHT,
		LED24_LIGHT,
		LED25_LIGHT,
		LED26_LIGHT,
		LED27_LIGHT,
		LED28_LIGHT,
		LED29_LIGHT,
		LED30_LIGHT,
		LED31_LIGHT,
		LED32_LIGHT,
		NUM_LIGHTS
	};

	Fax()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(NSTEPS_PARAM, 0.f, 1.f, 0.f, "");
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

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(40.724, 55.342)), module, Fax::LED1_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(45.699, 56.332)), module, Fax::LED2_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(49.917, 59.150)), module, Fax::LED3_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(52.735, 63.367)), module, Fax::LED4_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(53.725, 68.343)), module, Fax::LED5_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(52.735, 73.317)), module, Fax::LED6_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(49.917, 77.535)), module, Fax::LED7_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(45.699, 80.353)), module, Fax::LED8_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(40.724, 81.343)), module, Fax::LED9_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(35.749, 80.353)), module, Fax::LED10_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(31.531, 77.535)), module, Fax::LED11_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(28.713, 73.317)), module, Fax::LED12_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(27.723, 68.343)), module, Fax::LED13_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(28.713, 63.367)), module, Fax::LED14_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(31.531, 59.150)), module, Fax::LED15_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(35.749, 56.332)), module, Fax::LED16_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(40.724, 51.342)), module, Fax::LED17_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(47.230, 52.636)), module, Fax::LED18_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(52.745, 56.321)), module, Fax::LED19_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(56.430, 61.837)), module, Fax::LED20_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(57.724, 68.343)), module, Fax::LED21_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(56.430, 74.848)), module, Fax::LED22_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(52.745, 80.363)), module, Fax::LED23_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(47.230, 84.049)), module, Fax::LED24_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(40.724, 85.343)), module, Fax::LED25_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(34.218, 84.049)), module, Fax::LED26_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(28.703, 80.363)), module, Fax::LED27_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(25.018, 74.848)), module, Fax::LED28_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(23.724, 68.343)), module, Fax::LED29_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(25.018, 61.837)), module, Fax::LED30_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(28.703, 56.321)), module, Fax::LED31_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(34.218, 52.636)), module, Fax::LED32_LIGHT));
	}
};

Model *modelFax = createModel<Fax, FaxWidget>("Fax");