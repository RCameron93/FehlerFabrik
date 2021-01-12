#include "plugin.hpp"


struct Nova : Module {
	enum ParamIds {
		GAIN1_PARAM,
		GAIN2_PARAM,
		GAIN3_PARAM,
		GAIN4_PARAM,
		GAIN5_PARAM,
		GAIN6_PARAM,
		GAIN7_PARAM,
		GAIN8_PARAM,
		MUTE1_PARAM,
		MUTE2_PARAM,
		MUTE3_PARAM,
		MUTE4_PARAM,
		MUTE5_PARAM,
		MUTE6_PARAM,
		MUTE7_PARAM,
		MUTE8_PARAM,
		ATTACK_PARAM,
		SKIP1_PARAM,
		SKIP2_PARAM,
		SKIP3_PARAM,
		SKIP4_PARAM,
		SKIP5_PARAM,
		SKIP6_PARAM,
		SKIP7_PARAM,
		SKIP8_PARAM,
		START_PARAM,
		RELEASE_PARAM,
		REVERSE1_PARAM,
		REVERSE2_PARAM,
		REVERSE3_PARAM,
		REVERSE4_PARAM,
		REVERSE5_PARAM,
		REVERSE6_PARAM,
		REVERSE7_PARAM,
		REVERSE8_PARAM,
		RESET_PARAM,
		TRIGGER1_PARAM,
		TRIGGER2_PARAM,
		TRIGGER3_PARAM,
		TRIGGER4_PARAM,
		TRIGGER5_PARAM,
		TRIGGER6_PARAM,
		TRIGGER7_PARAM,
		TRIGGER8_PARAM,
		PITCH_PARAM,
		DIRECTION_PARAM,
		RECORD_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN_INPUT,
		CLOCK_INPUT,
		START_INPUT,
		RESET_INPUT,
		DIRECTION_INPUT,
		TRIGGER1_INPUT,
		TRIGGER2_INPUT,
		TRIGGER3_INPUT,
		TRIGGER4_INPUT,
		TRIGGER5_INPUT,
		TRIGGER6_INPUT,
		TRIGGER7_INPUT,
		TRIGGER8_INPUT,
		RECORD_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		OUT3_OUTPUT,
		OUT4_OUTPUT,
		OUT5_OUTPUT,
		OUT6_OUTPUT,
		OUT7_OUTPUT,
		OUT8_OUTPUT,
		MAINOUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		SEQ1_LIGHT,
		SEQ2_LIGHT,
		SEQ3_LIGHT,
		SEQ4_LIGHT,
		SEQ5_LIGHT,
		SEQ6_LIGHT,
		SEQ7_LIGHT,
		SEQ8_LIGHT,
		NUM_LIGHTS
	};

