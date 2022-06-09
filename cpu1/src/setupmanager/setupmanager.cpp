/**
 * @file
 * @ingroup setup_manager
 */


#include "setupmanager.h"


///
///
///
SetupManager::SetupManager()
{
	m_converter = static_cast<BoostConverter*>(NULL);
}


///
///
///
void SetupManager::registerConverter(BoostConverter* converter)
{
	m_converter = converter;
}


