///
#pragma once


#include "../emb_common.h"


namespace emb {


/// Pin active states
enum PinActiveState
{
	ACTIVE_LOW = 0,
	ACTIVE_HIGH = 1
};


/// Pin states
enum PinState
{
	PIN_INACTIVE = 0,
	PIN_ACTIVE = 1
};


/**
 * @brief GPIO interface class.
 */
class IGpio
{
public:
	IGpio() {}
	virtual ~IGpio() {}

	virtual emb::PinState read() const = 0;
	virtual void set(emb::PinState state = emb::PIN_ACTIVE) const = 0;
	virtual void reset() const = 0;
	virtual void toggle() const = 0;
};


} // namespace emb


