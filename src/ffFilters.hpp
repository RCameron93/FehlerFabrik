struct DCBlock
{
    // https://www.dsprelated.com/freebooks/filters/DC_Blocker.html

    float xm1 = 0;
    float ym1 = 0;

    float r = 0.995;

    float process(float x)
    {
        float y = x - xm1 + r * ym1;
        xm1 = x;
        ym1 = y;
        return y;
    }
};

struct ButterWorth2Filter : dsp::IIRFilter<3, 3>
{
    enum Type
    {
        LOWPASS,
        HIGHPASS
    };

    ButterWorth2Filter()
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

struct LinkwitzRiley2Filter : dsp::IIRFilter<3, 3>
{
    enum Type
    {
        LOWPASS,
        HIGHPASS
    };

    LinkwitzRiley2Filter()
    {
        setParameters(LOWPASS, 0.f, 0.f);
    }

    /** Second order Linkwitz-Riley filter
	Coefficients taken from Pirkle - Designing Audio Effect Plugins in C++ 
	Code based on filter.hpp
	Calculates and sets the biquad transfer function coefficients.
	fc: cutoff frequency
	fs: sample rate
	*/

    void setParameters(int type, float fc, float fs)
    {
        float omega = M_PI * fc;
        float theta = omega / fs;
        float kappa = omega / std::tan(theta);
        float delta = kappa * kappa + omega * omega + 2 * kappa * omega;

        // Note - Pirkle uses a0... for the x coefficients, b0 for the y coefficients.
        // Rack API is switched
        switch (type)
        {
        case LOWPASS:
            this->b[0] = omega * omega / delta;
            this->b[1] = 2 * this->b[0];
            this->b[2] = this->b[0];
            this->a[0] = (-2 * kappa * kappa + 2 * omega * omega) / delta;
            this->a[1] = (-2 * kappa * omega + kappa * kappa + omega * omega) / delta;
            break;

        case HIGHPASS:
            this->b[0] = kappa * kappa / delta;
            this->b[1] = -2 * this->b[0];
            this->b[2] = this->b[0];
            this->a[0] = (-2 * kappa * kappa + 2 * omega * omega) / delta;
            this->a[1] = (-2 * kappa * omega + kappa * kappa + omega * omega) / delta;
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
    Computes both low and high pass in parallel
	*/

    // 0,2: LPF, 1,3: HPF
    ButterWorth2Filter butterWorth[4];
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
