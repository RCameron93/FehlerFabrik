#include "plugin.hpp"
#include "../dep/lib/samplerate.h"

#define HISTORY_SIZE (1 << 21)

struct SimpleDelay
{
	// From Fundamental Delay
	// https://github.com/VCVRack/Fundamental/blob/v1/src/Delay.cpp

	dsp::DoubleRingBuffer<float, HISTORY_SIZE> historyBuffer;
	dsp::DoubleRingBuffer<float, 16> outBuffer;
	SRC_STATE *src = nullptr;

	SimpleDelay()
	{
		src = src_new(SRC_SINC_FASTEST, 1, NULL);
		assert(src);
	}

	~SimpleDelay()
	{
		src_delete(src);
	}

	float process(float in, float delay, float smpRate)
	{
		float dry = in;

		delay = 1e-3 * std::pow(10.f / 1e-3, delay);
		// Number of delay samples
		float index = std::round(delay * smpRate);

		// Push dry sample into history buffer
		if (!historyBuffer.full())
		{
			historyBuffer.push(dry);
		}

		// How many samples do we need consume to catch up?
		float consume = index - historyBuffer.size();

		if (outBuffer.empty())
		{
			double ratio = 1.f;
			if (std::fabs(consume) >= 16.f)
			{
				// Here's where the delay magic is. Smooth the ratio depending on how divergent we are from the correct delay time.
				ratio = std::pow(10.f, clamp(consume / 10000.f, -1.f, 1.f));
			}

			SRC_DATA srcData;
			srcData.data_in = (const float *)historyBuffer.startData();
			srcData.data_out = (float *)outBuffer.endData();
			srcData.input_frames = std::min((int)historyBuffer.size(), 16);
			srcData.output_frames = outBuffer.capacity();
			srcData.end_of_input = false;
			srcData.src_ratio = ratio;
			src_process(src, &srcData);
			historyBuffer.startIncr(srcData.input_frames_used);
			outBuffer.endIncr(srcData.output_frames_gen);
		}

		float wet = 0.f;
		if (!outBuffer.empty())
		{
			wet = outBuffer.shift();
		}

		return wet;
	}
};

struct SlewLimiter
{
	// Taken from Befaco Slew Limiter
	// https://github.com/VCVRack/Befaco/blob/v1/src/SlewLimiter.cpp

	float out = 0.0;

	float process(float input, float amount, float delta)
	{
		float shape = 0.f;

		// minimum and maximum slopes in volts per second
		const float slewMin = 0.1f;
		const float slewMax = 100000.f;
		// Amount of extra slew per voltage difference
		const float shapeScale = 1 / 10.f;

		// Rise
		if (input > out)
		{
			float rise = amount;
			float slew = slewMax * std::pow(slewMin / slewMax, rise);
			out += slew * crossfade(1.f, shapeScale * (input - out), shape) * delta;
			if (out > input)
				out = input;
		}
		// Fall
		else if (input < out)
		{
			float fall = amount;
			float slew = slewMax * std::pow(slewMin / slewMax, fall);
			out -= slew * crossfade(1.f, shapeScale * (out - input), shape) * delta;
			if (out < input)
				out = input;
		}

		return out;
	}
};

struct Slip : Module
{
	enum ParamIds
	{
		THRESH_PARAM,
		WET_PARAM,
		LOWSHIFTTRIM_PARAM,
		HIGHSHIFTTRIM_PARAM,
		LOWPINCHTRIM_PARAM,
		HIGHPINCHTRIM_PARAM,
		LOWFOLDTRIM_PARAM,
		HIGHFOLDTRIM_PARAM,
		LOWSLEWTRIM_PARAM,
		HIGHSLEWTRIM_PARAM,
		LOWSHIFT_PARAM,
		HIGHSHIFT_PARAM,
		LOWPINCH_PARAM,
		HIGHPINCH_PARAM,
		LOWFOLD_PARAM,
		HIGHFOLD_PARAM,
		LOWSLEW_PARAM,
		HIGHSLEW_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		THRESH_INPUT,
		IN_INPUT,
		WET_INPUT,
		LOWSHIFT_INPUT,
		HIGHSHIFT_INPUT,
		LOWPINCH_INPUT,
		HIGHPINCH_INPUT,
		LOWFOLD_INPUT,
		HIGHFOLD_INPUT,
		LOWSLEW_INPUT,
		HIGHSLEW_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		LOW_OUTPUT,
		OUT_OUTPUT,
		HIGH_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		NUM_LIGHTS
	};

