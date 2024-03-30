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

#pragma once

#include "WwiseUnrealDefines.h"

// We use Catch2 Test Harness, available starting UE5.1
#if UE_5_2_OR_LATER
#include "Tests/TestHarnessAdapter.h"
#elif UE_5_1_OR_LATER
#include "Tests/TestHarness.h"
#else
#include "Misc/AutomationTest.h"
#endif // UE_5_2_OR_LATER

// Determine if we have unit tests. These Unreal macros are defined in the previously included header files.
#if !defined(WWISE_LOW_LEVEL_TESTS)
#if defined(WITH_LOW_LEVEL_TESTS) && WITH_LOW_LEVEL_TESTS
#define WWISE_LOW_LEVEL_TESTS 1
#else
#define WWISE_LOW_LEVEL_TESTS 0
#endif
#endif

#if !defined(WWISE_AUTOMATION_TESTS)
#if defined(WITH_AUTOMATION_TESTS) && WITH_AUTOMATION_TESTS
#define WWISE_AUTOMATION_TESTS 1
#else
#define WWISE_AUTOMATION_TESTS 0
#endif
#endif

#if !defined(WWISE_UNIT_TESTS)
#if WWISE_LOW_LEVEL_TESTS || WWISE_AUTOMATION_TESTS
#define WWISE_UNIT_TESTS 1
#else
#define WWISE_UNIT_TESTS 0
#endif
#endif

//
// Definition of Wwise Unit Tests API. Unreal's Test API improved throughout the earlier UE5 versions. This
// allows for a normalized interface starting from UE4.27.
//
#if WWISE_UNIT_TESTS
#include <type_traits>


// This is an adapted copy of UE5.1's Misc/LowLevelTestAdapter.h for UE4.27 & UE5.0 usage.
#if !UE_5_1_OR_LATER
#include "Misc/AssertionMacros.h"
#include "Misc/ScopeExit.h"
#include "CoreTypes.h"
class FWwiseAutomationTestBase : public FAutomationTestBase
{
public:
	FWwiseAutomationTestBase( const FString& InName )
	: FAutomationTestBase( InName, false )
	{
	}

	// Adapted from UE5.1's Misc/AutomationTest.h and .cpp
	uint32 ExtractAutomationTestFlags(FString InTagNotation)
	{
		static const TMap<FString, EAutomationTestFlags::Type> FlagsMap = {
			{ TEXT("EditorContext"), EAutomationTestFlags::Type::EditorContext},
			{ TEXT("ClientContext"), EAutomationTestFlags::Type::ClientContext},
			{ TEXT("ServerContext"), EAutomationTestFlags::Type::ServerContext},
			{ TEXT("CommandletContext"), EAutomationTestFlags::Type::CommandletContext},
			{ TEXT("ApplicationContextMask"), EAutomationTestFlags::Type::ApplicationContextMask},
			{ TEXT("NonNullRHI"), EAutomationTestFlags::Type::NonNullRHI},
			{ TEXT("RequiresUser"), EAutomationTestFlags::Type::RequiresUser},
			{ TEXT("FeatureMask"), EAutomationTestFlags::Type::FeatureMask},
			{ TEXT("Disabled"), EAutomationTestFlags::Type::Disabled},
			{ TEXT("CriticalPriority"), EAutomationTestFlags::Type::CriticalPriority},
			{ TEXT("HighPriority"), EAutomationTestFlags::Type::HighPriority},
			{ TEXT("HighPriorityAndAbove"), EAutomationTestFlags::Type::HighPriorityAndAbove},
			{ TEXT("MediumPriority"), EAutomationTestFlags::Type::MediumPriority},
			{ TEXT("MediumPriorityAndAbove"), EAutomationTestFlags::Type::MediumPriorityAndAbove},
			{ TEXT("LowPriority"), EAutomationTestFlags::Type::LowPriority},
			{ TEXT("PriorityMask"), EAutomationTestFlags::Type::PriorityMask},
			{ TEXT("SmokeFilter"), EAutomationTestFlags::Type::SmokeFilter},
			{ TEXT("EngineFilter"), EAutomationTestFlags::Type::EngineFilter},
			{ TEXT("ProductFilter"), EAutomationTestFlags::Type::ProductFilter},
			{ TEXT("PerfFilter"), EAutomationTestFlags::Type::PerfFilter},
			{ TEXT("StressFilter"), EAutomationTestFlags::Type::StressFilter},
			{ TEXT("NegativeFilter"), EAutomationTestFlags::Type::NegativeFilter},
			{ TEXT("FilterMask"), EAutomationTestFlags::Type::FilterMask}
		};

		uint32 Result = 0;
		TArray<FString> OutputParts;
		InTagNotation
			.Replace(TEXT("["), TEXT(""))
			.Replace(TEXT("]"), TEXT(";"))
			.ParseIntoArray(OutputParts, TEXT(";"), true);
		for (auto it = OutputParts.begin(); it != OutputParts.end(); ++it)
		{
			if (FlagsMap.Contains(*it))
			{
				Result |= FlagsMap[*it];
			}
		}
		return Result;
	}
};

