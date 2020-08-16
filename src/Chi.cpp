// 3-Band Voltage controlled crossover
// Ross Cameron 2020/08/16
// Title Font - Rumeur Bold
// https://github.com/groupeccc/Rumeur
// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"

struct ButterWorthFilter {
	// Second order
	// Pirkle - Designing Audio Effect Plugins in C++
	// Obviously we'll be tidying this one up at some point

	// Coefficients for low pass
	float aLP[3] = {0.f};
	float bLP[2] = {0.f};

	// Coefficients for high pass
	float aHP[3] = {0.f};
	float bHP[2] = {0.f};

	// Samples for LP first stage
	float xLP[3] = {0.f};
	float yLP[3] = {0.f};

	// Samples for LP second stage
	float xLP_2[3] = {0.f};
	float yLP_2[3] = {0.f};

	// Sample for HP first stage
	float xHP[3] = {0.f};
	float yHP[3] = {0.f};

	// Sample for HP second stage
	float xHP_2[3] = {0.f};
	float yHP_2[3] = {0.f};

	void setCoefficients(float fc, float fs)
	{
		float cHP = tan(M_PI * fc / fs);
		float cHP_2 = std::pow(cHP, 2.0);
		float cHP_sqrt2 = cHP * M_SQRT2;
		
		
		float cLP = 1 / cHP;
		float cLP_2 = std::pow(cLP, 2.0);
		float cLP_sqrt2 = cLP * M_SQRT2;

		aLP[0] = 1 / (1 + cLP_sqrt2 + cLP_2);
		aLP[1] = 2 * aLP[0];
		aLP[2] = aLP[0];

		bLP[0] = 2 * aLP[0] * (1 - cLP_2);
		bLP[1] = aLP[0] * (1 - cLP_sqrt2 + cLP_2);


		aHP[0] = 1 / (1 + cHP_sqrt2 + cHP_2);
		aHP[1] = -2 * aHP[0];
		aHP[2] = aHP[0];

		bHP[0] = 2 * aHP[0] * (cHP_2 - 1);
		bHP[1] = aHP[0] * (1 - cHP_sqrt2 + cHP_2);
	}
	float lowpass(float input)
	{
		// First stage
		xLP[0] = input;

		yLP[0] = aLP[0] * xLP[0] + aLP[1] * xLP[1] + aLP[2] * xLP[2] - bLP[0] * yLP[1] - bLP[1] * yLP[2];

		xLP[2] = xLP[1];
		xLP[1] = xLP[0];

		yLP[2] = yLP[1];
		yLP[1] = yLP[0];

		// Second stage
		xLP_2[0] = yLP[0];

		yLP_2[0] = aLP[0] * xLP_2[0] + aLP[1] * xLP_2[1] + aLP[2] * xLP_2[2] - bLP[0] * yLP_2[1] - bLP[1] * yLP_2[2];

		xLP_2[2] = xLP_2[1];
		xLP_2[1] = xLP_2[0];

		yLP_2[2] = yLP_2[1];
		yLP_2[1] = yLP_2[0];

		return yLP_2[0];
	}