	SlewLimiter slewLims[2];
	SimpleDelay delays[2];

	Slip()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(THRESH_PARAM, -10.f, 10.f, 0.f, "");
		configParam(WET_PARAM, 0.f, 1.f, 1.f, "");

		for (int i = 0; i < 8; ++i)
		{
			configParam(LOWSHIFTTRIM_PARAM + i, -1.f, 1.f, 0.f, "");
		}

		configParam(LOWSHIFT_PARAM, 0.f, 1.f, 0.f, "");
		configParam(HIGHSHIFT_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LOWPINCH_PARAM, -1.f, 1.f, 0.f, "");
		configParam(HIGHPINCH_PARAM, -1.f, 1.f, 0.f, "");
		configParam(LOWFOLD_PARAM, 0.1f, 1.f, 1.f, "");
		configParam(HIGHFOLD_PARAM, 0.1f, 1.f, 1.f, "");
		configParam(LOWSLEW_PARAM, 0.f, 1.f, 0.f, "");
		configParam(HIGHSLEW_PARAM, 0.f, 1.f, 0.f, "");
	}

	float pincher(float input, float amount)
	{
		// Taken from HetrickCV Waveshaper
		// https://github.com/mhetrick/hetrickcv/blob/master/src/Waveshape.cpp
		input *= 0.2;

		amount *= 0.99f;
		const float shapeB = (1.0 - amount) / (1.0 + amount);
		const float shapeA = (4.0 * amount) / ((1.0 - amount) * (1.0 + amount));

		float output = input * (shapeA + shapeB);
		output = output / ((std::abs(input) * shapeA) + shapeB);

		return output * 5;
	}

	float folder(float input, float amount)
	{
		// Taken from Autodafe FoldBack
		// https://github.com/antoniograzioli/Autodafe/blob/master/src/FoldBack.cpp
		input = input * 0.1f;
		if (input > amount || input < -amount)
		{
			input = fabs(fabs(fmod(input - amount, amount * 4)) - amount * 2) - amount;
		}
		return input * 10.f;
	}

	void process(const ProcessArgs &args) override
	{
		// Get input sample
		float input = inputs[IN_INPUT].getVoltage();

		// Split Input into high and low
		// What constitutes high and low is determined by the threshold
		float threshold = params[THRESH_PARAM].getValue();
		threshold += inputs[THRESH_INPUT].getVoltage();
		threshold = clamp(threshold, -10.f, 10.f);

		// Is the current sample above or below the threshold?
		bool high = input > threshold;
		// int high = 0;

		// Get parameters
		// The sample will run through the same type of process if it's high or low
		// The difference is which paramaters to use for high or low processing

		// We've interleaved the parameter ENUMS so we can avoid an if branch by simply adding the high state to the index
		float shift = params[LOWSHIFT_PARAM + high].getValue();
		shift += inputs[LOWSHIFT_INPUT + high].getVoltage() * 0.1f * params[LOWSHIFTTRIM_PARAM + high].getValue();
		shift = clamp(shift, 0.f, 1.f);

		float pinch = params[LOWPINCH_PARAM + high].getValue();
		pinch += inputs[LOWPINCH_INPUT + high].getVoltage() * params[LOWPINCHTRIM_PARAM + high].getValue();
		pinch = clamp(pinch, -1.f, 1.f);

		float fold = params[LOWFOLD_PARAM + high].getValue();
		fold += inputs[LOWFOLD_INPUT + high].getVoltage() * 0.1f * params[LOWFOLDTRIM_PARAM + high].getValue();
		fold = clamp(fold, 0.1f, 1.f);

		float slew = params[LOWSLEW_PARAM + high].getValue();
		slew += inputs[LOWSLEW_INPUT + high].getVoltage() * 0.1f * params[LOWSLEWTRIM_PARAM + high].getValue();
		slew = clamp(slew, 0.f, 1.f);

		// Processing
		float output = input;
		// shift - taken from vcv delay
		if (shift > 0)
		{
			output = delays[high].process(output, shift, args.sampleRate);
		}

		// pinch - taken from HetrickCV Waveshaper
		output = pincher(output, pinch);

		// fold - taken output Autodafe wavefolder
		output = folder(output, fold);

		// slew - taken from befaco slew limiter
		output = slewLims[high].process(output, slew, args.sampleTime);

		// Output high and low components
		if (high)
		{
			outputs[HIGH_OUTPUT].setVoltage(output);
			outputs[LOW_OUTPUT].setVoltage(threshold);
			// outputs[LOW_OUTPUT].setVoltage(0);	// This gives a great weird pulsey "gapped" wave
		}
		else
		{
			outputs[HIGH_OUTPUT].setVoltage(threshold);
			// outputs[HIGHA_OUTPUT].setVoltage(0);	// This gives a great weird pulsey "gapped" wave
			outputs[LOW_OUTPUT].setVoltage(output);
		}

		// Dry/Wet
		float wet = params[WET_PARAM].getValue();
		wet += inputs[WET_INPUT].getVoltage() * 0.1;
		wet = clamp(wet, 0.f, 1.f);

		output = crossfade(input, output, wet);

		// Output main
		outputs[OUT_OUTPUT].setVoltage(output);
	}
};