	Nova() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(GAIN1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(GAIN2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(GAIN3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(GAIN4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(GAIN5_PARAM, 0.f, 1.f, 0.f, "");
		configParam(GAIN6_PARAM, 0.f, 1.f, 0.f, "");
		configParam(GAIN7_PARAM, 0.f, 1.f, 0.f, "");
		configParam(GAIN8_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MUTE1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MUTE2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MUTE3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MUTE4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MUTE5_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MUTE6_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MUTE7_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MUTE8_PARAM, 0.f, 1.f, 0.f, "");
		configParam(ATTACK_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SKIP1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SKIP2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SKIP3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SKIP4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SKIP5_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SKIP6_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SKIP7_PARAM, 0.f, 1.f, 0.f, "");
		configParam(SKIP8_PARAM, 0.f, 1.f, 0.f, "");
		configParam(START_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RELEASE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(REVERSE1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(REVERSE2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(REVERSE3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(REVERSE4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(REVERSE5_PARAM, 0.f, 1.f, 0.f, "");
		configParam(REVERSE6_PARAM, 0.f, 1.f, 0.f, "");
		configParam(REVERSE7_PARAM, 0.f, 1.f, 0.f, "");
		configParam(REVERSE8_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RESET_PARAM, 0.f, 1.f, 0.f, "");
		configParam(TRIGGER1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(TRIGGER2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(TRIGGER3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(TRIGGER4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(TRIGGER5_PARAM, 0.f, 1.f, 0.f, "");
		configParam(TRIGGER6_PARAM, 0.f, 1.f, 0.f, "");
		configParam(TRIGGER7_PARAM, 0.f, 1.f, 0.f, "");
		configParam(TRIGGER8_PARAM, 0.f, 1.f, 0.f, "");
		configParam(PITCH_PARAM, 0.f, 1.f, 0.f, "");
		configParam(DIRECTION_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RECORD_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct NovaWidget : ModuleWidget {
	NovaWidget(Nova* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Nova.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(61.531, 23.43)), module, Nova::GAIN1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(76.581, 23.43)), module, Nova::GAIN2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(91.63, 23.43)), module, Nova::GAIN3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(106.68, 23.43)), module, Nova::GAIN4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(121.73, 23.43)), module, Nova::GAIN5_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(136.779, 23.43)), module, Nova::GAIN6_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(151.829, 23.43)), module, Nova::GAIN7_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(166.879, 23.43)), module, Nova::GAIN8_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(61.531, 38.834)), module, Nova::MUTE1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(76.581, 38.834)), module, Nova::MUTE2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(91.63, 38.834)), module, Nova::MUTE3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(106.68, 38.834)), module, Nova::MUTE4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(121.73, 38.834)), module, Nova::MUTE5_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(136.779, 38.834)), module, Nova::MUTE6_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(151.829, 38.834)), module, Nova::MUTE7_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(166.879, 38.834)), module, Nova::MUTE8_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(190.051, 39.694)), module, Nova::ATTACK_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(61.531, 54.238)), module, Nova::SKIP1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(76.581, 54.238)), module, Nova::SKIP2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(91.63, 54.238)), module, Nova::SKIP3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(106.68, 54.238)), module, Nova::SKIP4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(121.73, 54.238)), module, Nova::SKIP5_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(136.779, 54.238)), module, Nova::SKIP6_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(151.829, 54.238)), module, Nova::SKIP7_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(166.879, 54.238)), module, Nova::SKIP8_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(28.976, 59.34)), module, Nova::START_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(190.051, 65.493)), module, Nova::RELEASE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(61.531, 69.642)), module, Nova::REVERSE1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(76.581, 69.642)), module, Nova::REVERSE2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(91.63, 69.642)), module, Nova::REVERSE3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(106.68, 69.642)), module, Nova::REVERSE4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(121.73, 69.642)), module, Nova::REVERSE5_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(136.779, 69.642)), module, Nova::REVERSE6_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(151.829, 69.642)), module, Nova::REVERSE7_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(166.879, 69.642)), module, Nova::REVERSE8_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(28.976, 77.302)), module, Nova::RESET_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(61.531, 85.046)), module, Nova::TRIGGER1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(76.581, 85.046)), module, Nova::TRIGGER2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(91.63, 85.046)), module, Nova::TRIGGER3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(106.68, 85.046)), module, Nova::TRIGGER4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(121.73, 85.046)), module, Nova::TRIGGER5_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(136.779, 85.046)), module, Nova::TRIGGER6_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(151.829, 85.046)), module, Nova::TRIGGER7_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(166.879, 85.046)), module, Nova::TRIGGER8_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(190.051, 91.272)), module, Nova::PITCH_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(28.976, 95.263)), module, Nova::DIRECTION_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(28.976, 113.225)), module, Nova::RECORD_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.88, 23.417)), module, Nova::IN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.88, 41.379)), module, Nova::CLOCK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.335, 59.34)), module, Nova::START_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.335, 77.302)), module, Nova::RESET_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.335, 95.263)), module, Nova::DIRECTION_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(61.531, 97.487)), module, Nova::TRIGGER1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(76.581, 97.487)), module, Nova::TRIGGER2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(91.63, 97.487)), module, Nova::TRIGGER3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(106.68, 97.487)), module, Nova::TRIGGER4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(121.73, 97.487)), module, Nova::TRIGGER5_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(136.779, 97.487)), module, Nova::TRIGGER6_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(151.829, 97.487)), module, Nova::TRIGGER7_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(166.879, 97.487)), module, Nova::TRIGGER8_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.335, 113.225)), module, Nova::RECORD_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(61.531, 110.766)), module, Nova::OUT1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(76.581, 110.766)), module, Nova::OUT2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(91.63, 110.766)), module, Nova::OUT3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(106.68, 110.766)), module, Nova::OUT4_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(121.73, 110.766)), module, Nova::OUT5_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(136.779, 110.766)), module, Nova::OUT6_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(151.829, 110.766)), module, Nova::OUT7_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(166.879, 110.766)), module, Nova::OUT8_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(190.051, 110.766)), module, Nova::MAINOUT_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(61.531, 117.503)), module, Nova::SEQ1_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(76.581, 117.503)), module, Nova::SEQ2_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(91.63, 117.503)), module, Nova::SEQ3_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(106.68, 117.503)), module, Nova::SEQ4_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(121.73, 117.503)), module, Nova::SEQ5_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(136.779, 117.503)), module, Nova::SEQ6_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(151.829, 117.503)), module, Nova::SEQ7_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(166.879, 117.503)), module, Nova::SEQ8_LIGHT));
	}
};


Model* modelNova = createModel<Nova, NovaWidget>("Nova");