#include "plugin.hpp"

#include "ffCommon.hpp"

const int n_steps = 16;
// For a sequence of n steps the Markov array will have n*n elements

struct Shaney : Module {
	enum ParamId {
		ENUMS(PROB_PARAM, n_steps),
		ENUMS(STEP_PARAM, n_steps),
		PARAMS_LEN
	};
	enum InputId {
		CLOCK_INPUT,
		RESET_INPUT,
		RUN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(GATE_OUTPUT, n_steps),
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(ELEMENT_LIGHT, n_steps * n_steps),
		LIGHTS_LEN
	};

	dsp::SchmittTrigger clockTrigger;
	dsp::SchmittTrigger resetTrigger;

	int sequencer_index = 0;

	Shaney() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
	}

	void process(const ProcessArgs& args) override {
	}
};


struct ShaneyWidget : ModuleWidget {
	ShaneyWidget(Shaney* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Shaney.svg")));

		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		float led_x_base = RACK_GRID_WIDTH;
		float led_y_base = 24;
		float x_delta = 11;
		float y_delta = 5.5;

		for (int i = 0; i < n_steps; ++i)
		{
			float x_pos = led_x_base + i * x_delta;
			for (int j = 0; j < n_steps; ++j)
			{
				float y_pos = led_y_base + (j % n_steps) * y_delta;
				Vec led_pos = Vec(x_pos, y_pos);
				addChild(createLightCentered<MediumLight<RedLight>>(mm2px(led_pos), module, Shaney::ELEMENT_LIGHT + i));
			}
			float button_y_pos = 14;
			Vec button_pos = Vec(x_pos, button_y_pos);
			addParam(createParamCentered<FFDPW>(mm2px(button_pos), module, Shaney::STEP_PARAM + i));

			float out_y_pos = 115;
			Vec out_pos = Vec(x_pos, out_y_pos);
			addOutput(createOutputCentered<FF01JKPort>(mm2px(out_pos), module, Shaney::GATE_OUTPUT + i));
		}
	}
};


Model* modelShaney = createModel<Shaney, ShaneyWidget>("Shaney");
