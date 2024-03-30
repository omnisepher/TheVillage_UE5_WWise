/*******************************************************************************
The content of this file includes portions of the proprietary AUDIOKINETIC Wwise
Technology released in source code form as part of the game integration package.
The content of this file may not be used without valid licenses to the
AUDIOKINETIC Wwise Technology.
Note that the use of the game engine is subject to the Unreal(R) Engine End User
License Agreement at https://www.unrealengine.com/en-US/eula/unreal
 
License Usage
 
Licensees holding valid licenses to the AUDIOKINETIC Wwise Technology may use
this file in accordance with the end user license agreement provided with the
software or, alternatively, in accordance with the terms contained
in a written agreement between you and Audiokinetic Inc.
Copyright (c) 2024 Audiokinetic Inc.
*******************************************************************************/

#include "Wwise/WwiseSoundEngineUtils.h"
#include "WwiseDefines.h"


namespace WwiseUnrealHelper
{
		const TCHAR* GetResultString(AKRESULT InResult)
	{
		switch (InResult)
		{
		case AK_NotImplemented:				return TEXT("This feature is not implemented.");
		case AK_Success:						return TEXT("The operation was successful.");
		case AK_Fail:							return TEXT("The operation failed.");
		case AK_PartialSuccess:				return TEXT("The operation succeeded partially.");
		case AK_NotCompatible:				return TEXT("Incompatible formats");
		case AK_AlreadyConnected:				return TEXT("The stream is already connected to another node.");
		case AK_InvalidFile:					return TEXT("The provided file is the wrong format or unexpected values causes the file to be invalid.");
		case AK_AudioFileHeaderTooLarge:		return TEXT("The file header is too large.");
		case AK_MaxReached:					return TEXT("The maximum was reached.");
		case AK_InvalidID:					return TEXT("The ID is invalid.");
		case AK_IDNotFound:					return TEXT("The ID was not found.");
		case AK_InvalidInstanceID:			return TEXT("The InstanceID is invalid.");
		case AK_NoMoreData:					return TEXT("No more data is available from the source.");
		case AK_InvalidStateGroup:			return TEXT("The StateGroup is not a valid channel.");
		case AK_ChildAlreadyHasAParent:		return TEXT("The child already has a parent.");
		case AK_InvalidLanguage:				return TEXT("The language is invalid (applies to the Low-Level I/O).");
		case AK_CannotAddItselfAsAChild:		return TEXT("It is not possible to add itself as its own child.");
		case AK_InvalidParameter:				return TEXT("Something is not within bounds, check the documentation of the function returning this code.");
		case AK_ElementAlreadyInList:			return TEXT("The item could not be added because it was already in the list.");
		case AK_PathNotFound:					return TEXT("This path is not known.");
		case AK_PathNoVertices:				return TEXT("Stuff in vertices before trying to start it");
		case AK_PathNotRunning:				return TEXT("Only a running path can be paused.");
		case AK_PathNotPaused:				return TEXT("Only a paused path can be resumed.");
		case AK_PathNodeAlreadyInList:		return TEXT("This path is already there.");
		case AK_PathNodeNotInList:			return TEXT("This path is not there.");
		case AK_DataNeeded:					return TEXT("The consumer needs more.");
		case AK_NoDataNeeded:					return TEXT("The consumer does not need more.");
		case AK_DataReady:					return TEXT("The provider has available data.");
		case AK_NoDataReady:					return TEXT("The provider does not have available data.");
		case AK_InsufficientMemory:			return TEXT("Memory error.");
		case AK_Cancelled:					return TEXT("The requested action was cancelled (not an error).");
		case AK_UnknownBankID:				return TEXT("Trying to load a bank using an ID which is not defined.");
		case AK_BankReadError:				return TEXT("Error while reading a bank.");
		case AK_InvalidSwitchType:			return TEXT("Invalid switch type (used with the switch container)");
		case AK_FormatNotReady:				return TEXT("Source format not known yet.");
		case AK_WrongBankVersion:				return TEXT("The bank version is not compatible with the current bank reader.");
		case AK_FileNotFound:					return TEXT("File not found.");
		case AK_DeviceNotReady:				return TEXT("Specified ID doesn't match a valid hardware device: either the device doesn't exist or is disabled.");
		case AK_BankAlreadyLoaded:			return TEXT("The bank load failed because the bank is already loaded.");
		case AK_RenderedFX:					return TEXT("The effect on the node is rendered.");
		case AK_ProcessNeeded:				return TEXT("A routine needs to be executed on some CPU.");
		case AK_ProcessDone:					return TEXT("The executed routine has finished its execution.");
		case AK_MemManagerNotInitialized:		return TEXT("The memory manager should have been initialized at this point.");
		case AK_StreamMgrNotInitialized:		return TEXT("The stream manager should have been initialized at this point.");
		case AK_SSEInstructionsNotSupported:	return TEXT("The machine does not support SSE instructions (required on PC).");
		case AK_Busy:							return TEXT("The system is busy and could not process the request.");
		case AK_UnsupportedChannelConfig:		return TEXT("Channel configuration is not supported in the current execution context.");
		case AK_PluginMediaNotAvailable:		return TEXT("Plugin media is not available for effect.");
		case AK_MustBeVirtualized:			return TEXT("Sound was Not Allowed to play.");
		case AK_CommandTooLarge:				return TEXT("SDK command is too large to fit in the command queue.");
		case AK_RejectedByFilter:				return TEXT("A play request was rejected due to the MIDI filter parameters.");
		case AK_InvalidCustomPlatformName:	return TEXT("Detecting incompatibility between Custom platform of banks and custom platform of connected application");
		case AK_DLLCannotLoad:				return TEXT("Plugin DLL could not be loaded, either because it is not found or one dependency is missing.");
		case AK_DLLPathNotFound:				return TEXT("Plugin DLL search path could not be found.");
		case AK_NoJavaVM:						return TEXT("No Java VM provided in AkInitSettings.");
		case AK_OpenSLError:					return TEXT("OpenSL returned an error.  Check error log for more details.");
		case AK_PluginNotRegistered:			return TEXT("Plugin is not registered.  Make sure to implement a AK::PluginRegistration class for it and use AK_STATIC_LINK_PLUGIN in the game binary.");
		case AK_DataAlignmentError:			return TEXT("A pointer to audio data was not aligned to the platform's required alignment (check AkTypes.h in the platform-specific folder)");
		case AK_DeviceNotCompatible:			return TEXT("Incompatible Audio device.");
		case AK_DuplicateUniqueID:			return TEXT("Two Wwise objects share the same ID.");
		case AK_InitBankNotLoaded:			return TEXT("The Init bank was not loaded yet, the sound engine isn't completely ready yet.");
		case AK_DeviceNotFound:				return TEXT("The specified device ID does not match with any of the output devices that the sound engine is currently using.");
		case AK_PlayingIDNotFound:			return TEXT("Calling a function with a playing ID that is not known.");
		case AK_InvalidFloatValue:			return TEXT("One parameter has a invalid float value such as NaN, INF or FLT_MAX.");
		case AK_FileFormatMismatch:			return TEXT("Media file format unexpected");
		case AK_NoDistinctListener:			return TEXT("No distinct listener provided for AddOutput");
		case AK_ACP_Error:					return TEXT("Generic XMA decoder error.");
		case AK_ResourceInUse:				return TEXT("Resource is in use and cannot be released.");
		case AK_InvalidBankType:				return TEXT("Invalid bank type. The bank type was either supplied through a function call (e.g. LoadBank) or obtained from a bank loaded from memory.");
		case AK_AlreadyInitialized:			return TEXT("Init() was called but that element was already initialized.");
		case AK_NotInitialized:				return TEXT("The component being used is not initialized. Most likely AK::SoundEngine::Init() was not called yet, or AK::SoundEngine::Term was called too early.");
		case AK_FilePermissionError:			return TEXT("The file access permissions prevent opening a file.");
		case AK_UnknownFileError:				return TEXT("Rare file error occured, as opposed to AK_FileNotFound or AK_FilePermissionError. This lumps all unrecognized OS file system errors.");
#if WWISE_2023_1_OR_LATER
		case AK_TooManyConcurrentOperations:	return TEXT("When using StdStream, file operations can be blocking or not. When not blocking, operations need to be synchronized externally properly. If not, this error occurs.");
		case AK_InvalidFileSize:				return TEXT("The file requested was found and opened but is either 0 bytes long or not the expected size. This usually point toward a Low Level IO Hook implementation error.");
		case AK_Deferred:						return TEXT("Returned by functions to indicate to the caller the that the operation is done asynchronously. Used by Low Level IO Hook implementations when async operation are suppored by the hardware.");
		case AK_FilePathTooLong:				return TEXT("The combination of base path and file name exceeds maximum buffer lengths.");
#endif
		default: return TEXT("Unknown Error.");
		}
	}
}
