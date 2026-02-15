// Markov Step Sequencer
// Title Font -  STEREO SANS by Joonas Timmi (SUVA type foundry)
// https://www.suvatypefoundry.ee/stereo-sans/
// Main font - Jost
// https://indestructibletype.com/Jost.html


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
		RUN_LIGHT,
		LIGHTS_LEN
	};

	dsp::SchmittTrigger clockTrigger;
	dsp::SchmittTrigger runTrigger;
	dsp::SchmittTrigger jumpTriggers[n_steps];

	int sequencer_index = 0;
	bool out = false;
	bool running = true;

	Shaney() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

        configInput(CLOCK_INPUT, "External Clock Trigger");

		// Init position of knobs is 100% chance of moving to the next step
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

				// Create an array that will hold the probabilty of moving to each step
				// With an extra step for the probabilty of the sequence stopping
				// These are stored as cumulative probabilities, akin to a cumulative distribution function
				// Note that these aren't **really** probabilities since the total of them all can sum to greater than 1
				float cumulative_probabilities[n_steps + 1] = {0};
				// This will be a running total of all the probabilities
				float running_total = 0.0;

				// Read the probability for each step into the array and add to the running total
				for (int i = 0; i < n_steps; ++i)
				{
					float this_probability = params[PROB_PARAM + i + (n_steps * sequencer_index)].getValue();
					running_total += this_probability;
					cumulative_probabilities[i] = running_total;
				}

				// If the running total is less than 1, there's a chance that the sequencer will stop
				// That chance is the difference between the current running total and 1
				float chance_of_stopping = fmax(0, 1 - running_total);
				running_total += chance_of_stopping;
				cumulative_probabilities[n_steps] = running_total;

				// Get a random value that determines which step to change to 
				// This value is a fraction of the running total
				// The step that will be jumped to is the one that is to the left of where the random_value would be inserted into the probabilities array
				float random_value = random::uniform() * running_total;
				
				// Find where that insertion point would be, relative to the beginning of the array
				auto bisected = std::upper_bound(std::begin(cumulative_probabilities), std::end(cumulative_probabilities), random_value);
				// Pointers! C++!
				int new_index = bisected - std::begin(cumulative_probabilities);
				// Check that the new index is not the final one in the array, ie the one that represents sequencer stopping
				if (new_index < n_steps) {
					sequencer_index = new_index;
				}
				// If the new_index _is_ the final position, we leave the sequencer_index as it is and stop running
				else
				{
					running = false;
				}
			}
		}

		// Check if the Start input has been triggered
		// As this comes after the stopping probability check above it will take precedence
		if (runTrigger.process(inputs[RUN_INPUT].getVoltage()))
		{
			running = true;
		}

		// Reset the step outputs and lights
		for (int i = 0; i < n_steps; ++i)
		{
			outputs[GATE_OUTPUT + i].setVoltage(0);
			lights[STEP_LIGHT + i].setBrightness(0);
			// Check for jump triggers
			// If multiple triggers are detected on a process() call then the highest one takes priority
			// The highest step trigger takes priority
			if (jumpTriggers[i].process(inputs[JUMP_INPUT + i].getVoltage()))
			{
				sequencer_index = i;
			}
		}

		// Output
		lights[STEP_LIGHT + sequencer_index].setBrightness(1);
		if (running)
		{
			lights[RUN_LIGHT].setBrightness(1);
			float out = clockTrigger.isHigh() * 10;
			outputs[GATE_OUTPUT + sequencer_index].setVoltage(out);
		}
		else
		{
			lights[RUN_LIGHT].setBrightness(0);
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

		float knob_x_base = 21.926;
		float knob_y_base = 31.110;
		float x_delta = 15.507;
		float y_delta = 10.5;

		float input_x_base = 143.894;
		Vec clock_input_pos = Vec(input_x_base, knob_y_base);
		addInput(createInputCentered<FF01JKPort>(mm2px(clock_input_pos), module, Shaney::CLOCK_INPUT));

		Vec run_input_pos = Vec(input_x_base, knob_y_base + 2 * y_delta);
		addInput(createInputCentered<FF01JKPort>(mm2px(run_input_pos), module, Shaney::RUN_INPUT));

		Vec run_light_pos = Vec(input_x_base, 60);
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(run_light_pos), module, Shaney::RUN_LIGHT));

		for (int i = 0; i < n_steps; ++i)
		{
			// X position for the knobs and ports for this step
			float x_pos = knob_x_base + i * x_delta;

			// Probability knobs for each step
			for (int j = 0; j < n_steps; ++j)
			{
				float y_pos = knob_y_base + (j % n_steps) * y_delta;
				Vec knob_pos = Vec(x_pos, y_pos);
				int element_index = i * n_steps + j;
				addParam(createParamCentered<FF08GKnob>(mm2px(knob_pos), module, Shaney::PROB_PARAM + element_index));
			}

			float in_y_pos = 15.776;
			Vec in_pos = Vec(x_pos, in_y_pos);
			addInput(createInputCentered<FF01JKPort>(mm2px(in_pos), module, Shaney::JUMP_INPUT + i));

			// float led_y_pos = 108;
			float led_y_pos = 22.7;
			Vec led_pos = Vec(x_pos, led_y_pos);
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(led_pos), module, Shaney::STEP_LIGHT + i));

			float out_y_pos = 113.813;
			Vec out_pos = Vec(x_pos, out_y_pos);
			addOutput(createOutputCentered<FF01JKPort>(mm2px(out_pos), module, Shaney::GATE_OUTPUT + i));
		}
	}
};


Model* modelShaney = createModel<Shaney, ShaneyWidget>("Shaney");
