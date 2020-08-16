#include "plugin.hpp"

struct Chi : Module
{
	enum ParamIds
	{
		LOW_GAIN_PARAM,
		MID_GAIN_PARAM,
		HIGH_GAIN_PARAM,
		LOW_GAIN_CV_PARAM,
		MID_GAIN_CV_PARAM,
		HIGH_GAIN_CV_PARAM,
		LOW_X_PARAM,
		HIGH_X_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		LOW_GAIN_INPUT,
		MID_GAIN_INPUT,
		HIGH_GAIN_INPUT,
		LOW_X_INPUT,
		MID_X_INPUT,
		IN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		LOW_OUTPUT,
		MID_OUTPUT,
		HIGH_OUTPUT,
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		NUM_LIGHTS
	};

	Chi()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(LOW_GAIN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MID_GAIN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(HIGH_GAIN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LOW_GAIN_CV_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MID_GAIN_CV_PARAM, 0.f, 1.f, 0.f, "");
		configParam(HIGH_GAIN_CV_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LOW_X_PARAM, 0.f, 1.f, 0.f, "");
		configParam(HIGH_X_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs &args) override
	{
	}
};

struct ChiWidget : ModuleWidget
{
	ChiWidget(Chi *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Chi.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(18.473, 47.126)), module, Chi::LOW_GAIN_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(55.88, 47.126)), module, Chi::MID_GAIN_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(93.289, 47.126)), module, Chi::HIGH_GAIN_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(18.473, 70.063)), module, Chi::LOW_GAIN_CV_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(55.88, 70.063)), module, Chi::MID_GAIN_CV_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(93.289, 70.063)), module, Chi::HIGH_GAIN_CV_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(37.177, 70.063)), module, Chi::LOW_X_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(74.584, 70.063)), module, Chi::HIGH_X_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.473, 87.595)), module, Chi::LOW_GAIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(55.88, 87.595)), module, Chi::MID_GAIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(93.289, 87.595)), module, Chi::HIGH_GAIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(37.177, 87.595)), module, Chi::LOW_X_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(74.584, 87.595)), module, Chi::MID_X_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(37.177, 113.225)), module, Chi::IN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.473, 23.417)), module, Chi::LOW_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(55.88, 23.417)), module, Chi::MID_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(93.289, 23.417)), module, Chi::HIGH_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(74.584, 113.225)), module, Chi::OUT_OUTPUT));
	}
};

Model *modelChi = createModel<Chi, ChiWidget>("Chi");