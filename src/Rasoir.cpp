// Asymmetrical Voltage Processor
// Ross Cameron 2020/07/21
// Title Font - VTF Lment
// https://velvetyne.fr/fonts/lment/
// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"
#include "samplerate.h"
#include "ffFilters.hpp"
#include "ffCommon.hpp"

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

struct Rasoir : Module
{
	enum ParamIds
	{
		THRESH_PARAM,
		WET_PARAM,
		THRESHTRIM_PARAM,
		DC_PARAM,
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

	DCBlock dcFilter;

	Rasoir()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(THRESH_PARAM, -10.f, 10.f, 0.f, "High/Low Threshold", "V");
		configParam(WET_PARAM, 0.f, 1.f, 1.f, "Wet/Dry Mix", "%", 0.f, 100.f);
		configParam(THRESHTRIM_PARAM, -1.f, 1.f, 0.f, "Threshold CV Amount", "%", 0.f, 100.f);
		configSwitch(DC_PARAM, 0.f, 1.f, 1.f, "DC Offset Filter", {"Off", "On"});
		for (int i = 0; i < 8; ++i)
		{
			configParam(LOWSHIFTTRIM_PARAM + i, -1.f, 1.f, 0.f, "CV Amount", "%", 0.f, 100.f);
		}

		configParam(LOWSHIFT_PARAM, 0.f, 1.f, 0.f, "Low Shift");
		configParam(HIGHSHIFT_PARAM, 0.f, 1.f, 0.f, "High Shift");
		configParam(LOWPINCH_PARAM, -1.f, 1.f, 0.f, "Low Pinch");
		configParam(HIGHPINCH_PARAM, -1.f, 1.f, 0.f, "High Pinch");
		configParam(LOWFOLD_PARAM, 0.1f, 1.f, 1.f, "Low Wavefold");
		configParam(HIGHFOLD_PARAM, 0.1f, 1.f, 1.f, "High Wavefold");
		configParam(LOWSLEW_PARAM, 0.f, 1.f, 0.f, "Low Slew Limiter");
		configParam(HIGHSLEW_PARAM, 0.f, 1.f, 0.f, "High Slew Limiter");

		configInput(THRESH_INPUT, "Voltage Threshold CV");
		configInput(IN_INPUT, "Signal");
		configInput(WET_INPUT, "Dry/Wet CV");
		configInput(LOWSHIFT_INPUT, "Low Shift CV");
		configInput(HIGHSHIFT_INPUT, "High Shift CV");
		configInput(LOWPINCH_INPUT, "Low Pinch CV");
		configInput(HIGHPINCH_INPUT, "High Pinch CV");
		configInput(LOWFOLD_INPUT, "Low Wavefold CV");
		configInput(HIGHFOLD_INPUT, "High Wavefold CV");
		configInput(LOWSLEW_INPUT, "Low Slew Limiter CV");
		configInput(HIGHSLEW_INPUT, "High Slew Limiter CV");

		configOutput(LOW_OUTPUT, "Low Voltage");
		configOutput(OUT_OUTPUT, "Main");
		configOutput(HIGH_OUTPUT, "High Voltage");

		configBypass(IN_INPUT, LOW_OUTPUT);
		configBypass(IN_INPUT, OUT_OUTPUT);
		configBypass(IN_INPUT, HIGH_OUTPUT);
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
		threshold += inputs[THRESH_INPUT].getVoltage() * params[THRESHTRIM_PARAM].getValue();
		threshold = clamp(threshold, -10.f, 10.f);

		// Is the current sample above or below the threshold?
		bool high = input > threshold;

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
		if (params[DC_PARAM].getValue())
		{
			output = dcFilter.process(output);
		}

		// Check for NaN
		output = std::isfinite(output) ? output : 0.f;

		outputs[OUT_OUTPUT].setVoltage(output);
	}
};

struct RasoirWidget : ModuleWidget
{
	RasoirWidget(Rasoir *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Rasoir.svg")));

		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(30.084, 23.404)), module, Rasoir::THRESH_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(78.503, 23.404)), module, Rasoir::WET_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(19.14, 23.864)), module, Rasoir::THRESHTRIM_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(50.8, 36.251)), module, Rasoir::DC_PARAM));

		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(17.82, 90.023)), module, Rasoir::LOWSHIFTTRIM_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(17.82, 48.282)), module, Rasoir::HIGHSHIFTTRIM_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(38.813, 90.023)), module, Rasoir::LOWPINCHTRIM_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(38.813, 48.282)), module, Rasoir::HIGHPINCHTRIM_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(62.807, 90.023)), module, Rasoir::LOWFOLDTRIM_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(62.807, 48.282)), module, Rasoir::HIGHFOLDTRIM_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(83.8, 90.023)), module, Rasoir::LOWSLEWTRIM_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(83.8, 48.282)), module, Rasoir::HIGHSLEWTRIM_PARAM));

		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(20.82, 77.584)), module, Rasoir::LOWSHIFT_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(20.82, 59.027)), module, Rasoir::HIGHSHIFT_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(40.813, 77.584)), module, Rasoir::LOWPINCH_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(40.813, 59.027)), module, Rasoir::HIGHPINCH_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(60.807, 77.584)), module, Rasoir::LOWFOLD_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(60.807, 59.027)), module, Rasoir::HIGHFOLD_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(80.8, 77.584)), module, Rasoir::LOWSLEW_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(80.8, 59.027)), module, Rasoir::HIGHSLEW_PARAM));

		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(8.0, 23.417)), module, Rasoir::THRESH_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(50.758, 23.417)), module, Rasoir::IN_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(93.6, 23.417)), module, Rasoir::WET_INPUT));

		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(14.82, 100.386)), module, Rasoir::LOWSHIFT_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(14.82, 36.251)), module, Rasoir::HIGHSHIFT_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(36.813, 100.386)), module, Rasoir::LOWPINCH_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(36.813, 36.251)), module, Rasoir::HIGHPINCH_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(64.807, 100.386)), module, Rasoir::LOWFOLD_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(64.807, 36.251)), module, Rasoir::HIGHFOLD_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(86.8, 100.386)), module, Rasoir::LOWSLEW_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(86.8, 36.251)), module, Rasoir::HIGHSLEW_INPUT));

		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(26.0, 113.225)), module, Rasoir::LOW_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(50.8, 113.225)), module, Rasoir::OUT_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(75.6, 113.225)), module, Rasoir::HIGH_OUTPUT));
	}
};

Model *modelRasoir = createModel<Rasoir, RasoirWidget>("Rasoir");