struct SlipWidget : ModuleWidget
{
	SlipWidget(Slip *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Slip.svg")));

		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(23.097, 23.404)), module, Slip::THRESH_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(78.503, 23.404)), module, Slip::WET_PARAM));

		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(17.82, 90.023)), module, Slip::LOWSHIFTTRIM_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(17.82, 48.282)), module, Slip::HIGHSHIFTTRIM_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(38.813, 90.023)), module, Slip::LOWPINCHTRIM_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(38.813, 48.282)), module, Slip::HIGHPINCHTRIM_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(62.807, 90.023)), module, Slip::LOWFOLDTRIM_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(62.807, 48.282)), module, Slip::HIGHFOLDTRIM_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(83.8, 90.023)), module, Slip::LOWSLEWTRIM_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(83.8, 48.282)), module, Slip::HIGHSLEWTRIM_PARAM));

		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(20.82, 77.584)), module, Slip::LOWSHIFT_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(20.82, 59.027)), module, Slip::HIGHSHIFT_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(40.813, 77.584)), module, Slip::LOWPINCH_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(40.813, 59.027)), module, Slip::HIGHPINCH_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(60.807, 77.584)), module, Slip::LOWFOLD_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(60.807, 59.027)), module, Slip::HIGHFOLD_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(80.8, 77.584)), module, Slip::LOWSLEW_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(80.8, 59.027)), module, Slip::HIGHSLEW_PARAM));

		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(8.0, 23.417)), module, Slip::THRESH_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(50.758, 23.417)), module, Slip::IN_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(93.6, 23.417)), module, Slip::WET_INPUT));

		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(14.82, 100.386)), module, Slip::LOWSHIFT_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(14.82, 36.251)), module, Slip::HIGHSHIFT_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(36.813, 100.386)), module, Slip::LOWPINCH_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(36.813, 36.251)), module, Slip::HIGHPINCH_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(64.807, 100.386)), module, Slip::LOWFOLD_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(64.807, 36.251)), module, Slip::HIGHFOLD_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(86.8, 100.386)), module, Slip::LOWSLEW_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(86.8, 36.251)), module, Slip::HIGHSLEW_INPUT));

		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(26.0, 113.225)), module, Slip::LOW_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(50.8, 113.225)), module, Slip::OUT_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(75.6, 113.225)), module, Slip::HIGH_OUTPUT));
	}
};

Model *modelSlip = createModel<Slip, SlipWidget>("Slip");