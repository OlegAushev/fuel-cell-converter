///
#include "fftcalculator.h"


#ifdef CPU1

#pragma DATA_SECTION("RFFTDATA")
float FftCalculator::input[FftCalculator::m_size];
float FftCalculator::mag[FftCalculator::m_size/2+1];
float FftCalculator::phase[FftCalculator::m_size/2];
float FftCalculator::output[FftCalculator::m_size];
float FftCalculator::coef[FftCalculator::m_size];
const float FftCalculator::window[FftCalculator::m_size/2] = HAMMING64;

RFFT_F32_STRUCT FftCalculator::m_core;
RFFT_F32_STRUCT_Handle FftCalculator::m_handle;

bool FftCalculator::m_isInputFilled;
bool FftCalculator::m_isBusy;
bool FftCalculator::m_syncFlag;
size_t FftCalculator::m_index;
size_t FftCalculator::m_signalSize;

/**
 * @brief
 * @param
 * @return
 */
void FftCalculator::init()
{
	if (initialized())
	{
		return;
	}

	m_handle = &m_core;
	m_isInputFilled = false;
	m_isBusy = false;
	m_syncFlag = false;
	m_index = 0;


	m_handle->FFTSize = m_size;
	m_handle->FFTStages = m_nStages;

	m_handle->InBuf 	= &input[0];
	m_handle->OutBuf 	= &output[0];

	m_handle->MagBuf		= &mag[0];
	m_handle->PhaseBuf	= &phase[0];
	m_handle->CosSinBuf	= &coef[0];

	RFFT_f32_sincostable(m_handle);         //Calculate twiddle factor

	setInitialized();

	FftCalculator fftCalculator;
	fftCalculator.reset();
}

/**
 * @brief
 * @param
 * @return
 */
void FftCalculator::reset()
{
	for (int i = 0; i < m_size; ++i)
	{
		output[i] = 0;	//Clean up output buffer
	}

	for (int i = 0; i < m_size/2+1; ++i)
	{
		mag[i] = 0;		//Clean up magnitude buffer
	}

	for (int i = 0; i < m_size/2; ++i)
	{
		phase[i] = 0;	//Clean up phase buffer
	}

	m_isInputFilled = false;
	m_isBusy = false;
	m_syncFlag = false;
	m_index = 0;
}

/**
 * @brief
 * @param
 * @return
 */
void FftCalculator::push(float value, size_t signalSize)
{
	if (m_isInputFilled || m_isBusy)
	{
		m_syncFlag = false;	// next sync signal must be waited
		return;
	}

	if (!m_syncFlag)
	{
		return;
	}

	m_signalSize = signalSize;
	input[m_index] = value;
	++m_index;
	if (m_index >= m_signalSize)
	{
		for (int i = m_index; i < m_size; ++i)
		{
			input[i] = 0;	// zero-padding
		}
		m_isInputFilled = true;
	}
}

/**
 * @brief
 * @param
 * @return
 */
void FftCalculator::pushWindowed(float value, size_t signalSize)
{
	if (m_isInputFilled || m_isBusy)
	{
		return;
	}

	m_signalSize = signalSize;
	input[m_index + (m_size-m_signalSize)/2] = value;
	++m_index;
	if (m_index >= m_signalSize)
	{
		for (int i = 0; i < (m_size-m_signalSize)/2; ++i)
		{
			input[i] = 0;
		}

		for (int i = (m_size-m_signalSize)/2 + m_signalSize; i < m_size; ++i)
		{
			input[i] = 0;	// zero-padding
		}

		RFFT_f32_win(m_handle->InBuf, window, m_size);	// apply window
		m_isInputFilled = true;
	}
}

/**
 * @brief
 * @param
 * @return
 */
void FftCalculator::calculate()
{
	if (m_isInputFilled)
	{
		m_isBusy = true;
		RFFT_f32(m_handle);
		RFFT_f32_mag_TMU0(m_handle);
		RFFT_f32_phase_TMU0(m_handle);

		m_index = 0;
		m_isInputFilled = false;
	}
}

/**
 * @brief
 * @param
 * @return
 */
emb::Pair<float, float> FftCalculator::result(size_t harmonic) const
{
	float mag, phase;

	if (harmonic == 0)
	{
		mag = FftCalculator::mag[0] / m_size;
		phase = 0;
	}
	else
	{
		mag = FftCalculator::mag[harmonic] / (m_size/2);
		phase = FftCalculator::phase[harmonic];
	}
	return emb::Pair<float, float>(mag, phase);
}


#endif


