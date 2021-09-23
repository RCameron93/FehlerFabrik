#include "plugin.hpp"


struct Botzinger : Module {
	enum ParamIds {
		ENUMS(TIME_PARAM, 8),
		ENUMS(REPEAT_PARAM, 8),
		ENUMS(WIDTH_PARAM, 8),
		START_PARAM,
		DIRECTION_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(TIME_INPUT, 8),
		ENUMS(REPEAT_INPUT, 8),
		ENUMS(WIDTH_INPUT, 8),
		CLOCK_INPUT,
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
		NUM_LIGHTS
	};

	Botzinger() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		for (int i = 0; i < 8; ++i)
		{
			configParam(TIME_PARAM + i, 0.f, 1.f, 0.f, "");
			configParam(REPEAT_PARAM + i, 0.f, 1.f, 0.f, "");
			configParam(WIDTH_PARAM + i, 0.f, 1.f, 0.f, "");
		}
		
		configParam(START_PARAM, 0.f, 1.f, 0.f, "");
		configParam(DIRECTION_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
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

			addParam(createParamCentered<BefacoSlidePot>(mm2px(Vec(31.462 + (i * deltaX), 50.814)), module, Botzinger::TIME_PARAM + i));
			addParam(createParamCentered<FF08GKnob>(mm2px(Vec(31.462 + (i * deltaX), 89.104)), module, Botzinger::REPEAT_PARAM + i));
			addParam(createParamCentered<FF08GKnob>(mm2px(Vec(31.462 + (i * deltaX), 100.669)), module, Botzinger::WIDTH_PARAM + i));

			addInput(createInputCentered<FF01JKPort>(mm2px(Vec(31.462 + (i * deltaX), 24.189)), module, Botzinger::TIME_INPUT + i));
			addInput(createInputCentered<FF01JKPort>(mm2px(Vec(31.462 + (i * deltaX), 76.492)), module, Botzinger::REPEAT_INPUT + i));

			addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(31.462 + (i * deltaX), 113.225)), module, Botzinger::OUTS_OUTPUT + i));
		}

		addParam(createParamCentered<FFDPW>(mm2px(Vec(167.958, 56.407)), module, Botzinger::START_PARAM));
		addParam(createParamCentered<FFDPW>(mm2px(Vec(167.958, 76.492)), module, Botzinger::DIRECTION_PARAM));
	
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(161.638, 24.189)), module, Botzinger::CLOCK_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(155.317, 56.407)), module, Botzinger::START_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(155.317, 76.492)), module, Botzinger::DIRECTION_INPUT));

		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(155.317, 113.225)), module, Botzinger::MAIN_OUTPUT));
	}
};


Model* modelBotzinger = createModel<Botzinger, BotzingerWidget>("Botzinger");