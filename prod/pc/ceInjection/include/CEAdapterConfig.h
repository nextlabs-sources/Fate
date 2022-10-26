/*********************************************************************
 *
 * CEAdapterConfig.h
 *
 * This file provides the interface for the reading of adapter dynamic
 * injection configuration files.
 *
 * Notes: This interface is MT-safe.
 *
 ********************************************************************/

#ifndef __CEADAPTERCONFIG_H__
#define __CEADAPTERCONFIG_H__

#include "Detours.h"

/** LoadHookingConfiguration
 *
 */
bool LoadHookingConfiguration(const WCHAR *m_szProcessName,
			      std::vector<HookDetour*>& hlist);

/** LoadHookingConfigurationFile
 *
 */
bool LoadHookingConfigurationFile(const WCHAR* config_file ,
				  std::vector<HookDetour*>& hlist );

/** LoadHookingConfiguration_free
 *
 */
void LoadHookingConfiguration_free( std::vector<HookDetour*>& hlist );

#endif /* __CEADAPTERCONFIG_H__ */
