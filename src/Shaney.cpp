#include "plugin.hpp"

#include "ffCommon.hpp"
#include "random.hpp"
#include <algorithm>
#include <cstddef>
#include <iterator>

const int n_steps = 8;
const int n_elements = n_steps * n_steps;
// For a sequence of n steps the Markov array will have n*n elements

struct Shaney : Module {
	enum ParamId {
		ENUMS(PROB_PARAM, n_steps),
		ENUMS(STEP_PARAM, n_elements),
		PARAMS_LEN
	};
	enum InputId {
		ENUMS(JUMP_INPUT, n_steps),
		CLOCK_INPUT,
		RUN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(GATE_OUTPUT, n_steps),
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(STEP_LIGHT, n_steps),
		LIGHTS_LEN
	};

	dsp::SchmittTrigger clockTrigger;

	dsp::SchmittTrigger jumpTriggers[n_steps];

	int sequencer_index = 0;
	bool out = false;
	bool running = true;

	Shaney() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

        configInput(CLOCK_INPUT, "External Clock Trigger");

		for (int i = 0; i < n_elements; ++i)
		{
			float default_value = (i % n_steps == i / n_steps) ? 1.f : 0.f;
			std::string knob_name = string::f("Probability %d", i);
			configParam(PROB_PARAM + i, 0.f, 1.f, default_value, knob_name);
		}

		for (int i = 0; i < n_steps; ++i)
		{
            configInput(JUMP_INPUT + i, string::f("Step %d Jump", i + 1));
            configOutput(GATE_OUTPUT + i, string::f("Step %d Gate Output", i + 1));
            configLight(STEP_LIGHT + i, string::f("Step %d", i + 1));
		}
	}

	void process(const ProcessArgs& args) override {
		// Only bother to check the probabilities if the sequencer is running
		if (running && inputs[CLOCK_INPUT].isConnected())
		{
			if (clockTrigger.process(inputs[CLOCK_INPUT].getVoltage()))
			{

				// Add an extra step for the probabilty of the sequence stopping
				float cumulative_probabilities[n_steps + 1] = {0};
				float running_total = 0.0;
				for (int i = 0; i < n_steps; ++i)
				{
					float this_probability = params[PROB_PARAM + i + (n_steps * sequencer_index)].getValue();
					running_total += this_probability;
					cumulative_probabilities[i] = running_total;
				}
				float chance_of_stopping = fmax(0, 1 - running_total);
				running_total += chance_of_stopping;
				cumulative_probabilities[n_steps] = running_total;
				float random_value = random::uniform() * running_total;
				auto bisected = std::upper_bound(std::begin(cumulative_probabilities), std::end(cumulative_probabilities), random_value);
				int new_index = bisected - std::begin(cumulative_probabilities);
				if (new_index < n_steps) {
					sequencer_index = new_index;
				}
			}
		}

				
		for (int i = 0; i < n_steps; ++i)
		{
			outputs[GATE_OUTPUT + i];
			lights[STEP_LIGHT + i].setBrightness(0);
			if (jumpTriggers[i].process(inputs[JUMP_INPUT + i].getVoltage()))
			{
				sequencer_index = i;
			}
		}

		float out = clockTrigger.isHigh() * 10;
		lights[STEP_LIGHT + sequencer_index].setBrightness(1);
		if (running)
		{
			outputs[GATE_OUTPUT + sequencer_index].setVoltage(out);
		}
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

		float input_x_base = 160;
		Vec clock_input_pos = Vec(input_x_base, 30);
		addInput(createInputCentered<FF01JKPort>(mm2px(clock_input_pos), module, Shaney::CLOCK_INPUT));

		float knob_x_base = 2 * RACK_GRID_WIDTH;
		float knob_y_base = 24;
		float x_delta = 13;
		float y_delta = 11;

		for (int i = 0; i < n_steps; ++i)
		{
			float x_pos = knob_x_base + i * x_delta;
			for (int j = 0; j < n_steps; ++j)
			{
				float y_pos = knob_y_base + (j % n_steps) * y_delta;
				Vec knob_pos = Vec(x_pos, y_pos);
				int element_index = i * n_steps + j;
				addParam(createParamCentered<FF08GKnob>(mm2px(knob_pos), module, Shaney::PROB_PARAM + element_index));
			}

			float in_y_pos = 14;
			Vec in_pos = Vec(x_pos, in_y_pos);
			addInput(createInputCentered<FF01JKPort>(mm2px(in_pos), module, Shaney::JUMP_INPUT + i));

			float led_y_pos = 108;
			Vec led_pos = Vec(x_pos, led_y_pos);
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(led_pos), module, Shaney::STEP_LIGHT + i));

			float out_y_pos = 115;
			Vec out_pos = Vec(x_pos, out_y_pos);
			addOutput(createOutputCentered<FF01JKPort>(mm2px(out_pos), module, Shaney::GATE_OUTPUT + i));
		}
	}
};


Model* modelShaney = createModel<Shaney, ShaneyWidget>("Shaney");
