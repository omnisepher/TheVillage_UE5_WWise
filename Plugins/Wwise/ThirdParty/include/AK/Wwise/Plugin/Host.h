/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Apache License Usage

Alternatively, this file may be used under the Apache License, Version 2.0 (the 
"Apache License"); you may not use this file except in compliance with the 
Apache License. You may obtain a copy of the Apache License at 
http://www.apache.org/licenses/LICENSE-2.0.

Unless required by applicable law or agreed to in writing, software distributed
under the Apache License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
OR CONDITIONS OF ANY KIND, either express or implied. See the Apache License for
the specific language governing permissions and limitations under the License.

  Copyright (c) 2024 Audiokinetic Inc.
*******************************************************************************/

/**
 * \brief Wwise Authoring Plug-ins - API to request host's current state and services.
 * \file AK/Wwise/Plugin/Host.h
 */

#pragma once

#include "./V1/Host.h"
#include "../PlatformID.h"
#include "PluginInfoGenerator.h"

/**
 * \brief API to request host's current state and services.
 * 
 * If requested, this contains information on the current state of the host as well as generic operations.
 * 
 * For example, the currently selected platform, tools to post
 * when internal plug-in data changed, or a way to make Waapi calls.
 * 
 * In order to be reactive to host's state, you should consider implementing ak_wwise_plugin_notifications_host_v1
 * notifications.
 */
struct ak_wwise_plugin_host_v2
#ifdef __cplusplus
	: public ak_wwise_plugin_host_v1
#endif
{
#ifndef __cplusplus
	ak_wwise_plugin_host_v1 m_host_v1;
#else
	/// Base host-provided instance type.
	using Instance = ak_wwise_plugin_host_instance_v2;
	ak_wwise_plugin_host_v2() :
		ak_wwise_plugin_host_v1(/*in_version = */ 2)
	{
	}
#endif

	/**
	 * \brief Obtain the project license ID
	 * 
	 * This ID is composed of 8 characters and is used to identify a project 
	 * with Audiokinetic. You may use this ID as a key to identify a given 
	 * project when implementing a custom licensing scheme.
	 * 
	 * \param[in] in_this Current instance of this interface.
	 * \return An instance of LicenseID filled with the project license ID, or 
	 * zeroed-out if the project has no license.
	 */
	AK::Wwise::Plugin::LicenseID(*GetProjectLicenseID)(const struct ak_wwise_plugin_host_v2* in_this);
};

#define AK_WWISE_PLUGIN_HOST_V2_ID() \
	AK_WWISE_PLUGIN_BASE_INTERFACE_FROM_ID(AK_WWISE_PLUGIN_INTERFACE_TYPE_HOST, 2)
#define AK_WWISE_PLUGIN_HOST_V2_CTOR() \
{ \
	.m_host_v1.m_baseInterface = AK_WWISE_PLUGIN_HOST_V2_ID() \
}

#ifdef __cplusplus
namespace AK::Wwise::Plugin
{
	namespace V2
	{
		using CHost = ak_wwise_plugin_host_v2;

		/// \copydoc ak_wwise_plugin_host_v2
		class Host : public AK::Wwise::Plugin::V1::HostBase<CHost, 2>
		{
		public:
			using Interface = CHost;
			using Instance = CHost::Instance;

			/**
			 * \brief Obtain the project license ID
			 * 
			 * This ID is composed of 8 characters and is used to identify a project 
			 * with Audiokinetic. You may use this ID as a key to identify a given 
			 * project when implementing a custom licensing scheme.
			 * 
			 * \param[in] in_this Current instance of this interface.
			 * \return An instance of LicenseID filled with the project license ID, or 
			 * zeroed-out if the project has no license.
			 */
			inline LicenseID GetProjectLicenseID() const
			{
				return g_cinterface->GetProjectLicenseID(g_cinterface);
			}

		#if defined( DOXYGEN_INCLUDE )
			GUID GetCurrentPlatform() const;
			BasePlatformID GetCurrentBasePlatform() const;
			BasePlatformID GetDefaultNativeAuthoringPlaybackPlatform() const;
			GUID GetAuthoringPlaybackPlatform() const;
			void NotifyInternalDataChanged(AkPluginParamID in_idData, bool in_bMakeProjectDirty);
			void GetLicenseStatus(
				const GUID& in_guidPlatform,
				LicenseType& out_eType,
				LicenseStatus& out_eStatus,
				uint32_t& out_uDaysToExpiry
			) const;
			void GetAssetLicenseStatus(
				const GUID& in_guidPlatform,
				AkUInt32 in_uAssetID,
				LicenseType& out_eType,
				LicenseStatus& out_eStatus,
				uint32_t& out_uDaysToExpiry
			) const;
			void WaapiCall(
				const char* in_szUri,
				const char* in_szArgs,
				const char* in_szOptions,
				AK::IAkPluginMemAlloc& in_alloc,
				char*& out_szResults,
				char*& out_szError
			) const;
		#endif
		};

		/**
		 * \brief Requests a Host interface, provided as m_host variable.
		 * 
		 * Deriving your plug-in class from RequestHost will automatically request both Host and
		 * Notifications::Host_ interfaces. From this point, you will be able to derive from the virtual
		 * functions as defined in Notifications::Host_, and access the host-provided functions in the
		 * \c m_host variable.
		 */
		using RequestHost = RequestedHostInterface<Host>;

	} // of namespace V2

	/// Latest version of the C Host interface.
	using CHost = V2::CHost;
	/// Latest version of the C++ Host interface.
	using Host = V2::Host;
	/// Latest version of the requested C++ Host interface.
	using RequestHost = V2::RequestHost;

	AK_WWISE_PLUGIN_SPECIALIZE_HOST_INTERFACE(Host, host,, public Notifications::Host);
	AK_WWISE_PLUGIN_SPECIALIZE_INTERFACE_CLASS(Host);
	AK_WWISE_PLUGIN_SPECIALIZE_INTERFACE_VERSION(Host);
} // of namespace AK::Wwise::Plugin

#endif
