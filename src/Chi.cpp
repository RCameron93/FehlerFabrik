// 3-Band Voltage controlled crossover
// Ross Cameron 2020/08/16
// Title Font - Rumeur Bold
// https://github.com/groupeccc/Rumeur
// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"

struct ButterWorthFilter : dsp::IIRFilter<3, 3>
{
	enum Type
	{
		LOWPASS,
		HIGHPASS
	};

	ButterWorthFilter()
	{
		setParameters(LOWPASS, 0.f, 0.f);
	}

	/** Second order Butterworth filter
	Coefficients taken from Pirkle - Designing Audio Effect Plugins in C++ 
	Code based on filter.hpp
	Calculates and sets the biquad transfer function coefficients.
	fc: cutoff frequency
	fs: sample rate
	*/

	void setParameters(int type, float fc, float fs)
	{
		// fn: normalised frequency
		float fn = fc / fs;
		// Used in HPF
		float K = std::tan(M_PI * fn);
		// Used in LPF
		float C = 1 / K;

		// Note - Pirkle uses a0... for the x coefficients, b0 for the y coefficients.
		// Rack API is switched
		switch (type)
		{
		case LOWPASS:
			this->b[0] = 1 / (1 + M_SQRT2 * C + C * C);
			this->b[1] = 2 * this->b[0];
			this->b[2] = this->b[0];
			this->a[0] = 2 * this->b[0] * (1 - C * C);
			this->a[1] = this->b[0] * (1 - M_SQRT2 * C + C * C);
			break;

		case HIGHPASS:
			this->b[0] = 1 / (1 + M_SQRT2 * K + K * K);
			this->b[1] = -2 * this->b[0];
			this->b[2] = this->b[0];
			this->a[0] = 2 * this->b[0] * (K * K - 1);
			this->a[1] = this->b[0] * (1 - M_SQRT2 * K + K * K);
			break;

		default:
			break;
		}
	}
};

struct LinkwitzRiley4Filter
{
	/** 24 dB/Oct 4th order LR filter
	Built from 2 cascaded 2nd order BW Filters
	*/

	// 0,2: LPF, 1,3: HPF
	ButterWorthFilter butterWorth[4];
	float outs[2] = {};

	void process(float input, float fc, float fs)
	{
		// First Stage
		butterWorth[0].setParameters(0, fc, fs);
		outs[0] = butterWorth[0].process(input);

		butterWorth[1].setParameters(1, fc, fs);
		outs[1] = butterWorth[1].process(input);

		// Second Stage
		butterWorth[2].setParameters(0, fc, fs);
		outs[0] = butterWorth[2].process(outs[0]);

		butterWorth[3].setParameters(1, fc, fs);
		outs[1] = butterWorth[3].process(outs[1]);
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
	// ButterWorth2Filter filter[32];
	LinkwitzRiley4Filter filter[32];

	float lowFreqScale(float knobValue)
	{
		// Converts a knob value from 0 -> 0.5 -> 1 to 80 -> 225 -> 640
		float scaled = 540 * knobValue * knobValue + 20 * knobValue + 80;
		return scaled;
	}

	float highFreqScale(float knobValue)
	{
		// Converts a knob value from 0 -> 0.5 -> 1 to 1k -> 2.8k -> 8k
		float scaled = 6800 * knobValue * knobValue + 200 * knobValue + 1000;
		return scaled;
	}

	bool anythingConnected()
	{
		for (int i = 0; i < NUM_OUTPUTS; ++i)
		{
			if (outputs[i].isConnected())
			{
				return true;
			}
		}

		return false;
	}

	void process(const ProcessArgs &args) override
	{
		// Do we need to do anything?
		if (!anythingConnected())
		{
			return;
		}

		// Global Parameters#
		// Xover Freqs
		float lowXParam = params[LOW_X_PARAM].getValue();
		float highXParam = params[HIGH_X_PARAM].getValue();

		// Gains
		float gainParams[3] = {0.f};
		float gainCVTrimParams[3] = {0.f};
		for (int i = 0; i < 3; ++i)
		{
			gainParams[i] = params[LOW_GAIN_PARAM + i].getValue();
			gainCVTrimParams[i] = params[LOW_GAIN_CV_PARAM + i].getValue();
		}

		int channels = inputs[IN_INPUT].getChannels();

		for (int c = 0; c < channels; ++c)
		{
			float in = inputs[IN_INPUT].getPolyVoltage(c);

			/////////////// Get parameters

			// Frequencies
			// For now we're just taking MP2015 frequency ranges
			// Start with all values [0:1]
			float lowX = lowXParam;
			lowX += inputs[LOW_X_INPUT].getPolyVoltage(c) / 10.f;
			lowX = clamp(lowX, 0.f, 1.f);
			// Convert to Hz
			lowX = lowFreqScale(lowX);

			// Start with all values [0:1]
			float highX = highXParam;
			highX += inputs[HIGH_X_INPUT].getPolyVoltage(c) / 10.f;
			highX = clamp(highX, 0.f, 1.f);
			// Convert to Hz
			highX = highFreqScale(highX);

			// Gains
			// We go up to 2.0 because we want the right hand side of the knob to be 0dB -> +6dB

			float gains[3] = {0.f};
			for (int i = 0; i < 3; ++i)
			{
				gains[i] = gainParams[i];
				gains[i] += inputs[LOW_GAIN_INPUT + i].getPolyVoltage(c) * gainCVTrimParams[i];
				gains[i] = clamp(gains[i], 0.f, 2.f);
			}

			/////////////// Process

			float outs[3] = {0.f};

			// Process low/mid Xover
			filter[c * 2].process(in, lowX, args.sampleRate);
			outs[0] = filter[c * 2].outs[0];
			outs[1] = filter[c * 2].outs[1];

			// Process mid/high Xover
			filter[c * 2 + 1].process(outs[1], highX, args.sampleRate);
			outs[1] = filter[c * 2 + 1].outs[0];
			outs[2] = filter[c * 2 + 1].outs[1];

			// // Process low/mid Xover
			// filter[c * 2].setCoefficients(lowX, args.sampleRate);
			// outs[0] = filter[c * 2].lowpass(in);
			// outs[1] = filter[c * 2].highpass(in);

			// // Process mid/high Xover
			// filter[c * 2 + 1].setCoefficients(highX, args.sampleRate);
			// outs[2] = filter[c * 2 + 1].highpass(outs[1]);
			// outs[1] = filter[c * 2 + 1].lowpass(outs[1]);

			/////////////// Output
			// Set gains and outs

			float mainOut = 0.f;
			for (int i = 0; i < 3; ++i)
			{
				outs[i] *= gains[i];
				outputs[LOW_OUTPUT + i].setVoltage(outs[i], c);
				mainOut += outs[i];
			}

			outputs[OUT_OUTPUT].setVoltage(mainOut, c);
		}

		for (int i = 0; i < NUM_OUTPUTS; ++i)
		{
			outputs[i].setChannels(channels);
		}
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