#define IMPLEMENT_SIMPLE_AUTOMATION_TEST_PRIVATE_LLT( TClass, PrettyName, TFlags, FileName, LineNumber ) \
		class TClass : public FWwiseAutomationTestBase \
		{ \
		public:\
			TClass( const FString& InName) \
			: FWwiseAutomationTestBase( InName ) \
			{ \
				TestFlags = ExtractAutomationTestFlags(TFlags); \
				PrettyNameDotNotation = FString(PrettyName).Replace(TEXT("::"), TEXT(".")); \
				if (!(TestFlags & EAutomationTestFlags::ApplicationContextMask)) \
				{ \
					TestFlags |= EAutomationTestFlags::ApplicationContextMask; \
				} \
				if (!(TestFlags & EAutomationTestFlags::FilterMask)) \
				{ \
					TestFlags |= EAutomationTestFlags::EngineFilter; \
				} \
			} \
			virtual uint32 GetTestFlags() const override { return TestFlags; } \
			virtual bool IsStressTest() const { return false; } \
			virtual uint32 GetRequiredDeviceNum() const override { return 1; } \
			virtual FString GetTestSourceFileName() const override { return FileName; } \
			virtual int32 GetTestSourceFileLine() const override { return LineNumber; } \
		protected: \
			virtual void GetTests(TArray<FString>& OutBeautifiedNames, TArray <FString>& OutTestCommands) const override \
			{ \
				OutBeautifiedNames.Add(PrettyNameDotNotation); \
				OutTestCommands.Add(FString());\
			} \
			void TestBody(const FString& Parameters); \
			virtual bool RunTest(const FString& Parameters) { \
				TestBody(Parameters); \
				return true; \
			} \
			virtual FString GetBeautifiedTestName() const override { return PrettyNameDotNotation; } \
		private:\
			uint32 TestFlags; \
			FString PrettyNameDotNotation; \
		};

#define LLT_JOIN(Prefix, Counter) LLT_JOIN_INNER(Prefix, Counter)
#define LLT_JOIN_INNER(Prefix, Counter) Prefix##Counter

#define TEST_CASE_NAMED(TClass, StrName, PrettyName, TFlags) \
		IMPLEMENT_SIMPLE_AUTOMATION_TEST_PRIVATE_LLT(TClass, PrettyName, TFlags, __FILE__, __LINE__) \
		namespace \
		{ \
			TClass LLT_JOIN(TClass, Instance)(TEXT(StrName)); \
		} \
		void TClass::TestBody(const FString& Parameters)


//#define TEST_CASE_GENERATED_NAME_UNIQUE LLT_JOIN(FLLTAdaptedTest, __COUNTER__)
#define LLT_STR(Macro) #Macro
#define LLT_STR_EXPAND(Macro) LLT_STR(Macro)
//#define TEST_CASE_GENERATED_NAME_UNIQUE_STR LLT_STR_EXPAND(TEST_CASE_GENERATED_NAME_UNIQUE)
//#define TEST_CASE(PrettyName, TFlags) TEST_CASE_NAMED(TEST_CASE_GENERATED_NAME_UNIQUE, TEST_CASE_GENERATED_NAME_UNIQUE_STR, PrettyName, TFlags)

#define CHECK(Expr) if (!(Expr)) { FAutomationTestFramework::Get().GetCurrentTest()->AddError(TEXT("Condition failed")); }
#define CHECK_FALSE(Expr) if (Expr) { FAutomationTestFramework::Get().GetCurrentTest()->AddError(TEXT("Condition expected to return false but returned true")); }
#define REQUIRE(Expr) if (!(Expr)) { FAutomationTestFramework::Get().GetCurrentTest()->AddError(TEXT("Required condition failed, interrupting test")); return; }

#define SECTION(Text) AddInfo(TEXT(Text));
#endif


// Some of our testing actually prefer to use the fast version of HashCombine. This is only accessible in UE5.
inline uint32 WwiseHashCombineFast(uint32 A, uint32 B)
{
#if UE_5_0_OR_LATER
	return HashCombineFast(A, B);
#else
	return HashCombine(A, B);
#endif
}


// Add logging facilities when running in Automation
#if WWISE_AUTOMATION_TESTS
#define WWISE_TEST_LOG(Format, ...) FAutomationTestFramework::Get().GetCurrentTest()->AddInfo(FString::Printf(TEXT(Format), __VA_ARGS__));
#else
#define WWISE_TEST_LOG(Format, ...) (void)0
#endif


// Use a normalized test case that works correctly with multiple compilation units (Unreal's TEST_CASE doesn't like being used in more than one module) 
#if WWISE_LOW_LEVEL_TESTS
#define WWISE_TEST_CASE(ClassName, PrettyName, Flags) TEST_CASE(PrettyName, Flags)
#elif UE_5_3_OR_LATER
#define WWISE_TEST_CASE(ClassName, PrettyName, Flags) TEST_CASE_NAMED(FWwiseTest ## ClassName, PrettyName, Flags)
#else
#define WWISE_TEST_CASE(ClassName, PrettyName, Flags) TEST_CASE_NAMED(FWwiseTest ## ClassName, LLT_STR_EXPAND(FWwiseTest ## ClassName), PrettyName, Flags)
#endif

#define WWISE_TEST_ASYNC_NAME TEXT("WwiseUnitTests Async")

#endif // WWISE_UNIT_TESTS