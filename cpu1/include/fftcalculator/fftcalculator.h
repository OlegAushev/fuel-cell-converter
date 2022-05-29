///
#pragma once


#include "device.h"
#include "math.h"
#include "dsp/fpu_rfft.h"
#include "dsp/fpu_fft_hamming.h"
#include "emb/emb_common.h"
#include "emb/emb_pair.h"


#ifdef CPU1

class FftCalculator : public emb::Monostate<FftCalculator>
{
private:
	static const uint16_t m_nStages = 6;
	static const uint16_t m_size = 1 << m_nStages;
	static RFFT_F32_STRUCT m_core;
	static RFFT_F32_STRUCT_Handle m_handle;

	static bool m_isInputFilled;
	static bool m_isBusy;
	static bool m_syncFlag;
	static size_t m_index;
	static size_t m_signalSize;

public:
	FftCalculator() {}
	static void init();

	static float input[m_size];
	static float mag[m_size/2+1];
	static float phase[m_size/2];
	static float output[m_size];
	static float coef[m_size];
	static const float window[m_size/2];

	size_t inputSize() const { return m_size; }
	size_t harmonicIndexMax() const { return m_size/2 - 1; }
	void reset();
	void push(float value, size_t signalSize);
	void pushWindowed(float value, size_t signalSize);
	void calculate();
	void allowUse() { m_isBusy = false; }
	void sync() { m_syncFlag = true; }
	emb::Pair<float, float> result(size_t harmonic)  const;
};

#endif