	float highpass(float input)
	{
		// First Stage
		xHP[0] = input;

		yHP[0] = aHP[0] * xHP[0] + aHP[1] * xHP[1] + aHP[2] * xHP[2] - bHP[0] * yHP[1] - bHP[1] * yHP[2];

		xHP[2] = xHP[1];
		xHP[1] = xHP[0];

		yHP[2] = yHP[1];
		yHP[1] = yHP[0];

		// Second Stage
		xHP_2[0] = yHP[0];

		yHP_2[0] = aHP[0] * xHP_2[0] + aHP[1] * xHP_2[1] + aHP[2] * xHP_2[2] - bHP[0] * yHP_2[1] - bHP[1] * yHP_2[2];

		xHP_2[2] = xHP_2[1];
		xHP_2[1] = xHP_2[0];

		yHP_2[2] = yHP_2[1];
		yHP_2[1] = yHP_2[0];

		return yHP_2[0];
	}
};

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
		HIGH_X_INPUT,
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
		configParam(LOW_GAIN_PARAM, 0.f, 2.f, 1.f, "Low Gain");
		configParam(MID_GAIN_PARAM, 0.f, 2.f, 1.f, "Mid Gain");
		configParam(HIGH_GAIN_PARAM, 0.f, 2.f, 1.f, "High Gain");
		configParam(LOW_GAIN_CV_PARAM, -1.f, 1.f, 0.f, "Low Gain CV Trim");
		configParam(MID_GAIN_CV_PARAM, -1.f, 1.f, 0.f, "Mid Gain CV Trim");
		configParam(HIGH_GAIN_CV_PARAM, -1.f, 1.f, 0.f, "High Gain CV Trim");
		configParam(LOW_X_PARAM, 0.f, 1.f, 0.5f, "Low/Mid X Freq");
		configParam(HIGH_X_PARAM, 0.f, 1.f, 0.5f, "Mid/High X Freq");
	}
	ButterWorthFilter filter[2];

	float lowFreqScale (float knobValue)
	{
		// Converts a knob value from 0 -> 0.5 -> 1 to 80 -> 225 -> 640
		// float scaled = 540 * knobValue * knobValue + 20 * knobValue + 80;
		return 600;
	}

	float highFreqScale (float knobValue)
	{
		// Converts a knob value from 0 -> 0.5 -> 1 to 1k -> 2.8k -> 8k
		// float scaled = 6800 * knobValue * knobValue + 200 * knobValue + 1000;
		return 2800;
	}

	void process(const ProcessArgs &args) override
	{
		float in = inputs[IN_INPUT].getVoltage();

		/////////////// Get parameters

		// Frequencies
		// For now we're just taking MP2015 frequency ranges
		// Start with all values [0:1]
		float lowX = params[LOW_X_PARAM].getValue();
		lowX += inputs[LOW_X_INPUT].getVoltage() / 10.f;
		lowX = clamp(lowX, 0.f, 1.f);
		// Convert to Hz
		lowX = lowFreqScale(lowX);

		// Start with all values [0:1]
		float highX = params[HIGH_X_PARAM].getValue();
		highX += inputs[HIGH_X_INPUT].getVoltage() / 10.f;
		highX = clamp(highX, 0.f, 1.f);
		// Convert to Hz
		highX = highFreqScale(highX);

		// Gains
		// We go up to 2.0 because we want the right hand side of the knob to be 0dB -> +6dB

		float gains[3] = {0.f};
		for (int i = 0; i < 3; ++i)
		{
			gains[i] = params[LOW_GAIN_PARAM + i].getValue();
			gains[i] += inputs[LOW_GAIN_INPUT + i].getVoltage() * params[LOW_GAIN_CV_PARAM + i].getValue();
			gains[i] = clamp(gains[i], 0.f, 2.f);
		}

		/////////////// Process

		float outs[3] = {0.f};

		// Process low/mid Xover
		filter[0].setCoefficients(lowX, args.sampleRate);
		outs[0] = filter[0].lowpass(in);
		outs[1] = filter[0].highpass(in);

		// Process mid/high Xover
		filter[1].setCoefficients(highX, args.sampleRate);
		outs[2] = filter[1].highpass(outs[1]);
		outs[1] = filter[1].lowpass(outs[1]);

		/////////////// Output
		// Set gains and outs

		float mainOut = 0.f;
		for (int i = 0; i < 3; ++i)
		{
			outs[i] *= gains[i];
			outputs[LOW_OUTPUT + i].setVoltage(outs[i]);
			mainOut += outs[i];
		}

		outputs[OUT_OUTPUT].setVoltage(mainOut);
	}
};

struct ChiWidget : ModuleWidget
{
	ChiWidget(Chi *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Chi.svg")));

		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<FF20GKnob>(mm2px(Vec(18.473, 47.126)), module, Chi::LOW_GAIN_PARAM));
		addParam(createParamCentered<FF20GKnob>(mm2px(Vec(55.88, 47.126)), module, Chi::MID_GAIN_PARAM));
		addParam(createParamCentered<FF20GKnob>(mm2px(Vec(93.289, 47.126)), module, Chi::HIGH_GAIN_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(18.473, 70.063)), module, Chi::LOW_GAIN_CV_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(55.88, 70.063)), module, Chi::MID_GAIN_CV_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(93.289, 70.063)), module, Chi::HIGH_GAIN_CV_PARAM));
		addParam(createParamCentered<FF15GKnob>(mm2px(Vec(37.177, 70.063)), module, Chi::LOW_X_PARAM));
		addParam(createParamCentered<FF15GKnob>(mm2px(Vec(74.584, 70.063)), module, Chi::HIGH_X_PARAM));

		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(18.473, 87.595)), module, Chi::LOW_GAIN_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(55.88, 87.595)), module, Chi::MID_GAIN_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(93.289, 87.595)), module, Chi::HIGH_GAIN_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(37.177, 87.595)), module, Chi::LOW_X_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(74.584, 87.595)), module, Chi::HIGH_X_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(37.177, 113.225)), module, Chi::IN_INPUT));

		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(18.473, 23.417)), module, Chi::LOW_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(55.88, 23.417)), module, Chi::MID_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(93.289, 23.417)), module, Chi::HIGH_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(74.584, 113.225)), module, Chi::OUT_OUTPUT));
	}
};

Model *modelChi = createModel<Chi, ChiWidget>("Chi");